#pragma once

#include <string>

namespace Rivet
{
    class Display
    {
        private:
            static Display& get_singleton();
        private: // --- PROPERTIES --- //
            unsigned int width;
            unsigned int height;
            std::string title;
            bool fullscreen;
            bool resizable;
            bool vsync;
        private: // --- CONTROL VARIABLES --- //
            bool ready;
            unsigned int flags;
        private: // --- CONSTRUCTOR --- //
            Display();
        public: // --- DESTRUCTOR / SINGLETON GUARD --- /
            ~Display();
            Display(const Display&) = delete;
            Display(Display&&) = delete;
            Display& operator=(const Display&) = delete;
            Display& operator=(Display&&) = delete;
        private: // --- UTILITY --- //
            bool create();
            void destroy();
        public: // --- GETTERS --- //
            static unsigned int get_width();
            static unsigned int get_height();
            static std::string get_title();
            static bool get_vsync();
            static bool is_ready();
        public: // --- SETTERS --- //
            static void set_size(unsigned int width, unsigned int height);
            static void set_title(const std::string& title);
            static void set_vsync(bool vsync);
        public: // --- LIFE CYCLE --- //
            static void compose();
            static void dispose();
            static bool should_close();
    }; 
}