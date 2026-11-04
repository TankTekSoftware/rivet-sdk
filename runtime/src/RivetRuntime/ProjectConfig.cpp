#include <RivetRuntime/ProjectConfig.hpp>

#include <RivetRuntime/GameSource.hpp>

#include <Rivet/core/Display.hpp>
#include <Rivet/core/Viewport.hpp>

#include <toml++/toml.hpp>

#include <string_view>

namespace
{
    const char* PROJECT_FILE = "project.rivet";

    Rivet::StretchMode to_stretch_mode(std::string_view name, Rivet::StretchMode fallback)
    {
        if (name == "disabled") {
            return Rivet::StretchMode::DISABLED;
        }
        if (name == "canvas_items") {
            return Rivet::StretchMode::CANVAS_ITEMS;
        }
        if (name == "viewport") {
            return Rivet::StretchMode::VIEWPORT;
        }

        return fallback;
    }

    Rivet::StretchAspect to_stretch_aspect(std::string_view name, Rivet::StretchAspect fallback)
    {
        if (name == "ignore") {
            return Rivet::StretchAspect::IGNORE;
        }
        if (name == "keep") {
            return Rivet::StretchAspect::KEEP;
        }
        if (name == "keep_width") {
            return Rivet::StretchAspect::KEEP_WIDTH;
        }
        if (name == "keep_height") {
            return Rivet::StretchAspect::KEEP_HEIGHT;
        }
        if (name == "expand") {
            return Rivet::StretchAspect::EXPAND;
        }

        return fallback;
    }
}

namespace RivetRuntime
{
    ProjectConfig& ProjectConfig::get_singleton()
    {
        static ProjectConfig singleton;
        return singleton;
    }

    ProjectConfig::ProjectConfig()
    {
        this->name = "RivetGame";
        this->entry = "main.lua";

        // The same defaults the SDK starts at, so a project only has to name what it wants
        //      changed. The viewport sizes of 0 leave it following the window.
        this->display_width = 800;
        this->display_height = 600;
        this->fullscreen = false;
        this->resizable = false;
        this->vsync = true;

        this->viewport_width = 0;
        this->viewport_height = 0;
        this->stretch_mode = Rivet::StretchMode::VIEWPORT;
        this->stretch_aspect = Rivet::StretchAspect::KEEP;
        this->stretch_scale = 1.0f;
        this->integer_scaling = false;
        this->pixel_filter = false;
    }

    // --- GETTERS --- //
    std::string ProjectConfig::get_name()
    {
        ProjectConfig& self = ProjectConfig::get_singleton();
        return self.name;
    }

    std::string ProjectConfig::get_entry()
    {
        ProjectConfig& self = ProjectConfig::get_singleton();
        return self.entry;
    }

    std::string ProjectConfig::get_error()
    {
        ProjectConfig& self = ProjectConfig::get_singleton();
        return self.error;
    }

    // --- LIFE CYCLE --- //
    bool ProjectConfig::compose()
    {
        ProjectConfig& self = ProjectConfig::get_singleton();

        std::string contents;
        if (GameSource::exists(PROJECT_FILE) && !GameSource::read(PROJECT_FILE, contents)) {
            self.error = std::string("Couldn't read ") + PROJECT_FILE + ".";
            return false;
        }

        // A game without a project file runs at the defaults, so there is nothing to parse.
        if (!contents.empty()) {
            toml::table table;
            try {
                table = toml::parse(contents, std::string_view(PROJECT_FILE));
            }
            catch (const toml::parse_error& parse_error) {
                const toml::source_position where = parse_error.source().begin;
                self.error = std::string(PROJECT_FILE) + ":" + std::to_string(where.line) + ":"
                    + std::to_string(where.column) + ": " + std::string(parse_error.description());
                return false;
            }

            const toml::node_view<toml::node> general = table["general"];
            self.name = general["name"].value_or(self.name);
            self.entry = general["entry"].value_or(self.entry);

            const toml::node_view<toml::node> display = table["display"];
            self.display_width = display["width"].value_or(self.display_width);
            self.display_height = display["height"].value_or(self.display_height);
            self.fullscreen = display["fullscreen"].value_or(self.fullscreen);
            self.resizable = display["resizable"].value_or(self.resizable);
            self.vsync = display["vsync"].value_or(self.vsync);

            const toml::node_view<toml::node> viewport = table["viewport"];
            self.viewport_width = viewport["width"].value_or(self.viewport_width);
            self.viewport_height = viewport["height"].value_or(self.viewport_height);
            self.stretch_mode = to_stretch_mode(viewport["stretch_mode"].value_or(""), self.stretch_mode);
            self.stretch_aspect = to_stretch_aspect(viewport["stretch_aspect"].value_or(""), self.stretch_aspect);
            self.stretch_scale = viewport["stretch_scale"].value_or(self.stretch_scale);
            self.integer_scaling = viewport["integer_scaling"].value_or(self.integer_scaling);
            self.pixel_filter = viewport["pixel_filter"].value_or(self.pixel_filter);
        }

        Rivet::Display::set_title(self.name);
        Rivet::Display::set_size(self.display_width, self.display_height);
        Rivet::Display::set_fullscreen(self.fullscreen);
        Rivet::Display::set_resizable(self.resizable);
        Rivet::Display::set_vsync(self.vsync);

        // A project that names no viewport size is left following the window, which is what the
        //      viewport does on its own when it has no base size.
        if (self.viewport_width > 0 && self.viewport_height > 0) {
            Rivet::Viewport::set_size(self.viewport_width, self.viewport_height);
        }

        Rivet::Viewport::set_stretch_mode(self.stretch_mode);
        Rivet::Viewport::set_stretch_aspect(self.stretch_aspect);
        Rivet::Viewport::set_stretch_scale(self.stretch_scale);
        Rivet::Viewport::set_integer_scaling(self.integer_scaling);
        Rivet::Viewport::set_pixel_filter(self.pixel_filter);

        return true;
    }
}
