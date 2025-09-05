# SinWave - 音響信号生成コマンド

## このコマンドの機能

SinWaveは、サイン波とホワイトノイズを生成するコマンドラインツールです。以下の主要な機能を提供します：

### 波形の種類

1. **サイン波**: 指定した周波数の純音を生成
2. **ホワイトノイズ**: ランダムな雑音信号を生成（フルバンドまたは帯域制限）

### 出力形式

1. **C言語配列の生成**: 1周期分のサイン波データをC言語の配列定義として出力（サイン波のみ）
2. **WAVファイルの生成**: 指定した長さの音響信号をWAVオーディオファイルとして出力

このツールは、組み込み開発でのサイン波テーブル生成、音響テスト、楽器の調律、音声信号処理の学習、ノイズ生成などに活用できます。

## 起動パラメータの説明

### コマンド形式

```bash
# C言語配列の生成（サイン波のみ）
sinwave sine array -f<frequency> [-a<amplitude>] <output.c>

# WAVファイルの生成
sinwave <wave_type> wave -d<duration> [options] <output.wav>
```

### 波形タイプ

- `sine` : サイン波を生成します
- `noise` : ホワイトノイズを生成します

### 出力モード

- `array` : 1周期分のC言語配列を生成します（サイン波のみサポート）
- `wave`  : WAVオーディオファイルを生成します

### オプション

#### `-f<frequency>` : 周波数設定

- **サイン波の場合**: 基音周波数（Hz）【必須】
  - 例: `-f440`（A4音）、`-f1000`（1kHz）
- **ホワイトノイズの場合**: 上限周波数（Hz）【省略可能】
  - 例: `-f8000`（8kHz以下に制限）
  - 省略時: フルバンドホワイトノイズ

#### `-d<duration>` : 継続時間（秒）【waveコマンドで必須】

- 例: `-d0.5`（0.5秒）、`-d2.0`（2秒）

#### `-a<amplitude>` : 振幅（0.0～1.0）【省略可能】

- デフォルト: 0.8
- 例: `-a0.5`（半分の音量）、`-a1.0`（最大音量）

#### `-s<sample_rate>` : サンプリング周波数（Hz）【省略可能】

- デフォルト: 44100
- 例: `-s48000`（48kHz）、`-s96000`（96kHz）

## サンプルコマンド

### サイン波 - C言語配列生成の例

```bash
# 440Hzの1周期分をC配列として生成
sinwave sine array -f440 tuning_A4.c

# 1kHzの1周期分を振幅0.5でC配列として生成
sinwave sine array -f1000 -a0.5 test_1khz.c

# 高周波（10kHz）のC配列を生成
sinwave sine array -f10000 high_freq.c
```

### サイン波 - WAVファイル生成の例

```bash
# 440Hzを1秒間、CD品質で生成
sinwave sine wave -f440 -d1.0 tuning_A4.wav

# 1kHzを0.5秒間、48kHzサンプリングレートで生成
sinwave sine wave -f1000 -d0.5 -s48000 test_1khz.wav

# 低音（100Hz）を2秒間、振幅0.3で生成
sinwave sine wave -f100 -d2.0 -a0.3 low_freq.wav

# 高品質（96kHz）で音楽用A音を生成
sinwave sine wave -f440 -d3.0 -s96000 -a0.7 reference_A4.wav
```

### ホワイトノイズ - WAVファイル生成の例

```bash
# フルバンドホワイトノイズを2秒間生成
sinwave noise wave -d2.0 -a0.5 fullband_noise.wav

# 8kHz以下に制限したホワイトノイズを1秒間生成
sinwave noise wave -d1.0 -f8000 -a0.6 limited_noise.wav

# 高品質（96kHz）でノイズ信号を生成
sinwave noise wave -d3.0 -f20000 -s96000 -a0.3 hq_noise.wav

# テスト用の短いノイズバースト
sinwave noise wave -d0.1 -f4000 -a0.8 test_burst.wav
```

### 組み込み開発での活用例

```bash
# マイコン用のサイン波テーブル生成（小さなメモリ使用量）
sinwave sine array -f1000 -s16000 sine_table.c

# PWM用の低周波サイン波テーブル
sinwave sine array -f50 -s8000 -a1.0 pwm_sine.c

# オーディオ処理用の基準信号生成
sinwave sine array -f440 audio_reference.c
```

## 出力ファイルについて

### C配列ファイルの特徴（サイン波のみ）

生成される`.c`ファイルには以下が含まれます：

- `int16_t`型の配列定義
- 配列サイズの定数
- 周波数とサンプリングレートの情報
- コンパイル可能なC言語ソースコード

例：

```c
// Generated C array for sine wave data (1 cycle)
// Sample rate: 44100 Hz
// Frequency: 1000 Hz
// Amplitude: 0.9
// Samples per cycle: 44

#include <stdint.h>

const int16_t sine_wave_data[] = {
    0, 4187, 8289, 12224, 15911, 19275, 22249, 24772, 26793, 28271, 29176, 29489,
    // ... (続く)
};

const size_t sine_wave_data_size = 44;
const double sine_wave_data_frequency = 1000;
const uint32_t sine_wave_data_sample_rate = 44100;
```

### WAVファイルの特徴

- 16bit PCM形式
- モノラル音声
- 指定したサンプリング周波数
- 標準的な音声プレイヤーで再生可能

## 使用用途

### サイン波の用途

- **組み込み開発**: マイコンでのサイン波テーブル生成
- **音響テスト**: スピーカーやオーディオ機器の動作確認
- **楽器調律**: 基準音の生成
- **信号処理学習**: デジタル信号処理の教材作成
- **周波数測定**: 測定器の校正用信号生成
- **PWM制御**: モーター制御用の滑らかな制御信号生成

### ホワイトノイズの用途

- **音響測定**: スピーカーやマイクの周波数特性測定
- **マスキング**: 環境音のマスキング用
- **オーディオテスト**: アンプやフィルターの動作確認
- **信号処理学習**: フィルター設計の検証用
- **音響実験**: 残響時間測定などの音響実験
- **睡眠補助**: 環境音として利用

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

## 技術的詳細

### ホワイトノイズの実装

- 乱数生成器: Mersenne Twister（`std::mt19937`）
- 帯域制限: 1次ローパスフィルタによる簡易実装
- 振幅制御: 指定された振幅に正規化

### サイン波の実装

- 高精度な数学関数（`std::sin`）を使用
- 位相の連続性を保持
- 16bit PCM形式で出力

## 追記

このプログラムは、README.mdも含めて、Claude AI（GitHub Copilot）で生成・拡張しました。初期のサイン波生成機能から、ホワイトノイズ生成機能の追加、コマンドライン引数の再設計まで、AIによって段階的に開発されています。現代のAI支援開発の一例として、実用的な音響信号生成ツールが完成しました。

Seikenは、仕様をプロンプトとしてClaudeに送り、コードの確認のみを行いました。仕様に従っていないものは、改めて仕様を送信し、人間はコードを変更していません。
