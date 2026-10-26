#pragma once

#include <Rivet/core/Application.hpp>
#include <Rivet/core/Display.hpp>

extern Rivet::Application* Rivet::create_application();

#if defined(PLATFORM_DESKTOP)

int main(int argc, char* argv[])
{
    Rivet::Application* app = Rivet::create_application();
    app->run();
    delete app;
    return 0;
}

#elif defined(PLATFORM_IOS)

// PLATFORM ENTRY: iOS runs the game loop itself, so raylib's iOS backend owns main() and calls
//      these instead: once when the window can be created, on every frame, and on termination.
static Rivet::Application* ios_application = nullptr;

extern "C" void ios_ready()
{
    ios_application = Rivet::create_application();
    ios_application->create();
}

extern "C" void ios_update(bool view_size_changed)
{
    // RUNTIME GUARD: Each call is one pass of the desktop loop in `Application::run()`,
    //      so it stops in the same way once the display can't be drawn to.
    if (Rivet::Display::should_close()) {
        return;
    }

    ios_application->tick(view_size_changed);
}

extern "C" void ios_destroy()
{
    ios_application->destroy();
    delete ios_application;
    ios_application = nullptr;
}

#endif