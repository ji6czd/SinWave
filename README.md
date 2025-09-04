# Sinwave - サイン波生成コマンド

## このコマンドの機能

Sinwaveは、指定した周波数のサイン波を生成するコマンドラインツールです。以下の2つの主要な機能を提供します：

1. **C言語配列の生成**: 1周期分のサイン波データをC言語の配列定義として出力
2. **WAVファイルの生成**: 指定した長さのサイン波をWAVオーディオファイルとして出力

このツールは、組み込み開発でのサイン波テーブル生成、音響テスト、楽器の調律、音声信号処理の学習などに活用できます。

## 起動パラメータの説明

### コマンド形式

```bash
# C言語配列の生成
sinwave array -f<frequency> [-a<amplitude>] <output.c>

# WAVファイルの生成
sinwave wave -f<frequency> -d<duration> [-a<amplitude>] [-s<sample_rate>] <output.wav>
```

### コマンド

- `array` : 1周期分のC言語配列を生成します
- `wave`  : WAVオーディオファイルを生成します

### 必須オプション

- `-f<frequency>` : 周波数（Hz）
  - 例: `-f440`（A4音）、`-f1000`（1kHz）

### arrayコマンド専用オプション

特になし（周波数のみ必須）

### waveコマンド専用オプション

- `-d<duration>` : 継続時間（秒）
  - 例: `-d0.5`（0.5秒）、`-d2.0`（2秒）

### 共通オプション

- `-a<amplitude>` : 振幅（0.0～1.0）
  - デフォルト: 0.8
  - 例: `-a0.5`（半分の音量）、`-a1.0`（最大音量）

- `-s<sample_rate>` : サンプリング周波数（Hz）
  - デフォルト: 44100
  - 例: `-s48000`（48kHz）、`-s96000`（96kHz）

## サンプルコマンド

### C言語配列生成の例

```bash
# 440Hzの1周期分をC配列として生成
sinwave array -f440 tuning_A4.c

# 1kHzの1周期分を振幅0.5でC配列として生成
sinwave array -f1000 -a0.5 test_1khz.c

# 高周波（10kHz）のC配列を生成
sinwave array -f10000 high_freq.c
```

### WAVファイル生成の例

```bash
# 440Hzを1秒間、CD品質で生成
sinwave wave -f440 -d1.0 tuning_A4.wav

# 1kHzを0.5秒間、48kHzサンプリングレートで生成
sinwave wave -f1000 -d0.5 -s48000 test_1khz.wav

# 低音（100Hz）を2秒間、振幅0.3で生成
sinwave wave -f100 -d2.0 -a0.3 low_freq.wav

# 高品質（96kHz）で音楽用A音を生成
sinwave wave -f440 -d3.0 -s96000 -a0.7 reference_A4.wav
```

### 組み込み開発での活用例

```bash
# マイコン用のサイン波テーブル生成（小さなメモリ使用量）
sinwave array -f1000 -s16000 sine_table.c

# PWM用の低周波サイン波テーブル
sinwave array -f50 -s8000 -a1.0 pwm_sine.c

# オーディオ処理用の基準信号生成
sinwave array -f440 audio_reference.c
```

## 出力ファイルについて

### C配列ファイルの特徴

生成される`.c`ファイルには以下が含まれます：

- `int16_t`型の配列定義
- 配列サイズの定数
- 周波数とサンプリングレートの情報
- コンパイル可能なC言語ソースコード

### WAVファイルの特徴

- 16bit PCM形式
- モノラル音声
- 指定したサンプリング周波数
- 標準的な音声プレイヤーで再生可能

## 使用用途

- **組み込み開発**: マイコンでのサイン波テーブル生成
- **音響テスト**: スピーカーやオーディオ機器の動作確認
- **楽器調律**: 基準音の生成
- **信号処理学習**: デジタル信号処理の教材作成
- **周波数測定**: 測定器の校正用信号生成
- **PWM制御**: モーター制御用の滑らかな制御信号生成

## ビルド方法

```bash
# デバッグビルド
make

# またはCMakeを使用
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 追記

このプログラムは、README.mdも含めて、完全にClaude 4.0で生成しました。この部分を書いているSeikenは、空のcmakeプロジェクトを作製し、サイン波を生成するプログラムを作成するように指示をしただけです。もうこんな時代なのですね。
