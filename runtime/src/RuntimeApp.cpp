#include <Rivet.hpp>
#include <Rivet/EntryPoint.hpp>

#include <RivetRuntime/GameSource.hpp>
#include <RivetRuntime/ProjectConfig.hpp>
#include <RivetRuntime/ScriptHost.hpp>

#include <string>
#include <vector>

namespace
{
    // Breaks a message into lines that fit the width given, keeping the line breaks it already has.
    //      Words longer than the width are left to overrun rather than being cut apart.
    std::vector<std::string> wrap(const std::string& message, float width, float font_size)
    {
        std::vector<std::string> lines;
        std::string line;

        const auto flush = [&lines, &line]() {
            lines.push_back(line);
            line.clear();
        };

        const auto fits = [width, font_size](const std::string& text) {
            return MeasureTextEx(GetFontDefault(), text.c_str(), font_size, font_size / 10.0f).x <= width;
        };

        std::string word;
        for (std::size_t index = 0; index <= message.size(); ++index) {
            const char character = index < message.size() ? message[index] : '\n';

            if (character != ' ' && character != '\n' && character != '\t') {
                word += character;
                continue;
            }

            if (!word.empty()) {
                const std::string candidate = line.empty() ? word : line + " " + word;
                if (fits(candidate) || line.empty()) {
                    line = candidate;
                }
                else {
                    flush();
                    line = word;
                }
                word.clear();
            }

            if (character == '\n') {
                flush();
            }
        }

        // The loop ends on a line break, which leaves a trailing empty line behind.
        if (!lines.empty() && lines.back().empty()) {
            lines.pop_back();
        }

        return lines;
    }
}

// The runtime: a game whose behaviour comes from a Lua script rather than from C++. The callbacks
//      a script sets on the `rivet` table stand in for the virtuals below, so a Lua game and a
//      C++ game run through exactly the same application lifecycle.
class RuntimeApp : public Rivet::Application
{
    private:
        std::string startup_error;
        bool error_logged = false;
    private:
        // The first thing that went wrong, if anything did. Mounting the game, reading its
        //      project file and running its scripts can each fail, and the screen only shows one.
        std::string get_error() const
        {
            if (!this->startup_error.empty()) {
                return this->startup_error;
            }

            return RivetRuntime::ScriptHost::get_error();
        }

        bool has_error() const
        {
            return !this->startup_error.empty() || RivetRuntime::ScriptHost::has_failed();
        }

        void render_error() const
        {
            const float screen_width = static_cast<float>(Rivet::Display::get_width());
            const float margin = 32.0f;
            const float title_size = 28.0f;
            const float body_size = 18.0f;

            Rivet::Renderer::clear(0.11f, 0.13f, 0.28f);

            DrawTextEx(GetFontDefault(), "Rivet couldn't run this game", { margin, margin }, title_size, title_size / 10.0f, ::WHITE);

            float y = margin + title_size + 24.0f;
            const ::Color body_color = { 200, 205, 230, 255 };
            for (const std::string& line : wrap(this->get_error(), screen_width - margin * 2.0f, body_size)) {
                DrawTextEx(GetFontDefault(), line.c_str(), { margin, y }, body_size, body_size / 10.0f, body_color);
                y += body_size + 6.0f;
            }

            const ::Color hint_color = { 140, 146, 180, 255 };
            DrawTextEx(GetFontDefault(), "Press Escape to quit.", { margin, y + 18.0f }, body_size, body_size / 10.0f, hint_color);
        }
    protected:
        void on_compose() override
        {
            Rivet::Renderer::set_default_clear_color(0.0f, 0.0f, 0.0f);

            // What the runtime shows before a project is read, and what it keeps if there is no
            //      project to read: a window big enough to hold an error message.
            Rivet::Display::set_title("Rivet");
            Rivet::Display::set_size(960, 540);
            Rivet::Display::set_resizable(true);
            Rivet::Display::set_vsync(true);

            RivetRuntime::GameSource::compose();
            if (!RivetRuntime::GameSource::is_ready()) {
                this->startup_error = RivetRuntime::GameSource::get_error();
                return;
            }

            // The project settles the display and the viewport, so it's read before the window
            //      is created rather than from a script afterwards.
            if (!RivetRuntime::ProjectConfig::compose()) {
                this->startup_error = RivetRuntime::ProjectConfig::get_error();
            }
        }

        void on_ready() override
        {
            // RUNTIME GUARD: Without a game there is nothing to run, and the error is already
            //      waiting to be drawn.
            if (this->has_error()) {
                return;
            }

            TraceLog(LOG_INFO, "RIVET: running %s from %s",
                RivetRuntime::ProjectConfig::get_name().c_str(),
                RivetRuntime::GameSource::get_description().c_str()
            );

            if (!RivetRuntime::ScriptHost::compose(RivetRuntime::ProjectConfig::get_entry())) {
                return;
            }

            RivetRuntime::ScriptHost::load();
        }

        void on_update(float delta) override
        {
            if (this->has_error()) {
                // Reported once as well as drawn, so whatever started the runtime sees the error
                //      without having to look at the window.
                if (!this->error_logged) {
                    TraceLog(LOG_ERROR, "RIVET: %s", this->get_error().c_str());
                    this->error_logged = true;
                }

                // The error screen is the whole game at this point, so it gets the one key it
                //      needs. A running game decides for itself what Escape does.
                if (IsKeyPressed(KEY_ESCAPE)) {
                    Rivet::Application::quit();
                }
                return;
            }

            RivetRuntime::ScriptHost::update(delta);
        }

        void on_render(Rivet::RenderPass pass) override
        {
            // The error screen is drawn to the window rather than into the viewport, so it isn't
            //      scaled or letterboxed by a project whose settings may be what went wrong.
            if (this->has_error()) {
                if (pass == Rivet::RenderPass::SCREEN) {
                    this->render_error();
                }
                return;
            }

            if (pass == Rivet::RenderPass::VIEWPORT) {
                RivetRuntime::ScriptHost::draw();
            }
        }

        void on_dispose() override
        {
            RivetRuntime::ScriptHost::quit();
            RivetRuntime::ScriptHost::dispose();
            RivetRuntime::GameSource::dispose();
        }
};

Rivet::Application* Rivet::create_application()
{
    return new RuntimeApp();
}
