#pragma once

#include <string>

namespace RivetRuntime
{
    // Where the game being run comes from, and the only way the runtime reads its files. A game is
    //      either a folder or a `.rivet` archive (a zip), and both are mounted into one read-only
    //      file system, so scripts address their files the same way wherever the game is shipped.
    class GameSource
    {
        private:
            static GameSource& get_singleton();
        private: // --- PROPERTIES --- //
            std::string description;
            std::string error;
        private: // --- CONTROL VARIABLES --- //
            bool ready;
        private:
            GameSource();
        public:
            ~GameSource();
            GameSource(const GameSource&) = delete;
            GameSource(GameSource&&) = delete;
            GameSource& operator=(const GameSource&) = delete;
            GameSource& operator=(GameSource&&) = delete;
        private: // --- UTILITY --- //
            bool mount_path(const std::string& path);
            bool mount_bundled();
        public: // --- GETTERS --- //
            static bool is_ready();
            // Where the mounted game was found, for logs and the error screen.
            static std::string get_description();
            // Why mounting failed, when it did.
            static std::string get_error();
        public: // --- FILES --- //
            static bool exists(const std::string& path);
            static bool read(const std::string& path, std::string& out_contents);
        public: // --- LIFE CYCLE --- //
            static void compose();
            static void dispose();
    };
}
