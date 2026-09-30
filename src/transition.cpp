#include "transition.h"

namespace fotoframe {

TransitionFrame transition_frame(Effect effect, float progress, Size screen) {
    const float p = progress;
    switch (effect) {
        case Effect::Fade:
            return {{1 - p, 0, 0}, {p, 0, 0}};
        case Effect::SlideHorizontal:
            return {{1, -p * screen.width, 0}, {1, (1 - p) * screen.width, 0}};
        case Effect::SlideVertical:
            return {{1, 0, -p * screen.height}, {1, 0, (1 - p) * screen.height}};
        case Effect::None:
            break;
    }
    return {{0, 0, 0}, {1, 0, 0}};
}

}  // namespace fotoframe
