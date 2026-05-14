#include "platform/ds/NintendoDsRuntimeModel.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
namespace helengine::ds {
    /// Creates one empty DS runtime model.
    NintendoDsRuntimeModel::NintendoDsRuntimeModel()
        : Positions(nullptr)
        , Indices16(nullptr)
        , Indices32(nullptr)
        , Uses32BitIndices(false) {
    }
}
#endif
