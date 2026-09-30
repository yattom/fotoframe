// 試作：2枚の JPEG をフルスクリーンで表示し、クロスフェードで交互に切り替える。
// 使い方: prototype <jpeg1> <jpeg2> [実行秒数]
// 1枚目は fit、2枚目は fill で表示する。

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <libexif/exif-data.h>
#include <turbojpeg.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

double elapsed_ms(Clock::time_point since) {
    return std::chrono::duration<double, std::milli>(Clock::now() - since).count();
}

enum class Mode { Fit, Fill };

// デコード結果。回転はせず、描画時に角度と反転で表す。
struct Picture {
    SDL_Texture* texture = nullptr;
    int width = 0;   // テクスチャの幅（回転前）
    int height = 0;  // テクスチャの高さ（回転前）
    double angle = 0.0;
    SDL_FlipMode flip = SDL_FLIP_NONE;
    Mode mode = Mode::Fit;
};

int read_orientation(const std::vector<unsigned char>& data) {
    ExifData* exif = exif_data_new_from_data(data.data(), data.size());
    if (!exif) return 1;
    int orientation = 1;
    if (ExifEntry* entry = exif_data_get_entry(exif, EXIF_TAG_ORIENTATION)) {
        orientation = exif_get_short(entry->data, exif_data_get_byte_order(exif));
    }
    exif_data_unref(exif);
    return orientation;
}

// EXIF Orientation を SDL の反転＋時計回りの回転に変換する。SDL は反転してから回転する。
void apply_orientation(int orientation, Picture& pic) {
    switch (orientation) {
        case 2: pic.flip = SDL_FLIP_HORIZONTAL; break;
        case 3: pic.angle = 180; break;
        case 4: pic.flip = SDL_FLIP_VERTICAL; break;
        case 5: pic.flip = SDL_FLIP_VERTICAL; pic.angle = 90; break;
        case 6: pic.angle = 90; break;
        case 7: pic.flip = SDL_FLIP_HORIZONTAL; pic.angle = 90; break;
        case 8: pic.angle = 270; break;
        default: break;
    }
}

bool is_quarter_turn(const Picture& pic) {
    return pic.angle == 90 || pic.angle == 270;
}

// 表示に足りる範囲で、最も小さくなる縮小率を選ぶ。
tjscalingfactor choose_scale(int w, int h, bool quarter_turn, Mode mode, int screen_w, int screen_h) {
    int count = 0;
    tjscalingfactor* factors = tjGetScalingFactors(&count);
    tjscalingfactor best = {1, 1};
    long best_area = static_cast<long>(w) * h;
    for (int i = 0; i < count; ++i) {
        const tjscalingfactor f = factors[i];
        int sw = TJSCALED(w, f);
        int sh = TJSCALED(h, f);
        if (quarter_turn) std::swap(sw, sh);
        const bool enough = (mode == Mode::Fit) ? (sw >= screen_w || sh >= screen_h)
                                                : (sw >= screen_w && sh >= screen_h);
        const long area = static_cast<long>(sw) * sh;
        if (enough && area < best_area) {
            best = f;
            best_area = area;
        }
    }
    return best;
}

bool load_picture(SDL_Renderer* renderer, const char* path, Mode mode, int screen_w, int screen_h, Picture& pic) {
    const auto start = Clock::now();

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "cannot open %s\n", path);
        return false;
    }
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const double read_ms = elapsed_ms(start);

    pic.mode = mode;
    const int orientation = read_orientation(data);
    apply_orientation(orientation, pic);

    tjhandle tj = tjInitDecompress();
    int w = 0, h = 0, subsamp = 0, colorspace = 0;
    if (tjDecompressHeader3(tj, data.data(), data.size(), &w, &h, &subsamp, &colorspace) != 0) {
        std::fprintf(stderr, "header error %s: %s\n", path, tjGetErrorStr2(tj));
        tjDestroy(tj);
        return false;
    }
    const tjscalingfactor scale = choose_scale(w, h, is_quarter_turn(pic), mode, screen_w, screen_h);
    pic.width = TJSCALED(w, scale);
    pic.height = TJSCALED(h, scale);

    const auto decode_start = Clock::now();
    std::vector<unsigned char> pixels(static_cast<size_t>(pic.width) * pic.height * 4);
    const int ok = tjDecompress2(tj, data.data(), data.size(), pixels.data(), pic.width, 0, pic.height, TJPF_RGBX, 0);
    tjDestroy(tj);
    if (ok != 0) {
        std::fprintf(stderr, "decode error %s\n", path);
        return false;
    }
    const double decode_ms = elapsed_ms(decode_start);

    const auto upload_start = Clock::now();
    pic.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX32, SDL_TEXTUREACCESS_STATIC, pic.width, pic.height);
    if (!pic.texture) {
        std::fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        return false;
    }
    SDL_UpdateTexture(pic.texture, nullptr, pixels.data(), pic.width * 4);
    SDL_SetTextureBlendMode(pic.texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(pic.texture, SDL_SCALEMODE_LINEAR);
    const double upload_ms = elapsed_ms(upload_start);

    std::printf("%s: %dx%d orient=%d scale=%d/%d -> %dx%d read=%.0fms decode=%.0fms upload=%.0fms\n", path, w, h,
                orientation, scale.num, scale.denom, pic.width, pic.height, read_ms, decode_ms, upload_ms);
    return true;
}

void draw_picture(SDL_Renderer* renderer, const Picture& pic, int screen_w, int screen_h, float alpha) {
    const bool quarter = is_quarter_turn(pic);
    const float shown_w = quarter ? pic.height : pic.width;
    const float shown_h = quarter ? pic.width : pic.height;
    const float kx = screen_w / shown_w;
    const float ky = screen_h / shown_h;
    const float k = (pic.mode == Mode::Fit) ? std::min(kx, ky) : std::max(kx, ky);

    // 回転は描画先の矩形の中心を軸に行われるので、回転前の大きさで中央に置く。
    const float dw = pic.width * k;
    const float dh = pic.height * k;
    const SDL_FRect dst = {(screen_w - dw) / 2, (screen_h - dh) / 2, dw, dh};
    SDL_SetTextureAlphaModFloat(pic.texture, alpha);
    SDL_RenderTextureRotated(renderer, pic.texture, nullptr, &dst, pic.angle, nullptr, pic.flip);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <jpeg1> <jpeg2> [seconds]\n", argv[0]);
        return 1;
    }
    // ファイルやパイプへ出力するときもログがすぐ出るように行単位でフラッシュする。
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    const double run_seconds = (argc >= 4) ? std::stod(argv[3]) : 30.0;
    constexpr double hold_seconds = 4.0;
    constexpr double fade_seconds = 1.5;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("fotoframe prototype", 1920, 1080, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);
    SDL_HideCursor();

    // フルスクリーンへの切り替えを反映させてから画面サイズを取る。
    SDL_SyncWindow(window);
    int screen_w = 0, screen_h = 0;
    SDL_GetRenderOutputSize(renderer, &screen_w, &screen_h);
    std::printf("video driver=%s renderer=%s output=%dx%d\n", SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer),
                screen_w, screen_h);

    Picture pics[2];
    if (!load_picture(renderer, argv[1], Mode::Fit, screen_w, screen_h, pics[0]) ||
        !load_picture(renderer, argv[2], Mode::Fill, screen_w, screen_h, pics[1])) {
        return 1;
    }

    const auto start = Clock::now();
    const double cycle = hold_seconds + fade_seconds;
    int fade_frames = 0;
    int last_cycle = 0;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) running = false;
        }
        const double t = elapsed_ms(start) / 1000.0;
        if (t >= run_seconds) break;

        const int n = static_cast<int>(t / cycle);
        const double in_cycle = t - n * cycle;
        const Picture& current = pics[n % 2];
        const Picture& next = pics[(n + 1) % 2];

        if (n != last_cycle) {
            std::printf("fade %d: %d frames in %.1fs = %.1f fps\n", last_cycle, fade_frames, fade_seconds,
                        fade_frames / fade_seconds);
            fade_frames = 0;
            last_cycle = n;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (in_cycle < hold_seconds) {
            draw_picture(renderer, current, screen_w, screen_h, 1.0f);
        } else {
            const float p = static_cast<float>((in_cycle - hold_seconds) / fade_seconds);
            draw_picture(renderer, current, screen_w, screen_h, 1.0f - p);
            draw_picture(renderer, next, screen_w, screen_h, p);
            ++fade_frames;
        }
        SDL_RenderPresent(renderer);
    }

    for (Picture& pic : pics) SDL_DestroyTexture(pic.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
