#include "presentation.h"

namespace fotoframe {

PresentationChooser::PresentationChooser(ModeSetting mode, std::vector<Effect> effects, unsigned seed)
    : mode_(mode), effects_(std::move(effects)), rng_(seed) {}

Presentation PresentationChooser::next() {
    DisplayMode mode = DisplayMode::Fit;
    switch (mode_) {
        case ModeSetting::Fit: mode = DisplayMode::Fit; break;
        case ModeSetting::Fill: mode = DisplayMode::Fill; break;
        case ModeSetting::Random: mode = std::bernoulli_distribution(0.5)(rng_) ? DisplayMode::Fit : DisplayMode::Fill; break;
    }

    Effect effect = Effect::None;
    if (!effects_.empty()) {
        std::uniform_int_distribution<size_t> index(0, effects_.size() - 1);
        effect = effects_[index(rng_)];
    }
    return {mode, effect};
}

}  // namespace fotoframe
