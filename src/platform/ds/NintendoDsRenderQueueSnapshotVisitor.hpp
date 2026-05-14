#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "IDrawable3D.hpp"
#include "IRenderVisitor3D.hpp"
#include "runtime/native_list.hpp"

namespace helengine::ds {
    /// Captures one ordered snapshot of the current 3D render queue.
    class NintendoDsRenderQueueSnapshotVisitor : public IRenderVisitor3D {
    public:
        /// Creates one empty snapshot visitor.
        NintendoDsRenderQueueSnapshotVisitor();

        /// Clears the current snapshot so the visitor can be reused for the next frame.
        void Clear();

        /// Gets the ordered drawable snapshot captured during the most recent queue visit.
        List<IDrawable3D*>* get_Drawables();

        /// Captures one visited drawable in queue order.
        /// <param name="drawable">Drawable visited by the render queue.</param>
        void Visit(IDrawable3D* drawable) override;

    private:
        /// Stores the ordered drawable snapshot being accumulated for the current frame.
        List<IDrawable3D*>* Drawables;
    };
}
#endif
