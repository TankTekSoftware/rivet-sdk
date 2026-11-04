#include <Rivet/utils/System.hpp>

namespace Rivet
{
    System& System::get_singleton()
    {
        static System singleton;
        return singleton;
    }

    System::System()
    {

    }

    std::string_view System::get_os_name()
    {
#if defined(__EMSCRIPTEN__)
        return "web";
#elif defined(_WIN32)
        return "windows";
#elif defined(__APPLE__)
    #if TARGET_OS_IPHONE
        return "ios";
    #else
        return "macos";
    #endif
#elif defined(__ANDROID__)
        return "android";
#elif defined(__linux__)
        return "linux";
#else
        return "unknown";
#endif
    }

    // --- COMMAND LINE --- //
    const std::vector<std::string>& System::get_args()
    {
        System& self = System::get_singleton();
        return self.args;
    }

    void System::set_args(int argc, char* argv[])
    {
        System& self = System::get_singleton();
        self.args.clear();

        // The program's own name is argv[0], which isn't an argument the game was given.
        for (int index = 1; index < argc; ++index) {
            self.args.emplace_back(argv[index]);
        }
    }
}
