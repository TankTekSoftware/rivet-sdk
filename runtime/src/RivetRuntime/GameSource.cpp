#include <RivetRuntime/GameSource.hpp>

#include <Rivet/utils/System.hpp>

#include <physfs.h>
#include <raylib.h>

namespace
{
    // The file a fused build keeps its game in, next to the executable on desktop and inside the
    //      app on iOS and Android.
    const char* BUNDLED_ARCHIVE = "game.rivet";
    // The folder a fused build can keep its game in instead, which suits shipping it unpacked.
    const char* BUNDLED_FOLDER = "game";

    std::string physfs_error()
    {
        const PHYSFS_ErrorCode code = PHYSFS_getLastErrorCode();
        return PHYSFS_getErrorByCode(code);
    }

    // PhysFS frees memory it was mounted from through a callback of its own shape, and the memory
    //      came from raylib, so it has to go back to raylib.
    void free_file_data(void* data)
    {
        UnloadFileData(static_cast<unsigned char*>(data));
    }
}

namespace RivetRuntime
{
    GameSource& GameSource::get_singleton()
    {
        static GameSource singleton;
        return singleton;
    }

    GameSource::GameSource()
    {
        this->ready = false;
    }

    GameSource::~GameSource()
    {
        this->dispose();
    }

    // --- UTILITY --- //
    bool GameSource::mount_path(const std::string& path)
    {
        if (!DirectoryExists(path.c_str()) && !FileExists(path.c_str())) {
            return false;
        }

        // Mounted at the root, so a script asks for "main.lua" rather than the path the game
        //      happens to sit at on this machine.
        if (PHYSFS_mount(path.c_str(), "/", 1) == 0) {
            this->error = "Couldn't mount " + path + ": " + physfs_error();
            return false;
        }

        this->description = path;
        return true;
    }

    bool GameSource::mount_bundled()
    {
        // PLATFORM CHECK: iOS and Android keep the game inside the app rather than in a folder the
        //      file system can reach (on Android it's an APK asset), and raylib's file reading is
        //      what knows how to get at it on both. The bytes are then mounted straight from
        //      memory, so the game never has to be unpacked to storage first.
        int size = 0;
        unsigned char* data = LoadFileData(BUNDLED_ARCHIVE, &size);
        if (data == nullptr) {
            return false;
        }

        if (PHYSFS_mountMemory(data, static_cast<PHYSFS_uint64>(size), free_file_data, BUNDLED_ARCHIVE, "/", 1) == 0) {
            this->error = std::string("Couldn't mount the bundled game: ") + physfs_error();
            UnloadFileData(data);
            return false;
        }

        this->description = std::string("bundled ") + BUNDLED_ARCHIVE;
        return true;
    }

    // --- GETTERS --- //
    bool GameSource::is_ready()
    {
        GameSource& self = GameSource::get_singleton();
        return self.ready;
    }

    std::string GameSource::get_description()
    {
        GameSource& self = GameSource::get_singleton();
        return self.description;
    }

    std::string GameSource::get_error()
    {
        GameSource& self = GameSource::get_singleton();
        return self.error;
    }

    // --- FILES --- //
    bool GameSource::exists(const std::string& path)
    {
        GameSource& self = GameSource::get_singleton();

        // RUNTIME GUARD: Without a mounted game there is no file system to look in.
        if (!self.ready) {
            return false;
        }

        return PHYSFS_exists(path.c_str()) != 0;
    }

    bool GameSource::read(const std::string& path, std::string& out_contents)
    {
        GameSource& self = GameSource::get_singleton();

        // RUNTIME GUARD: Without a mounted game there is nothing to read from.
        if (!self.ready) {
            return false;
        }

        PHYSFS_File* file = PHYSFS_openRead(path.c_str());
        if (file == nullptr) {
            return false;
        }

        const PHYSFS_sint64 length = PHYSFS_fileLength(file);
        if (length < 0) {
            PHYSFS_close(file);
            return false;
        }

        out_contents.resize(static_cast<std::size_t>(length));

        // An empty file has nothing to read, and PhysFS takes a read of zero bytes as a failure.
        if (length > 0) {
            const PHYSFS_sint64 read = PHYSFS_readBytes(file, out_contents.data(), static_cast<PHYSFS_uint64>(length));
            if (read != length) {
                PHYSFS_close(file);
                return false;
            }
        }

        PHYSFS_close(file);
        return true;
    }

    // --- LIFE CYCLE --- //
    void GameSource::compose()
    {
        GameSource& self = GameSource::get_singleton();

        // RUNTIME GUARD: This function should only be able to run when no game is mounted yet.
        if (self.ready) {
            return;
        }

        if (PHYSFS_init(nullptr) == 0) {
            self.error = std::string("Couldn't start the file system: ") + physfs_error();
            return;
        }

        // A game is looked for in the order that puts development first: the path the runtime was
        //      given, then a game shipped beside the executable, then one bundled inside the app.
        //      Only the first one found is mounted, so a fused build can still be pointed at
        //      another game while it's being worked on.
        for (const std::string& argument : Rivet::System::get_args()) {
            // Reserved for the flags the runtime will take later (eg. --console).
            if (!argument.empty() && argument.front() == '-') {
                continue;
            }

            if (self.mount_path(argument)) {
                self.ready = true;
                return;
            }

            // A path that was given but can't be used is a mistake worth reporting, rather than
            //      something to quietly fall back from.
            if (self.error.empty()) {
                self.error = "There is no game at " + argument + ".";
            }
            return;
        }

        const std::string application_directory = GetApplicationDirectory();
        if (self.mount_path(application_directory + BUNDLED_ARCHIVE)
            || self.mount_path(application_directory + BUNDLED_FOLDER)
            || self.mount_bundled()) {
            self.ready = true;
            return;
        }

        if (self.error.empty()) {
            self.error =
                "No game to run. Give the runtime a game folder or .rivet archive, or ship one "
                "as game.rivet next to it.";
        }
    }

    void GameSource::dispose()
    {
        GameSource& self = GameSource::get_singleton();

        // RUNTIME GUARD: This function should only be able to run while a game is mounted.
        if (!self.ready) {
            return;
        }

        // Unmounts everything, and frees whatever a game mounted from memory.
        PHYSFS_deinit();

        self.ready = false;
    }
}
