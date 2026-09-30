#include "renderer.h"

#include <SDL3/SDL.h>

#include <cstdio>

#include "layout.h"

namespace fotoframe {

namespace {

SDL_FlipMode to_sdl_flip(Flip flip) {
    switch (flip) {
        case Flip::Horizontal: return SDL_FLIP_HORIZONTAL;
        case Flip::Vertical: return SDL_FLIP_VERTICAL;
        case Flip::None: break;
    }
    return SDL_FLIP_NONE;
}

}  // namespace

Slide::Slide(SDL_Texture* texture, Size size, Orientation orientation, Presentation presentation)
    : texture_(texture), size_(size), orientation_(orientation), presentation_(presentation) {}

Slide::~Slide() { SDL_DestroyTexture(texture_); }

std::unique_ptr<Renderer> Renderer::create() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return nullptr;
    }
    SDL_Window* window = SDL_CreateWindow("fotoframe", 1920, 1080, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return nullptr;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return nullptr;
    }
    SDL_SetRenderVSync(renderer, 1);
    SDL_HideCursor();

    // フルスクリーンへの切り替えを反映させてから画面サイズを取る。
    SDL_SyncWindow(window);
    int width = 0, height = 0;
    SDL_GetRenderOutputSize(renderer, &width, &height);
    std::printf("video driver=%s renderer=%s screen=%dx%d\n", SDL_GetCurrentVideoDriver(),
                SDL_GetRendererName(renderer), width, height);
    return std::unique_ptr<Renderer>(new Renderer(window, renderer, {width, height}));
}

Renderer::Renderer(SDL_Window* window, SDL_Renderer* renderer, Size screen)
    : window_(window), renderer_(renderer), screen_(screen) {}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

std::unique_ptr<Slide> Renderer::make_slide(const DecodedImage& image, const Presentation& presentation) {
    SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBX32, SDL_TEXTUREACCESS_STATIC,
                                             image.size.width, image.size.height);
    if (!texture) {
        std::fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        return nullptr;
    }
    SDL_UpdateTexture(texture, nullptr, image.pixels.data(), image.size.width * 4);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    return std::make_unique<Slide>(texture, image.size, image.orientation, presentation);
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
}

void Renderer::draw_slide(const Slide& slide, const LayerState& layer) {
    const Rect rect = place(slide.size(), slide.orientation(), slide.presentation().mode, screen_);
    const SDL_FRect dst{rect.x + layer.dx, rect.y + layer.dy, rect.width, rect.height};
    SDL_SetTextureAlphaModFloat(slide.texture(), layer.alpha);
    SDL_RenderTextureRotated(renderer_, slide.texture(), nullptr, &dst, slide.orientation().angle, nullptr,
                             to_sdl_flip(slide.orientation().flip));
}

void Renderer::draw_message(const std::string& message) {
    // SDL 組み込みの 8x8 ドットのフォントを2倍にして、左下に灰色で表示する。
    constexpr float kScale = 2;
    SDL_SetRenderScale(renderer_, kScale, kScale);
    SDL_SetRenderDrawColor(renderer_, 128, 128, 128, 255);
    const float y = screen_.height / kScale - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 2;
    SDL_RenderDebugText(renderer_, SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE, y, message.c_str());
    SDL_SetRenderScale(renderer_, 1, 1);
}

void Renderer::present() { SDL_RenderPresent(renderer_); }

bool Renderer::quit_requested() {
    bool quit = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) quit = true;
    }
    return quit;
}

}  // namespace fotoframe
