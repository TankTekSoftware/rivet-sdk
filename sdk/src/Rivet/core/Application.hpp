#pragma once

#include <Rivet/render/RenderPass.hpp>

namespace Rivet
{
    class Application
    {
        public:
            void create();
            void tick(bool view_size_changed);
            void destroy();
        protected:
            virtual void on_compose() {}
            virtual void on_ready() {}
            virtual void on_update(float delta) {}
            virtual void on_render(RenderPass pass) {}
            virtual void on_dispose() {}
        private:
            void update();
            void render();
        public:
            void run();
    };

    // DO NOT IMPLEMENT: This function should only be implemented in a Rivet project.
    // This function should remain unimplemented in the RivetSDK.
    Application* create_application();
}