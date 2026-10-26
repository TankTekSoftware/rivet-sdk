#pragma once

#include <Rivet/core/Application.hpp>

extern Rivet::Application* Rivet::create_application();

#if defined(PLATFORM_DESKTOP)

int main(int argc, char* argv[])
{
    Rivet::Application* app = Rivet::create_application();
    app->run();
    delete app;
    return 0;
}

#endif