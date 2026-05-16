#pragma once

namespace helengine::ds {
    /// <summary>
    /// Identifies the Nintendo DS physical screen that currently owns one runtime presentation pass.
    /// </summary>
    enum class NintendoDsScreenTarget {
        /// <summary>
        /// No physical screen currently owns the queried pass.
        /// </summary>
        None = 0,

        /// <summary>
        /// The top physical Nintendo DS screen owns the queried pass.
        /// </summary>
        Top = 1,

        /// <summary>
        /// The bottom physical Nintendo DS screen owns the queried pass.
        /// </summary>
        Bottom = 2
    };
}
