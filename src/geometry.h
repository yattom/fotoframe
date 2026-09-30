#pragma once

namespace fotoframe {

struct Size {
    int width;
    int height;
};

struct Rect {
    float x;
    float y;
    float width;
    float height;
    bool operator==(const Rect&) const = default;
};

}  // namespace fotoframe
