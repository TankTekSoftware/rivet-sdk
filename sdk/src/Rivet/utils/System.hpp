#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Rivet
{
    class System
    {
        private:
            static System& get_singleton();
        private: // --- PROPERTIES --- //
            std::vector<std::string> args;
        private:
            System();
        public:
            static std::string_view get_os_name();
        public: // --- COMMAND LINE --- //
            // The arguments the game was started with, without the program's own name. Only
            //      desktop platforms start a game with arguments; on iOS and Android this is empty.
            static const std::vector<std::string>& get_args();
            // ENTRY POINT ONLY: Called by `Rivet/EntryPoint.hpp` on the platforms that have a
            //      `main()`, so the rest of the game can read the arguments without being passed them.
            static void set_args(int argc, char* argv[]);
    };
}
