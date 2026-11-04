#pragma once

#include <memory>
#include <string>

namespace RivetRuntime
{
    // The game's Lua state, and the callbacks the runtime drives it through. These stand in for the
    //      virtuals a C++ game overrides: `rivet.load` for `on_ready()`, `rivet.update` for
    //      `on_update()`, `rivet.draw` for `on_render()` and `rivet.quit` for `on_dispose()`.
    //
    //      A script error doesn't end the process. The host remembers it and stops calling into
    //      Lua, which leaves the runtime free to show it instead of the game.
    class ScriptHost
    {
        private:
            static ScriptHost& get_singleton();
        private: // --- CONTROL VARIABLES --- //
            bool ready;
            bool failed;
            std::string error;
            // sol2 is a heavy header, and the Lua state is of no interest outside this class, so
            //      both are kept out of this one.
            struct Impl;
            std::unique_ptr<Impl> impl;
        private:
            ScriptHost();
        public:
            ~ScriptHost();
            ScriptHost(const ScriptHost&) = delete;
            ScriptHost(ScriptHost&&) = delete;
            ScriptHost& operator=(const ScriptHost&) = delete;
            ScriptHost& operator=(ScriptHost&&) = delete;
        public: // --- GETTERS --- //
            static bool is_ready();
            // Whether a script has raised an error. Once it has, no more callbacks are made.
            static bool has_failed();
            static std::string get_error();
        public: // --- CALLBACKS --- //
            static void load();
            static void update(float delta);
            static void draw();
            static void quit();
        public: // --- LIFE CYCLE --- //
            // Opens the Lua state, installs the `rivet` table and runs the game's entry script.
            static bool compose(const std::string& entry);
            static void dispose();
    };
}
