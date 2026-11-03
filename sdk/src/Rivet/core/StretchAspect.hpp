#pragma once

namespace Rivet
{
    // What the viewport does when the window's shape doesn't match the viewport size. Only
    //      used when the stretch mode isn't `StretchMode::DISABLED`.
    enum class StretchAspect
    {
        // Stretch the viewport to fill the window, even if that distorts it.
        IGNORE,
        // Keep the viewport size, and fill the rest of the window with bars.
        KEEP,
        // Keep the viewport width. A taller window makes the viewport taller, and a wider
        //      window gets bars on the sides.
        KEEP_WIDTH,
        // Keep the viewport height. A wider window makes the viewport wider, and a taller
        //      window gets bars on the top and bottom.
        KEEP_HEIGHT,
        // Grow the viewport in whichever direction fills the window, so there are never any
        //      bars. The viewport size is the smallest area the game will get.
        EXPAND
    };
}
