#pragma once

#include <Rivet/core/StretchAspect.hpp>
#include <Rivet/core/StretchMode.hpp>

#include <string>

namespace RivetRuntime
{
    // The game's `project.rivet`, which settles everything the runtime has to know before the
    //      window exists. It's read rather than scripted, so the display and the viewport are set
    //      up once, in `on_compose()`, without a Lua state having to run first.
    class ProjectConfig
    {
        private:
            static ProjectConfig& get_singleton();
        private: // --- GENERAL --- //
            std::string name;
            std::string entry;
        private: // --- DISPLAY --- //
            unsigned int display_width;
            unsigned int display_height;
            bool fullscreen;
            bool resizable;
            bool vsync;
        private: // --- VIEWPORT --- //
            unsigned int viewport_width;
            unsigned int viewport_height;
            Rivet::StretchMode stretch_mode;
            Rivet::StretchAspect stretch_aspect;
            float stretch_scale;
            bool integer_scaling;
            bool pixel_filter;
        private: // --- CONTROL VARIABLES --- //
            std::string error;
        private:
            ProjectConfig();
        public: // --- GETTERS --- //
            static std::string get_name();
            // The script the runtime runs first, "main.lua" unless the project names another.
            static std::string get_entry();
            static std::string get_error();
        public: // --- LIFE CYCLE --- //
            // Reads `project.rivet` from the mounted game, and applies what it holds to the
            //      display and the viewport. Every key is optional: a project that gives none
            //      runs at the defaults, like a game that only ships a `main.lua`.
            static bool compose();
    };
}
