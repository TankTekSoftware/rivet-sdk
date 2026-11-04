#include <RivetRuntime/Bindings.hpp>

#include <Rivet/core/Application.hpp>
#include <Rivet/core/Display.hpp>
#include <Rivet/core/Viewport.hpp>

#include <Rivet/render/Color.hpp>
#include <Rivet/render/Renderer.hpp>

#include <Rivet/utils/System.hpp>
#include <Rivet/utils/Time.hpp>

#include <raylib.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <tuple>

namespace
{
    // GRAPHICS STATE: Scripts set a color and then draw with it, rather than passing one to every
    //      call. The SDK has no drawing of its own yet, so the color is kept here in the shape
    //      raylib takes.
    ::Color current_color = { 255, 255, 255, 255 };

    unsigned char to_channel(float value)
    {
        const float clamped = std::clamp(value, 0.0f, 1.0f);
        return static_cast<unsigned char>(clamped * 255.0f);
    }

    ::Color to_rlcolor(float r, float g, float b, float a)
    {
        return { to_channel(r), to_channel(g), to_channel(b), to_channel(a) };
    }

    float from_channel(unsigned char value)
    {
        return static_cast<float>(value) / 255.0f;
    }

    // "fill" draws a solid shape and "line" draws its outline, as in `rivet.graphics.rectangle()`.
    bool is_fill(const std::string& mode)
    {
        return mode != "line";
    }

    // The key names scripts use. Single characters cover the letter and number keys, so only the
    //      rest have to be named.
    int to_key(const std::string& name)
    {
        if (name.size() == 1) {
            const unsigned char character = static_cast<unsigned char>(name.front());
            if (std::isalpha(character)) {
                return KEY_A + (std::toupper(character) - 'A');
            }
            if (std::isdigit(character)) {
                return KEY_ZERO + (character - '0');
            }
        }

        if (name == "up") { return KEY_UP; }
        if (name == "down") { return KEY_DOWN; }
        if (name == "left") { return KEY_LEFT; }
        if (name == "right") { return KEY_RIGHT; }
        if (name == "space") { return KEY_SPACE; }
        if (name == "escape") { return KEY_ESCAPE; }
        if (name == "enter" || name == "return") { return KEY_ENTER; }
        if (name == "tab") { return KEY_TAB; }
        if (name == "backspace") { return KEY_BACKSPACE; }
        if (name == "lshift") { return KEY_LEFT_SHIFT; }
        if (name == "rshift") { return KEY_RIGHT_SHIFT; }
        if (name == "lctrl") { return KEY_LEFT_CONTROL; }
        if (name == "rctrl") { return KEY_RIGHT_CONTROL; }

        // An unknown name is no key, which no device can report as held.
        return -1;
    }

    int to_mouse_button(const std::string& name)
    {
        if (name == "right") { return MOUSE_BUTTON_RIGHT; }
        if (name == "middle") { return MOUSE_BUTTON_MIDDLE; }

        return MOUSE_BUTTON_LEFT;
    }
}

namespace RivetRuntime
{
    void Bindings::install(sol::state_view lua)
    {
        sol::table rivet = lua.create_named_table("rivet");

        rivet.set_function("quit", []() {
            Rivet::Application::quit();
        });

        // --- GRAPHICS --- //
        sol::table graphics = rivet.create_named("graphics");

        graphics.set_function("set_color", [](float r, float g, float b, sol::optional<float> a) {
            current_color = to_rlcolor(r, g, b, a.value_or(1.0f));
        });

        graphics.set_function("get_color", []() {
            return std::make_tuple(
                from_channel(current_color.r),
                from_channel(current_color.g),
                from_channel(current_color.b),
                from_channel(current_color.a)
            );
        });

        graphics.set_function("clear", [](sol::optional<float> r, sol::optional<float> g, sol::optional<float> b, sol::optional<float> a) {
            // Without a color, the project's own clear color is used.
            if (!r.has_value() || !g.has_value() || !b.has_value()) {
                Rivet::Renderer::clear();
                return;
            }

            Rivet::Renderer::clear(Rivet::Color(*r, *g, *b, a.value_or(1.0f)));
        });

        graphics.set_function("rectangle", [](const std::string& mode, float x, float y, float width, float height) {
            if (is_fill(mode)) {
                DrawRectangleV({ x, y }, { width, height }, current_color);
                return;
            }

            DrawRectangleLinesEx({ x, y, width, height }, 1.0f, current_color);
        });

        graphics.set_function("circle", [](const std::string& mode, float x, float y, float radius) {
            if (is_fill(mode)) {
                DrawCircleV({ x, y }, radius, current_color);
                return;
            }

            DrawCircleLinesV({ x, y }, radius, current_color);
        });

        graphics.set_function("ring", [](float x, float y, float inner_radius, float outer_radius) {
            DrawRing({ x, y }, inner_radius, outer_radius, 0.0f, 360.0f, 0, current_color);
        });

        graphics.set_function("line", [](float from_x, float from_y, float to_x, float to_y, sol::optional<float> thickness) {
            DrawLineEx({ from_x, from_y }, { to_x, to_y }, thickness.value_or(1.0f), current_color);
        });

        graphics.set_function("print", [](const std::string& text, float x, float y, sol::optional<float> size) {
            const float font_size = size.value_or(20.0f);
            DrawTextEx(GetFontDefault(), text.c_str(), { x, y }, font_size, font_size / 10.0f, current_color);
        });

        graphics.set_function("get_text_width", [](const std::string& text, sol::optional<float> size) {
            const float font_size = size.value_or(20.0f);
            return MeasureTextEx(GetFontDefault(), text.c_str(), font_size, font_size / 10.0f).x;
        });

        // The size the game is laid out in, which is what it draws against.
        graphics.set_function("get_width", []() {
            return static_cast<float>(Rivet::Viewport::get_width());
        });

        graphics.set_function("get_height", []() {
            return static_cast<float>(Rivet::Viewport::get_height());
        });

        graphics.set_function("debug_fps", [](int x, int y) {
            Rivet::Renderer::debug_fps(x, y);
        });

        // --- KEYBOARD --- //
        sol::table keyboard = rivet.create_named("keyboard");

        keyboard.set_function("is_down", [](const std::string& name) {
            const int key = to_key(name);
            return key >= 0 && IsKeyDown(key);
        });

        keyboard.set_function("is_pressed", [](const std::string& name) {
            const int key = to_key(name);
            return key >= 0 && IsKeyPressed(key);
        });

        // --- MOUSE --- //
        sol::table mouse = rivet.create_named("mouse");

        // Reported in viewport units, so a script never has to know how the viewport is laid out
        //      over the window.
        mouse.set_function("get_position", []() {
            return std::make_tuple(
                Rivet::Viewport::screen_to_viewport_x(static_cast<float>(GetMouseX())),
                Rivet::Viewport::screen_to_viewport_y(static_cast<float>(GetMouseY()))
            );
        });

        mouse.set_function("is_down", [](const std::string& button) {
            return IsMouseButtonDown(to_mouse_button(button));
        });

        // --- TOUCH --- //
        sol::table touch = rivet.create_named("touch");

        touch.set_function("get_count", []() {
            return GetTouchPointCount();
        });

        // Lua counts from one, so the first touch is 1.
        touch.set_function("get_position", [](int index) {
            const Vector2 position = GetTouchPosition(index - 1);
            return std::make_tuple(
                Rivet::Viewport::screen_to_viewport_x(position.x),
                Rivet::Viewport::screen_to_viewport_y(position.y)
            );
        });

        // --- TIMER --- //
        sol::table timer = rivet.create_named("timer");

        timer.set_function("get_delta", []() {
            return Rivet::Time::get_delta();
        });

        timer.set_function("get_fps", []() {
            return GetFPS();
        });

        // --- WINDOW --- //
        sol::table window = rivet.create_named("window");

        window.set_function("get_width", []() {
            return Rivet::Display::get_width();
        });

        window.set_function("get_height", []() {
            return Rivet::Display::get_height();
        });

        window.set_function("get_title", []() {
            return Rivet::Display::get_title();
        });

        // --- SYSTEM --- //
        sol::table system = rivet.create_named("system");

        system.set_function("get_os", []() {
            return std::string(Rivet::System::get_os_name());
        });
    }
}
