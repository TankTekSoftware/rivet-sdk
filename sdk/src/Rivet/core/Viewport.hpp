#pragma once

// TODO: This include should be removed later.
//      RenderTexture2D and Rectangle needs to be abstracted to a Rivet class.
#include <raylib.h>

namespace Rivet
{
    class Viewport
    {
        private:
            static Viewport& get_singleton();
        private:
            unsigned int width;
            unsigned int height;
            bool pixel_filter;
            bool integer_scaling;
        private:
            bool ready;
            RenderTexture2D target;
            Rectangle dest;
        private:
            Viewport();
        public:
            ~Viewport();
            Viewport(const Viewport&) = delete;
            Viewport(Viewport&&) = delete;
            Viewport& operator=(const Viewport&) = delete;
            Viewport& operator=(Viewport&&) = delete;
        public: // --- GETTERS --- //
            static unsigned int get_width();
            static unsigned int get_height();
            static bool is_pixel_filter();
            static bool is_integer_scaling();
        public: // --- SETTERS --- //
            static void set_size(unsigned int width, unsigned int height);
            static void set_pixel_filter(bool pixel_filter);
            static void set_integer_scaling(bool integer_scaling);
        public: // --- LIFE CYCLE --- //
            static void compose();
            static void update();
            static void dispose();
            static void begin();
            static void end();
            static void render();
    };
}