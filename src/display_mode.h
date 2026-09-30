#pragma once

namespace fotoframe {

// 画面と縦横比が違う画像の表示方法。
enum class DisplayMode {
    Fit,   // 画像全体を画面に収め、余白は黒
    Fill,  // 画面全体を画像で覆い、はみ出た部分は切り捨てる
};

}  // namespace fotoframe
