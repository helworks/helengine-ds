#include "platform/ds/NintendoDsSpriteAffine.hpp"
#include <cmath>
#include <stdexcept>
#include <cstdio>

/// Checks inverse hardware coefficients and tile positions against known affine maps.
int main() {
    using helengine::ds::NintendoDsSpriteAffine;
    const auto affine = NintendoDsSpriteAffine::Create(80, 10, 0, 30, 100, 70, 80, 40);
    if (affine.Hdx != -236 || affine.Hdy != 39 || affine.Vdx != 79 || affine.Vdy != 158) {
        throw std::runtime_error("DS inverse 8.8 matrix lost signed axes or shear");
    }
    double x, y;
    affine.ResolveTilePosition(0, 0, 32, 16, true, 5, 7, x, y);
    if (std::abs(x - 41) > 1e-8 || std::abs(y - 17) > 1e-8) {
        throw std::runtime_error("DS doubled OBJ anchor must use unscaled source bounds around transformed tile center");
    }
    const auto identity = NintendoDsSpriteAffine::Create(10, 20, 90, 20, 10, 60, 80, 40);
    if (!identity.IsIdentity()) { throw std::runtime_error("DS identity must retain plain OBJ path"); }
    identity.ResolveTilePosition(32, 16, 16, 8, false, 0, 0, x, y);
    if (x != 42 || y != 36) { throw std::runtime_error("DS tile source origins changed"); }
    bool rejected = false;
    try { NintendoDsSpriteAffine::Create(0, 0, 0, 0, 0, 10, 8, 8); }
    catch (const std::invalid_argument&) { rejected = true; }
    if (!rejected) { throw std::runtime_error("DS singular matrix must reject explicitly"); }
    rejected = false;
    try { identity.ValidateTileSize(8, 8, false); }
    catch (...) { throw std::runtime_error("DS identity tile should fit its ordinary box"); }
    const auto quarterTurn = NintendoDsSpriteAffine::Create(100, 20, 100, 52, 68, 20, 32, 32);
    if (quarterTurn.Hdx != 0 || quarterTurn.Hdy != -256 || quarterTurn.Vdx != 256 || quarterTurn.Vdy != 0) {
        throw std::runtime_error("DS quarter turn inverse matrix has the wrong sign");
    }
    quarterTurn.ValidateTileSize(32, 32, true);
    quarterTurn.ResolveTilePosition(0, 0, 32, 32, true, 0, 0, x, y);
    if (x != 52 || y != 4) { throw std::runtime_error("DS quarter turn changed its world pivot"); }
    try { NintendoDsSpriteAffine::Create(0, 0, 80, 0, 0, 1, 8, 8).ValidateTileSize(8, 8, true); }
    catch (const std::invalid_argument&) { rejected = true; }
    if (!rejected) { throw std::runtime_error("DS must report extents outside its doubled native OBJ box"); }
    std::puts("PASS DS signed/sheared inverse matrix, tile anchors, identity and singular rejection");
}
