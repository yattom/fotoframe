#pragma once

#include <memory>
#include <string>

#include "geometry.h"
#include "jpeg_decoder.h"
#include "presentation.h"
#include "transition.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace fotoframe {

// GPU 上に置いた1枚の写真と、その見せ方。
class Slide {
public:
    Slide(SDL_Texture* texture, Size size, Orientation orientation, Presentation presentation);
    ~Slide();
    Slide(const Slide&) = delete;
    Slide& operator=(const Slide&) = delete;

    SDL_Texture* texture() const { return texture_; }
    Size size() const { return size_; }
    Orientation orientation() const { return orientation_; }
    const Presentation& presentation() const { return presentation_; }

private:
    SDL_Texture* texture_;
    Size size_;
    Orientation orientation_;
    Presentation presentation_;
};

// SDL3 のフルスクリーンウィンドウへの描画。
class Renderer {
public:
    // 失敗したら nullptr を返し、理由を標準エラー出力に書く。
    static std::unique_ptr<Renderer> create();
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Size screen() const { return screen_; }

    // デコード済みの画像を GPU に転送する。失敗したら nullptr。
    std::unique_ptr<Slide> make_slide(const DecodedImage& image, const Presentation& presentation);

    // 画面を黒で消す。以下の draw_* を呼んでから present で表示する。
    void clear();
    void draw_slide(const Slide& slide, const LayerState& layer);
    void draw_message(const std::string& message);
    void present();

    // 終了の要求（ウィンドウを閉じる、キー入力）があれば true。
    bool quit_requested();

private:
    Renderer(SDL_Window* window, SDL_Renderer* renderer, Size screen);

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    Size screen_;
};

}  // namespace fotoframe
