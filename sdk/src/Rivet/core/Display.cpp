#include <Rivet/core/Display.hpp>

#include <Rivet/utils/System.hpp>

#include <raylib.h>

namespace Rivet
{
    Display& Display::get_singleton()
    {
        static Display singleton;
        return singleton;
    }

    Display::Display()
    {
        this->width = 800;
        this->height = 600;
        this->title = "RivetGame";
        this->fullscreen = false;
        this->resizable = false;
        this->vsync = true;

        this->ready = false;
        this->flags = 0;
    }

    Display::~Display()
    {
        this->dispose();
    }

    // --- UTILITY --- //
    bool Display::create()
    {
        SetConfigFlags(this->flags);
        InitWindow(this->width, this->height, this->title.c_str());

        return IsWindowReady();
    }
    
    void Display::destroy()
    {
        CloseWindow();
    }

    // --- GETTERS --- //
    unsigned int Display::get_width()
    {
        Display& self = Display::get_singleton();
        return self.width;
    }

    unsigned int Display::get_height()
    {
        Display& self = Display::get_singleton();
        return self.height;
    }

    std::string Display::get_title()
    {
        Display& self = Display::get_singleton();
        return self.title;
    }

    bool Display::is_fullscreen()
    {
        Display& self = Display::get_singleton();
        return self.fullscreen;
    }

    bool Display::is_resizable()
    {
        Display& self = Display::get_singleton();
        return self.resizable;
    }

    bool Display::is_vsync()
    {
        Display& self = Display::get_singleton();
        return self.vsync;
    }

    bool Display::is_ready()
    {
        Display& self = Display::get_singleton();
        return self.ready;
    }

    // --- SETTERS --- //
    void Display::set_size(unsigned int width, unsigned int height)
    {
        Display& self = Display::get_singleton();
        self.width = width;
        self.height = height;

        if (self.ready) {
            SetWindowSize(self.width, self.height);
        }
    }

    void Display::set_title(const std::string& title)
    {
        Display& self = Display::get_singleton();
        self.title = title;

        if (self.ready) {
            SetWindowTitle(self.title.c_str());
        }
    }

    void Display::set_fullscreen(bool fullscreen)
    {
        Display& self = Display::get_singleton();
        self.fullscreen = fullscreen;

        if (self.ready) {
            if (self.fullscreen) {
                SetWindowState(FLAG_FULLSCREEN_MODE);
            } else {
                ClearWindowState(FLAG_FULLSCREEN_MODE);
            }
        }
    }

    void Display::set_resizable(bool resizable)
    {
        Display& self = Display::get_singleton();
        self.resizable = resizable;

        if (self.ready) {
            if (self.resizable) {
                SetWindowState(FLAG_WINDOW_RESIZABLE);
            } else {
                ClearWindowState(FLAG_WINDOW_RESIZABLE);
            }
        }
    }
 
    void Display::set_vsync(bool vsync)
    {
        Display& self = Display::get_singleton();
        self.vsync = vsync;

        if (self.ready) {
            if (self.vsync) {
                SetWindowState(FLAG_VSYNC_HINT);
            } else {
                ClearWindowState(FLAG_VSYNC_HINT);
            }
        }
    }

    // --- LIFE CYCLE --- //
    void Display::compose()
    {
        Display& self = Display::get_singleton();

        // RUNTIME GUARD: This function should only be able to run when
        //      the display is not active (eg. the window is closed).  
        if (self.ready) {
            return;
        }

        // PLATFORM CHECK: These are settings that need to be overridden on mobile platforms
        //      (eg. iPhone/iPad, Android phones/tablets), where the window always fills the screen.
        //      A size of 0 has raylib draw at the screen's own resolution; on Android, any other size
        //      is drawn at that size and then stretched over the screen.
        const std::string_view os_name = System::get_os_name();
        if (os_name == "ios" || os_name == "android") {
            self.width = 0;   
            self.height = 0;
            self.flags |= FLAG_FULLSCREEN_MODE;
        }

        if (self.fullscreen) {
            self.flags |= FLAG_FULLSCREEN_MODE;
        }

        if (self.resizable) {
            self.flags |= FLAG_WINDOW_RESIZABLE;
        }

        if (self.vsync) {
            self.flags |= FLAG_VSYNC_HINT;
        }

        // Creates the window and assigns the result to `self.ready`
        self.ready = self.create();

        // The platform can create the window at a different size than requested (eg. iOS
        //      always fills the screen), so start from the size the window actually has.
        Display::update();
    }

    void Display::update()
    {
        Display& self = Display::get_singleton();

        // RUNTIME GUARD: This function should only be able to run when
        //      the display is currently active (eg. the window is opened).
        if (!self.ready) {
            return;
        }

        // The window can change size without going through `set_size()` (eg. a resizable
        //      window, or rotating an iPhone/iPad), so keep the stored size in step with it.
        self.width = static_cast<unsigned int>(GetScreenWidth());
        self.height = static_cast<unsigned int>(GetScreenHeight());
    }

    void Display::dispose()
    {
        Display& self = Display::get_singleton();

        // RUNTIME GUARD: This function should only be able to run when
        //      the display is currently active (eg. the window is opened).
        if (!self.ready) {
            return;
        }

        self.destroy();

        self.ready = false; 
    }

    bool Display::should_close()
    {
        Display& self = Display::get_singleton();
        
        // RUNTIME GUARD: This function should return `true` if the window
        //      is not ready.
        if (!self.ready) {
            return true;
        }

        return WindowShouldClose();
    }
}