#include <Rivet/utils/System.hpp>

namespace Rivet
{
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

}