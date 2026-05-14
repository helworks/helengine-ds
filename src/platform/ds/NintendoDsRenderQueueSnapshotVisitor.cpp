#include "platform/ds/NintendoDsRenderQueueSnapshotVisitor.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Creates one empty snapshot visitor.
    NintendoDsRenderQueueSnapshotVisitor::NintendoDsRenderQueueSnapshotVisitor()
        : Drawables(new List<IDrawable3D*>()) {
    }

    /// Clears the current snapshot so the visitor can be reused for the next frame.
    void NintendoDsRenderQueueSnapshotVisitor::Clear() {
        Drawables->Clear();
    }

    /// Gets the ordered drawable snapshot captured during the most recent queue visit.
    List<IDrawable3D*>* NintendoDsRenderQueueSnapshotVisitor::get_Drawables() {
        return Drawables;
    }

    /// Captures one visited drawable in queue order.
    /// <param name="drawable">Drawable visited by the render queue.</param>
    void NintendoDsRenderQueueSnapshotVisitor::Visit(IDrawable3D* drawable) {
        Drawables->Add(drawable);
    }
}
#endif
