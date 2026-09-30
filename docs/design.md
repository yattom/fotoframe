# fotoframe 設計

仕様は [spec.md](spec.md)、技術選定は [architecture.md](architecture.md) を参照。

## 方針

- 純粋なロジックと、外部（SDL、turbojpeg、ファイルシステム、wlopm）とのやり取りを分ける
- 時刻・乱数は外から渡し、ロジックはすべてユニットテストで確かめる
- 外部とのやり取りは薄いラッパーにとどめ、動かして確認する
- 1つの役割を1つのファイル（クラスまたは関数群）にする
- テスト駆動開発（Kent Beck の Canon TDD）で進める

## ファイル構成

```
src/
  main.cpp               起動、各部品の組み立て、メインループ
  -- 共通の型 --
  geometry.h             Size、Rect
  display_mode.h         DisplayMode（Fit / Fill）
  -- ロジック（ユニットテスト対象） --
  config                 設定ファイル（TOML）の読み込み → Config 構造体
  photo_filter           表示対象のファイルか判定する（拡張子 .jpg/.jpeg、大小文字を区別しない）
  photo_scanner          指定フォルダ以下を再帰的に走査して写真の一覧を返す
  photo_deck             山札方式で次の写真を選ぶ。一覧の更新（追加・削除）を反映する
  photo_buffer           先読みバッファ（下記）
  presentation           表示モード（fit/fill）と切り替え効果を設定に従って選ぶ
  layout                 画像サイズ・EXIF の向き・表示モードから描画位置・角度・反転を計算する
  decode_scale           デコード時の縮小率を選ぶ
  transition             切り替え効果と進み具合（0〜1）から、2枚の透明度・位置を計算する
  sleep_schedule         消灯時間帯かどうか判定する（日付をまたぐ区間に対応）
  slideshow              表示の状態遷移（待機中、表示中、切り替え中、消灯中）。時刻を引数で受け取り、
                         「切り替えを始める」「消灯する」などのイベントを返す
  -- 外部とのやり取り（動かして確認） --
  jpeg_decoder           turbojpeg と libexif でデコードし、EXIF の向きを読む
  renderer               SDL3 のウィンドウ・テクスチャ・描画、エラー文字の表示
  display_power          wlopm でモニターをスタンバイにする／戻す
  loader                 バックグラウンドスレッド。定期的な再走査と、NAS からバッファへの読み込み
tests/
  <対応するファイル名>_test.cpp
config/
  fotoframe.example.toml 設定ファイルの例（実際の設定は ~/.config/fotoframe/ に置く）
```

## 処理の流れ

```
[loader スレッド]  PhotoScanner → PhotoDeck → NAS から JPEG のバイト列を読む → PhotoBuffer
[メインスレッド]    PhotoBuffer → JpegDecoder → Slideshow（状態と時刻）→ Transition / Layout → Renderer
                    SleepSchedule → DisplayPower
```

SDL の描画はメインスレッドで行う。スレッドはメインと loader の2つだけ。

## 先読みバッファ（PhotoBuffer）

これから表示する写真の JPEG バイト列を、容量上限付きで RAM に持つ。NAS に接続できない間のキャッシュも兼ねる。

入れる（loader スレッド）：
- 容量が足りないときは、表示済みの写真を、表示したのが古いものから捨てて空きを作る
- 未表示の写真は捨てない。空きを作れなければ入れずに待つ
- 容量上限より大きい写真は入れない

出す（メインスレッド）：
- 未表示の写真があれば、入れた順に出し、その写真を表示済みにする
- 未表示の写真がなければ、表示済みの写真の中から山札方式で選ぶ

バッファは NAS に接続できるかどうかを知らない。loader が補充できなくなると、自然に表示済みの写真を回す動きになる。再走査で NAS から消えた写真がバッファに残っていると一度は表示されることがあるが、許容する。

## シャッフル（PhotoDeck）

山札方式。全部を一巡するまで同じ写真を出さず、一巡したら並べ直す。再走査で見つけた新しい写真は残りの山札のランダムな位置に差し込み、消えた写真は山札から取り除く。

## 使用ライブラリ

- テスト：GoogleTest
- 設定ファイル：toml++
- エラー表示の文字：SDL3 組み込みの `SDL_RenderDebugText`
