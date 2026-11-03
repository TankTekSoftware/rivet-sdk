#pragma once

namespace Rivet
{
    // How the viewport is scaled to the window.
    enum class StretchMode
    {
        // The game isn't scaled: one viewport unit is one screen unit, and the viewport is
        //      always the size of the window (divided by the stretch scale).
        DISABLED,
        // The game is drawn straight at the window's resolution, so it stays sharp at any size.
        CANVAS_ITEMS,
        // The game is drawn at the viewport size, and the finished picture is scaled to the
        //      window. Suits pixel art.
        VIEWPORT
    };
}
