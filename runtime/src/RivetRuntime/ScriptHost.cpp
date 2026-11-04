#include <RivetRuntime/ScriptHost.hpp>

#include <RivetRuntime/Bindings.hpp>
#include <RivetRuntime/GameSource.hpp>

#include <sol/sol.hpp>

#include <algorithm>
#include <utility>

namespace
{
    // Turns a module name into the path it's read from, so `require("objects.ball")` looks for
    //      "objects/ball.lua" in the mounted game.
    std::string to_module_path(const std::string& name)
    {
        std::string path = name;
        std::replace(path.begin(), path.end(), '.', '/');
        return path + ".lua";
    }

    // Calls a callback the game may not have defined, and reports a script error rather than
    //      letting it reach the process.
    template <typename... Args>
    bool invoke(sol::protected_function& function, std::string& out_error, Args&&... args)
    {
        // A callback the game didn't define is nothing to call, which isn't a failure.
        if (!function.valid()) {
            return true;
        }

        sol::protected_function_result result = function(std::forward<Args>(args)...);
        if (!result.valid()) {
            const sol::error error = result;
            out_error = error.what();
            return false;
        }

        return true;
    }
}

namespace RivetRuntime
{
    struct ScriptHost::Impl
    {
        sol::state lua;
        // `debug.traceback`, so a script error says where in the game it came from and not just
        //      what went wrong.
        sol::reference traceback;
        sol::protected_function on_load;
        sol::protected_function on_update;
        sol::protected_function on_draw;
        sol::protected_function on_quit;
    };

    ScriptHost& ScriptHost::get_singleton()
    {
        static ScriptHost singleton;
        return singleton;
    }

    ScriptHost::ScriptHost()
    {
        this->ready = false;
        this->failed = false;
    }

    ScriptHost::~ScriptHost()
    {
        this->dispose();
    }

    // --- GETTERS --- //
    bool ScriptHost::is_ready()
    {
        ScriptHost& self = ScriptHost::get_singleton();
        return self.ready;
    }

    bool ScriptHost::has_failed()
    {
        ScriptHost& self = ScriptHost::get_singleton();
        return self.failed;
    }

    std::string ScriptHost::get_error()
    {
        ScriptHost& self = ScriptHost::get_singleton();
        return self.error;
    }

    // --- CALLBACKS --- //
    void ScriptHost::load()
    {
        ScriptHost& self = ScriptHost::get_singleton();

        // RUNTIME GUARD: Nothing is called into before the state is open, or after a script
        //      error, which is what leaves the error on screen instead of the game.
        if (!self.ready || self.failed) {
            return;
        }

        self.failed = !invoke(self.impl->on_load, self.error);
    }

    void ScriptHost::update(float delta)
    {
        ScriptHost& self = ScriptHost::get_singleton();

        if (!self.ready || self.failed) {
            return;
        }

        self.failed = !invoke(self.impl->on_update, self.error, delta);
    }

    void ScriptHost::draw()
    {
        ScriptHost& self = ScriptHost::get_singleton();

        if (!self.ready || self.failed) {
            return;
        }

        self.failed = !invoke(self.impl->on_draw, self.error);
    }

    void ScriptHost::quit()
    {
        ScriptHost& self = ScriptHost::get_singleton();

        if (!self.ready || self.failed) {
            return;
        }

        self.failed = !invoke(self.impl->on_quit, self.error);
    }

    // --- LIFE CYCLE --- //
    bool ScriptHost::compose(const std::string& entry)
    {
        ScriptHost& self = ScriptHost::get_singleton();

        // RUNTIME GUARD: This function should only be able to run when no state is open yet.
        if (self.ready) {
            return true;
        }

        self.impl = std::make_unique<Impl>();
        sol::state& lua = self.impl->lua;

        // Everything a game needs, without `io` or the parts of `os` that reach the file system:
        //      a game's own files come out of the mounted game instead, which is the only thing
        //      that works the same once the game ships inside an app.
        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::string,
            sol::lib::table,
            sol::lib::math,
            sol::lib::os,
            sol::lib::debug
        );

        // Every protected function made from here on picks this up, so a script error carries a
        //      traceback through the game's own frames instead of only naming what went wrong.
        self.impl->traceback = lua["debug"]["traceback"];
        sol::protected_function::set_default_handler(self.impl->traceback);

        Bindings::install(lua);

        // Lua's own searchers look in the file system and for C libraries, neither of which a
        //      game can reach, so `require` is pointed at the mounted game instead. The first
        //      searcher is left alone, since that's the one that finds preloaded modules.
        sol::table searchers = lua["package"]["searchers"];
        searchers[2] = [](sol::this_state state, const std::string& name) -> sol::object {
            sol::state_view lua(state);
            const std::string path = to_module_path(name);

            std::string source;
            if (!GameSource::read(path, source)) {
                // `require` collects these and shows them all when no searcher finds the module.
                return sol::make_object(lua, "\n\tno file '" + path + "' in the game");
            }

            // '@' marks the name as a file, so errors in the module point at its path.
            sol::load_result chunk = lua.load(source, "@" + path);
            if (!chunk.valid()) {
                const sol::error error = chunk;
                return sol::make_object(lua, std::string("\n\t") + error.what());
            }

            return chunk.get<sol::function>();
        };
        searchers[3] = sol::nil;
        searchers[4] = sol::nil;

        std::string source;
        if (!GameSource::read(entry, source)) {
            self.error = "Couldn't read " + entry + " from " + GameSource::get_description() + ".";
            self.failed = true;
            return false;
        }

        sol::load_result chunk = lua.load(source, "@" + entry);
        if (!chunk.valid()) {
            const sol::error error = chunk;
            self.error = error.what();
            self.failed = true;
            return false;
        }

        sol::protected_function entry_function = chunk;
        sol::protected_function_result result = entry_function();
        if (!result.valid()) {
            const sol::error error = result;
            self.error = error.what();
            self.failed = true;
            return false;
        }

        // The game fills these in while its entry script runs, the way a C++ game overrides the
        //      matching virtuals on `Rivet::Application`.
        sol::table rivet = lua["rivet"];
        self.impl->on_load = rivet["load"];
        self.impl->on_update = rivet["update"];
        self.impl->on_draw = rivet["draw"];
        self.impl->on_quit = rivet["quit"];

        self.ready = true;
        return true;
    }

    void ScriptHost::dispose()
    {
        ScriptHost& self = ScriptHost::get_singleton();

        // The handles hold references into the state, so they go before it does.
        self.impl.reset();

        self.ready = false;
    }
}
