#pragma once

#include <Rivet/core/StretchAspect.hpp>
#include <Rivet/core/StretchMode.hpp>

#include <memory>

namespace Rivet
{
    class Viewport
    {
        private:
            static Viewport& get_singleton();
        private: // --- PROPERTIES --- //
            unsigned int base_width;
            unsigned int base_height;
            StretchMode stretch_mode;
            StretchAspect stretch_aspect;
            float stretch_scale;
            bool pixel_filter;
            bool integer_scaling;
        private: // --- CONTROL VARIABLES --- //
            unsigned int width;
            unsigned int height;
            bool ready;
            struct Impl;
            std::unique_ptr<Impl> impl;
        private:
            Viewport();
        public:
            ~Viewport();
            Viewport(const Viewport&) = delete;
            Viewport(Viewport&&) = delete;
            Viewport& operator=(const Viewport&) = delete;
            Viewport& operator=(Viewport&&) = delete;
        private: // --- UTILITY --- //
            bool layout();
            void load_target(int width, int height);
        public: // --- GETTERS --- //
            // The size the game is laid out in. This starts as the size given to `set_size()`,
            //      but the stretch settings can grow it to fit the window's shape.
            static unsigned int get_width();
            static unsigned int get_height();
            // The size given to `set_size()`.
            static unsigned int get_base_width();
            static unsigned int get_base_height();
            static StretchMode get_stretch_mode();
            static StretchAspect get_stretch_aspect();
            static float get_stretch_scale();
            static bool is_pixel_filter();
            static bool is_integer_scaling();
        public: // --- SETTERS --- //
            static void set_size(unsigned int width, unsigned int height);
            static void set_stretch_mode(StretchMode stretch_mode);
            static void set_stretch_aspect(StretchAspect stretch_aspect);
            static void set_stretch_scale(float stretch_scale);
            static void set_pixel_filter(bool pixel_filter);
            static void set_integer_scaling(bool integer_scaling);
        public: // --- CONVERSION --- //
            // Converts a position on the window (eg. the mouse or a touch) to viewport units.
            static float screen_to_viewport_x(float x);
            static float screen_to_viewport_y(float y);
        public: // --- LIFE CYCLE --- //
            static void compose();
            static void update();
            static void dispose();
            static void begin();
            static void end();
            static void render();
    };
}
