# fotoframe

Raspberry Pi 4 を使ったデジタルフォトフレーム。NAS 上の写真をシャッフルしながらフルスクリーンでスライドショー表示する。

- 仕様：[docs/spec.md](docs/spec.md)
- アーキテクチャ：[docs/architecture.md](docs/architecture.md)

## ビルド

必要なパッケージ（バージョンは [docs/architecture.md](docs/architecture.md) を参照）:

```sh
sudo apt install -y cmake g++ libsdl3-dev libturbojpeg0-dev libexif-dev libtomlplusplus-dev libgtest-dev
```

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

## 実行

[config/fotoframe.example.toml](config/fotoframe.example.toml) を `~/.config/fotoframe/config.toml` にコピーして編集し、Wayland のセッション上で実行する。

```sh
./build/fotoframe [設定ファイル]
```
