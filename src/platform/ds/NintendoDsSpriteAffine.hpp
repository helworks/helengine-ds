#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace helengine::ds {
    /// Converts shared affine sprite geometry directly into DS OBJ inverse coefficients and tile anchors.
    class NintendoDsSpriteAffine {
    public:
        /// Horizontal source X step in signed 8.8 fixed point.
        int Hdx = 256;
        /// Horizontal source Y step in signed 8.8 fixed point.
        int Hdy = 0;
        /// Vertical source X step in signed 8.8 fixed point.
        int Vdx = 0;
        /// Vertical source Y step in signed 8.8 fixed point.
        int Vdy = 256;

        /// Inverts exact source-pixel basis vectors; hardware quantization happens only at submission.
        static NintendoDsSpriteAffine Create(double tlX, double tlY, double trX, double trY,
            double blX, double blY, double textureWidth, double textureHeight) {
            if (textureWidth <= 0 || textureHeight <= 0) { throw std::invalid_argument("DS sprite source size must be positive."); }
            NintendoDsSpriteAffine affine;
            affine.OriginX = tlX;
            affine.OriginY = tlY;
            affine.RightX = (trX - tlX) / textureWidth;
            affine.RightY = (trY - tlY) / textureWidth;
            affine.DownX = (blX - tlX) / textureHeight;
            affine.DownY = (blY - tlY) / textureHeight;
            const double determinant = affine.RightX * affine.DownY - affine.RightY * affine.DownX;
            if (!std::isfinite(determinant) || determinant == 0.0) { throw std::invalid_argument("DS sprite affine matrix must be finite and invertible."); }
            affine.Hdx = Quantize(affine.DownY / determinant);
            affine.Hdy = Quantize(-affine.RightY / determinant);
            affine.Vdx = Quantize(-affine.DownX / determinant);
            affine.Vdy = Quantize(affine.RightX / determinant);
            if (static_cast<int64_t>(affine.Hdx) * affine.Vdy - static_cast<int64_t>(affine.Hdy) * affine.Vdx == 0) {
                throw std::invalid_argument("DS sprite inverse matrix became singular at hardware precision.");
            }
            return affine;
        }

        /// Identifies the exact untransformed source-pixel mapping that needs no affine slot.
        bool IsIdentity() const { return RightX == 1 && RightY == 0 && DownX == 0 && DownY == 1; }

        /// Rejects geometry exceeding OBJ's doubled clipping box rather than truncating visible sprite pixels.
        void ValidateTileSize(double width, double height, bool doubled) const {
            const double boxScale = doubled ? 2.0 : 1.0;
            if (std::abs(RightX) * width + std::abs(DownX) * height > boxScale * width + 0.00001
                || std::abs(RightY) * width + std::abs(DownY) * height > boxScale * height + 0.00001) {
                throw std::invalid_argument("DS sprite affine extent exceeds the OBJ clipping box.");
            }
        }

        /// Places each unscaled OBJ clipping box around its source tile's transformed center.
        void ResolveTilePosition(double sourceX, double sourceY, double width, double height, bool doubled,
            double viewportX, double viewportY, double& x, double& y) const {
            const double centerX = sourceX + width * 0.5;
            const double centerY = sourceY + height * 0.5;
            const double anchor = doubled ? 1.0 : 0.5;
            x = OriginX + RightX * centerX + DownX * centerY + viewportX - width * anchor;
            y = OriginY + RightY * centerX + DownY * centerY + viewportY - height * anchor;
        }

    private:
        /// Shared world-space top-left pixel anchor.
        double OriginX = 0, OriginY = 0;
        /// Exact world-space basis per source texel for positioning source tile centers.
        double RightX = 1, RightY = 0, DownX = 0, DownY = 1;

        /// Converts one coefficient to OBJ's signed 16-bit register without wraparound or clamping.
        static int Quantize(double value) {
            const double fixed = std::round(value * 256.0);
            if (!std::isfinite(fixed) || fixed < -32768 || fixed > 32767) {
                throw std::invalid_argument("DS sprite inverse affine coefficient exceeds signed 8.8 range.");
            }
            return static_cast<int>(fixed);
        }
    };
}
