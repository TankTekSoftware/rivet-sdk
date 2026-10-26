#include <Rivet/core/Viewport.hpp>

#include <Rivet/core/Display.hpp>

#include <raylib.h>

#include <cmath>
#include <algorithm>

namespace Rivet
{
    Viewport& Viewport::get_singleton()
    {
        static Viewport singleton;
        return singleton;
    }

    Viewport::Viewport()
    {
        unsigned int width = 0;
        unsigned int height = 0;

        // RUNTIME GUARD: if the display is ready, then use the display size as the 
        //      default viewport size.
        if (Display::is_ready()) {
            width = Display::get_width();
            height = Display::get_height();
        } 
        
        // Assign the default viewport properties.
        this->width = width;
        this->height = height;
        this->pixel_filter = false;

        // Assign
        this->target = { };
        this->dest = { }; 

        this->ready = false;
    }

    Viewport::~Viewport()
    {
        this->dispose();
    }

    // --- GETTERS --- //
    unsigned int Viewport::get_width()
    {
        Viewport& self = Viewport::get_singleton();
        return self.width;
    }

    unsigned int Viewport::get_height()
    {
        Viewport& self = Viewport::get_singleton();
        return self.height;
    }

    bool Viewport::is_pixel_filter()
    {
        Viewport& self = Viewport::get_singleton();
        return self.pixel_filter;
    }

    bool Viewport::is_integer_scaling()
    {
        Viewport& self = Viewport::get_singleton();
        return self.integer_scaling;
    }

    // --- SETTERS --- //
    void Viewport::set_size(unsigned int width, unsigned int height)
    {
        Viewport& self = Viewport::get_singleton();
        self.width = width;
        self.height = height;   
    }

    void Viewport::set_pixel_filter(bool pixel_filter)
    {
        Viewport& self = Viewport::get_singleton();
        self.pixel_filter = pixel_filter;
    }

    void Viewport::set_integer_scaling(bool integer_scaling)
    {
        Viewport& self = Viewport::get_singleton();
        self.integer_scaling = integer_scaling;
    }

    // --- LIFE CYCLE --- //
    void Viewport::compose()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: This function should not run if the display has not been
        //      created yet.
        if (!Display::is_ready()) {
            return;
        }

        // RUNTIME GUARD: This function should not run if the viewport has already been
        //      setup.
        if (self.ready) {
            return;
        }

        self.target = LoadRenderTexture(self.width, self.height);
        if (self.is_pixel_filter) {
            SetTextureFilter(self.target.texture, TEXTURE_FILTER_POINT);
        }
    }

    void Viewport::update()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        float display_width = static_cast<float>(Display::get_width());
        float display_height = static_cast<float>(Display::get_height());

        float scale = std::min(display_width / self.width, display_height / self.height);
        if (self.integer_scaling) {
            scale = std::floorf(scale);
        }

        self.dest = {
            (display_width - self.width * scale) * 0.5f,
            (display_height - self.height * scale) * 0.5f,
            self.width * scale,
            self.height * scale
        };
    }

    void Viewport::dispose()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: This function should not run if the viewport has not been created,
        //      or is already disposed of.
        if (!self.ready) {
            return;
        }

        UnloadRenderTexture(self.target);
    }

    void Viewport::begin()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        BeginTextureMode(self.target);   
        ClearBackground(BLACK);
    }

    void Viewport::end()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        EndTextureMode();
    }

    void Viewport::render()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        const Rectangle source = {
            0.0f,                               // x
            0.0f,                               // y
            static_cast<float>(self.width),     // width
            static_cast<float>(self.height)     // height
        };

        const Vector2 origin = { 0.0f, 0.0f };
        const  float rotation = 0.0f;
        const Color tint = WHITE;

        DrawTexturePro(self.target.texture, source, self.dest, origin, rotation, tint);
    }
}