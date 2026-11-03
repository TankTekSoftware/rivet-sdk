#include <Rivet/core/Viewport.hpp>

#include <Rivet/core/Display.hpp>

#include <Rivet/render/Renderer.hpp>

#include <raylib.h>
#include <rlgl.h>

#include <cmath>
#include <algorithm>

namespace
{
    int texture_filter(bool pixel_filter)
    {
        return pixel_filter ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR;
    }
}

namespace Rivet
{
    struct Viewport::Impl
    {
        RenderTexture2D target = { };
        Rectangle dest = { };
        Vector2 canvas_scale = { 1.0f, 1.0f };
        int target_width = 0;
        int target_height = 0;
    };

    Viewport& Viewport::get_singleton()
    {
        static Viewport singleton;
        return singleton;
    }

    Viewport::Viewport()
    {
        unsigned int width = 0;
        unsigned int height = 0;

        // RUNTIME GUARD: if the display is ready, then use the display size as the
        //      default viewport size.
        if (Display::is_ready()) {
            width = Display::get_width();
            height = Display::get_height();
        }

        // Assign the default viewport properties.
        this->base_width = width;
        this->base_height = height;
        this->stretch_mode = StretchMode::VIEWPORT;
        this->stretch_aspect = StretchAspect::KEEP;
        this->stretch_scale = 1.0f;
        this->pixel_filter = false;
        this->integer_scaling = false;

        // Until the viewport is laid out against the window, the game is laid out at the base size.
        this->width = width;
        this->height = height;

        // Allocate the backend state.
        this->impl = std::make_unique<Impl>();

        this->ready = false;
    }

    Viewport::~Viewport()
    {
        this->dispose();
    }

    // --- UTILITY --- //
    bool Viewport::layout()
    {
        // The window is drawn to in screen units, but is made of render pixels. These are the same
        //      on most desktops, but iOS has 2-3 render pixels to every screen unit (point). The
        //      layout is worked out in render pixels, since that's what the target is made of.
        const float screen_width = static_cast<float>(Display::get_width());
        const float screen_height = static_cast<float>(Display::get_height());
        const float render_width = static_cast<float>(GetRenderWidth());
        const float render_height = static_cast<float>(GetRenderHeight());

        // RUNTIME GUARD: The window can briefly have no size (eg. while it's minimized), and there
        //      is nothing to lay out until it has one again.
        if (screen_width <= 0.0f || screen_height <= 0.0f || render_width <= 0.0f || render_height <= 0.0f) {
            return false;
        }

        // Without a base size there is nothing to stretch, so the viewport behaves as if
        //      stretching is disabled.
        const bool stretched = this->stretch_mode != StretchMode::DISABLED && this->base_width > 0 && this->base_height > 0;

        float content_width = 0.0f;     // The size of the game, in viewport units.
        float content_height = 0.0f;
        Rectangle area = { };           // Where the game covers the window, in render pixels.

        if (!stretched) {
            // The game covers the whole window, and one viewport unit is one screen unit.
            content_width = std::floor(screen_width / this->stretch_scale);
            content_height = std::floor(screen_height / this->stretch_scale);
            area = { 0.0f, 0.0f, render_width, render_height };
        }
        else {
            const float base_width = static_cast<float>(this->base_width);
            const float base_height = static_cast<float>(this->base_height);
            const float base_aspect = base_width / base_height;
            const float render_aspect = render_width / render_height;

            // `size` is the base size grown to the window's shape where the aspect allows it, and
            //      `fit` is how much of the window it covers. Whatever `fit` leaves uncovered
            //      becomes bars.
            float size_width = base_width;
            float size_height = base_height;
            float fit_width = render_width;
            float fit_height = render_height;

            const bool same_aspect = std::fabs(base_aspect - render_aspect) < 0.0001f;
            if (this->stretch_aspect == StretchAspect::IGNORE || same_aspect) {
                // The base size is stretched over the whole window.
            }
            else if (base_aspect < render_aspect) {
                // The window is wider than the base size.
                if (this->stretch_aspect == StretchAspect::KEEP_HEIGHT || this->stretch_aspect == StretchAspect::EXPAND) {
                    size_width = base_height * render_aspect;
                }
                else {
                    fit_width = render_height * base_aspect;
                }
            }
            else {
                // The window is taller than the base size.
                if (this->stretch_aspect == StretchAspect::KEEP_WIDTH || this->stretch_aspect == StretchAspect::EXPAND) {
                    size_height = base_width / render_aspect;
                }
                else {
                    fit_height = render_width / base_aspect;
                }
            }

            content_width = std::max(1.0f, std::floor(size_width / this->stretch_scale));
            content_height = std::max(1.0f, std::floor(size_height / this->stretch_scale));
            fit_width = std::max(1.0f, std::floor(fit_width));
            fit_height = std::max(1.0f, std::floor(fit_height));

            // Integer scaling shrinks the covered area to the largest whole multiple of the game
            //      that fits, so every viewport unit covers the same number of render pixels.
            if (this->integer_scaling) {
                const float multiple = std::max(1.0f, std::floor(std::min(fit_width / content_width, fit_height / content_height)));
                fit_width = content_width * multiple;
                fit_height = content_height * multiple;
            }

            area = {
                std::round((render_width - fit_width) * 0.5f),      // x
                std::round((render_height - fit_height) * 0.5f),    // y
                fit_width,                                          // width
                fit_height                                          // height
            };
        }

        content_width = std::max(1.0f, content_width);
        content_height = std::max(1.0f, content_height);

        // VIEWPORT draws the game at its own size, then scales the finished picture to the area it
        //      covers. CANVAS_ITEMS and DISABLED draw the game straight at the area's resolution.
        const bool scale_picture = stretched && this->stretch_mode == StretchMode::VIEWPORT;
        const float target_width = scale_picture ? content_width : area.width;
        const float target_height = scale_picture ? content_height : area.height;

        this->width = static_cast<unsigned int>(content_width);
        this->height = static_cast<unsigned int>(content_height);

        this->impl->target_width = static_cast<int>(target_width);
        this->impl->target_height = static_cast<int>(target_height);
        this->impl->canvas_scale = { target_width / content_width, target_height / content_height };

        // The window is drawn to in screen units, so the area is converted back to them.
        const float screen_per_render_x = screen_width / render_width;
        const float screen_per_render_y = screen_height / render_height;
        this->impl->dest = {
            area.x * screen_per_render_x,           // x
            area.y * screen_per_render_y,           // y
            area.width * screen_per_render_x,       // width
            area.height * screen_per_render_y       // height
        };

        return true;
    }

    void Viewport::load_target(int width, int height)
    {
        // A render texture can't be resized, so an existing target is replaced.
        if (this->impl->target.id != 0) {
            UnloadRenderTexture(this->impl->target);
        }

        this->impl->target = LoadRenderTexture(width, height);
        SetTextureFilter(this->impl->target.texture, texture_filter(this->pixel_filter));

        // Clamp the edges, so smoothing the picture as it's scaled doesn't blend in pixels that
        //      wrapped around from the opposite edge.
        SetTextureWrap(this->impl->target.texture, TEXTURE_WRAP_CLAMP);
    }

    // --- GETTERS --- //
    unsigned int Viewport::get_width()
    {
        Viewport& self = Viewport::get_singleton();
        return self.width;
    }

    unsigned int Viewport::get_height()
    {
        Viewport& self = Viewport::get_singleton();
        return self.height;
    }

    unsigned int Viewport::get_base_width()
    {
        Viewport& self = Viewport::get_singleton();
        return self.base_width;
    }

    unsigned int Viewport::get_base_height()
    {
        Viewport& self = Viewport::get_singleton();
        return self.base_height;
    }

    StretchMode Viewport::get_stretch_mode()
    {
        Viewport& self = Viewport::get_singleton();
        return self.stretch_mode;
    }

    StretchAspect Viewport::get_stretch_aspect()
    {
        Viewport& self = Viewport::get_singleton();
        return self.stretch_aspect;
    }

    float Viewport::get_stretch_scale()
    {
        Viewport& self = Viewport::get_singleton();
        return self.stretch_scale;
    }

    bool Viewport::is_pixel_filter()
    {
        Viewport& self = Viewport::get_singleton();
        return self.pixel_filter;
    }

    bool Viewport::is_integer_scaling()
    {
        Viewport& self = Viewport::get_singleton();
        return self.integer_scaling;
    }

    // --- SETTERS --- //
    // NOTE: The layout is worked out again on every `update()`, so changes to the size and
    //      stretch settings take effect on the next frame.
    void Viewport::set_size(unsigned int width, unsigned int height)
    {
        Viewport& self = Viewport::get_singleton();
        self.base_width = width;
        self.base_height = height;

        // Until the viewport is laid out against the window, the game is laid out at the base size.
        if (!self.ready) {
            self.width = width;
            self.height = height;
        }
    }

    void Viewport::set_stretch_mode(StretchMode stretch_mode)
    {
        Viewport& self = Viewport::get_singleton();
        self.stretch_mode = stretch_mode;
    }

    void Viewport::set_stretch_aspect(StretchAspect stretch_aspect)
    {
        Viewport& self = Viewport::get_singleton();
        self.stretch_aspect = stretch_aspect;
    }

    void Viewport::set_stretch_scale(float stretch_scale)
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: The game's size is divided by the scale, so a scale of zero or less
        //      is ignored.
        if (stretch_scale <= 0.0f) {
            return;
        }

        self.stretch_scale = stretch_scale;
    }

    void Viewport::set_pixel_filter(bool pixel_filter)
    {
        Viewport& self = Viewport::get_singleton();
        self.pixel_filter = pixel_filter;

        if (self.ready) {
            SetTextureFilter(self.impl->target.texture, texture_filter(self.pixel_filter));
        }
    }

    void Viewport::set_integer_scaling(bool integer_scaling)
    {
        Viewport& self = Viewport::get_singleton();
        self.integer_scaling = integer_scaling;
    }

    // --- CONVERSION --- //
    float Viewport::screen_to_viewport_x(float x)
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport isn't drawn anywhere, there is nothing to map onto.
        if (!self.ready || self.impl->dest.width <= 0.0f) {
            return x;
        }

        return (x - self.impl->dest.x) / self.impl->dest.width * static_cast<float>(self.width);
    }

    float Viewport::screen_to_viewport_y(float y)
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport isn't drawn anywhere, there is nothing to map onto.
        if (!self.ready || self.impl->dest.height <= 0.0f) {
            return y;
        }

        return (y - self.impl->dest.y) / self.impl->dest.height * static_cast<float>(self.height);
    }

    // --- LIFE CYCLE --- //
    void Viewport::compose()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: This function should not run if the display has not been
        //      created yet.
        if (!Display::is_ready()) {
            return;
        }

        // RUNTIME GUARD: This function should not run if the viewport has already been
        //      setup.
        if (self.ready) {
            return;
        }

        // RUNTIME GUARD: The target's size comes from the layout, so the viewport can't be setup
        //      until the window has a size to lay it out against.
        if (!self.layout()) {
            return;
        }

        self.load_target(self.impl->target_width, self.impl->target_height);

        self.ready = true;
    }

    void Viewport::update()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        // RUNTIME GUARD: While the window has no size, keep the last layout.
        if (!self.layout()) {
            return;
        }

        // The window's size or shape, or the stretch settings, may have changed since the last
        //      frame (eg. rotating an iPhone/iPad), so remake the target if it needs a new size.
        const Texture2D& texture = self.impl->target.texture;
        if (texture.width != self.impl->target_width || texture.height != self.impl->target_height) {
            self.load_target(self.impl->target_width, self.impl->target_height);
        }
    }

    void Viewport::dispose()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: This function should not run if the viewport has not been created,
        //      or is already disposed of.
        if (!self.ready) {
            return;
        }

        UnloadRenderTexture(self.impl->target);
        self.impl->target = { };

        self.ready = false;
    }

    void Viewport::begin()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        BeginTextureMode(self.impl->target);
        Renderer::clear();

        // Raylib's alpha blending blends the target's alpha the same way as its color, so anything
        //      translucent leaves the target partly see-through, and the window's clear color shows
        //      through it when the target is drawn. Alpha is built up instead, which keeps the
        //      target premultiplied.
        rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD, RL_FUNC_ADD);
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);

        // CANVAS_ITEMS and DISABLED draw the game at the target's resolution rather than the
        //      game's own size, so viewport units are scaled to the target's pixels.
        rlScalef(self.impl->canvas_scale.x, self.impl->canvas_scale.y, 1.0f);
    }

    void Viewport::end()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        EndBlendMode();
        EndTextureMode();
    }

    void Viewport::render()
    {
        Viewport& self = Viewport::get_singleton();

        // RUNTIME GUARD: If the viewport is not setup, don't run the rest of the function.
        if (!self.ready) {
            return;
        }

        const Texture2D& texture = self.impl->target.texture;
        const Rectangle source = {
            0.0f,                                   // x
            0.0f,                                   // y
            static_cast<float>(texture.width),      // width
            -static_cast<float>(texture.height)     // height
        };

        const Vector2 origin = { 0.0f, 0.0f };
        const float rotation = 0.0f;
        const ::Color tint = ::WHITE;

        // The target is premultiplied (see `begin()`), so it's blended as such.
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        DrawTexturePro(texture, source, self.impl->dest, origin, rotation, tint);
        EndBlendMode();
    }
}
