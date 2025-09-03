#pragma once

#include <cstdint>
#include <string>
#include <vector>

class WaveGenerator {
 public:
  struct WaveParams {
    uint32_t sample_rate;  // サンプリング周波数 (Hz)
    double frequency;      // 周波数 (Hz)
    double duration;       // 継続時間 (秒)
    double amplitude;      // 振幅 (0.0-1.0)
  };

  WaveGenerator(const WaveParams& params);

  // パラメータの妥当性をチェック
  static bool validateParams(const WaveParams& params);

  // サイン波のPCMデータを生成
  std::vector<int16_t> generateSineWave();

  // WAVファイルとして保存
  bool saveAsWAV(const std::vector<int16_t>& pcm_data,
                 const std::string& filename);

  // PCMデータをコンソールに出力
  void printPCMData(const std::vector<int16_t>& pcm_data,
                    size_t max_samples = 100);

  // C言語配列形式でPCMデータを保存（1週間分）
  bool saveAsCArray(const std::vector<int16_t>& pcm_data,
                    const std::string& filename,
                    const std::string& array_name = "wave_data");

  // C言語配列形式で1サイクル分のPCMデータを保存
  bool saveOneCycleAsCArray(const std::string& filename,
                            const std::string& array_name = "sine_wave_cycle");

 private:
  WaveParams params_;

  // WAVファイルヘッダー構造体
  struct WAVHeader {
    char riff[4];              // "RIFF"
    uint32_t chunk_size;       // ファイルサイズ - 8
    char wave[4];              // "WAVE"
    char fmt[4];               // "fmt "
    uint32_t fmt_size;         // フォーマットチャンクサイズ
    uint16_t format;           // オーディオフォーマット
    uint16_t channels;         // チャンネル数
    uint32_t sample_rate;      // サンプリング周波数
    uint32_t byte_rate;        // バイトレート
    uint16_t block_align;      // ブロックアライン
    uint16_t bits_per_sample;  // ビット深度
    char data[4];              // "data"
    uint32_t data_size;        // データサイズ
  };
};
