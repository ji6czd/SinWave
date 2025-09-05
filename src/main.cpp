#include <getopt.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "wave_generator.hpp"

void printUsage(const char* program_name) {
  std::cout << "Usage:" << std::endl;
  std::cout << "  " << program_name
            << " <wave_type> array -f<frequency> [-a<amplitude>] <output.c>"
            << std::endl;
  std::cout << "  " << program_name
            << " <wave_type> wave -d<duration> [-f<frequency>] [-a<amplitude>] "
               "[-s<sample_rate>] <output.wav>"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Wave Types:" << std::endl;
  std::cout << "  sine                 : サイン波" << std::endl;
  std::cout << "  noise                : ホワイトノイズ" << std::endl;
  std::cout << std::endl;
  std::cout << "Output Formats:" << std::endl;
  std::cout
      << "  array                : 1周期分のC言語配列を生成（サイン波のみ）"
      << std::endl;
  std::cout << "  wave                 : WAVファイルを生成" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -f<frequency>        : 周波数 (Hz) 例: -f440" << std::endl;
  std::cout << "                         サイン波: 基音周波数" << std::endl;
  std::cout << "                         ホワイトノイズ: "
               "上限周波数（省略時は制限なし）"
            << std::endl;
  std::cout
      << "  -d<duration>         : 継続時間 (秒) 例: -d0.5 (waveコマンドのみ)"
      << std::endl;
  std::cout
      << "  -a<amplitude>        : 振幅 (0.0-1.0) デフォルト: 0.8 例: -a0.8"
      << std::endl;
  std::cout << "  -s<sample_rate>      : サンプリング周波数 (Hz) デフォルト: "
               "44100 例: -s48000"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Examples:" << std::endl;
  std::cout << "  " << program_name << " sine array -f440 -a0.8 output.c"
            << std::endl;
  std::cout << "  " << program_name
            << " sine wave -f440 -d1.0 -s48000 tuning.wav" << std::endl;
  std::cout << "  " << program_name
            << " noise wave -d2.0 -f8000 -a0.5 whitenoise.wav" << std::endl;
  std::cout << "  " << program_name
            << " noise wave -d1.0 -a0.3 fullband_noise.wav" << std::endl;
}

// オプション文字列から数値を解析
bool parseOption(const char* option, const char* prefix, double& result) {
  if (strncmp(option, prefix, strlen(prefix)) != 0) {
    return false;
  }

  const char* value_str = option + strlen(prefix);
  if (*value_str == '\0') {
    return false;
  }

  char* endptr;
  errno = 0;
  double value = std::strtod(value_str, &endptr);

  if (errno != 0 || *endptr != '\0') {
    return false;
  }

  result = value;
  return true;
}

// オプション文字列から整数を解析
bool parseOption(const char* option, const char* prefix, uint32_t& result) {
  if (strncmp(option, prefix, strlen(prefix)) != 0) {
    return false;
  }

  const char* value_str = option + strlen(prefix);
  if (*value_str == '\0') {
    return false;
  }

  char* endptr;
  errno = 0;
  unsigned long value = std::strtoul(value_str, &endptr, 10);

  if (errno != 0 || *endptr != '\0' || value > UINT32_MAX) {
    return false;
  }

  result = static_cast<uint32_t>(value);
  return true;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    printUsage(argv[0]);
    return 1;
  }

  std::string wave_type_str = argv[1];
  std::string command = argv[2];

  // 波形タイプの解析
  WaveGenerator::WaveType wave_type;
  if (wave_type_str == "sine") {
    wave_type = WaveGenerator::WaveType::SINE;
  } else if (wave_type_str == "noise") {
    wave_type = WaveGenerator::WaveType::WHITE_NOISE;
  } else {
    std::cerr << "Error: Invalid wave type '" << wave_type_str << "'"
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  if (command != "array" && command != "wave") {
    std::cerr << "Error: Invalid output format '" << command << "'"
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // arrayコマンドはサイン波のみサポート
  if (command == "array" && wave_type != WaveGenerator::WaveType::SINE) {
    std::cerr << "Error: Array output is only supported for sine waves"
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // パラメータの初期化
  double frequency = 0.0;  // ホワイトノイズの場合は上限周波数（0なら制限なし）
  double duration = 0.0;
  double amplitude = 0.8;
  uint32_t sample_rate = 44100;
  std::string output_file;

  bool frequency_set = false;
  bool duration_set = false;

  // オプション解析
  for (int i = 3; i < argc - 1; ++i) {
    double double_val;
    uint32_t uint_val;

    if (parseOption(argv[i], "-f", double_val)) {
      frequency = double_val;
      frequency_set = true;
    } else if (parseOption(argv[i], "-d", double_val)) {
      duration = double_val;
      duration_set = true;
    } else if (parseOption(argv[i], "-a", double_val)) {
      amplitude = double_val;
    } else if (parseOption(argv[i], "-s", uint_val)) {
      sample_rate = uint_val;
    } else {
      std::cerr << "Error: Unknown option '" << argv[i] << "'" << std::endl;
      printUsage(argv[0]);
      return 1;
    }
  }

  // 出力ファイル名
  output_file = argv[argc - 1];

  // 必須パラメータのチェック
  if (wave_type == WaveGenerator::WaveType::SINE && !frequency_set) {
    std::cerr << "Error: Frequency (-f) is required for sine wave" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  if (command == "wave" && !duration_set) {
    std::cerr << "Error: Duration (-d) is required for wave command"
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // パラメータの妥当性チェック
  WaveGenerator::WaveParams params;
  params.sample_rate = sample_rate;
  params.frequency = frequency;
  params.duration =
      (command == "array") ? 1.0 : duration;  // arrayの場合はdurationは使わない
  params.amplitude = amplitude;
  params.wave_type = wave_type;

  if (!WaveGenerator::validateParams(params)) {
    return 1;
  }

  // WaveGeneratorを作成
  WaveGenerator generator(params);

  if (command == "array") {
    // 1周期分のC配列を生成（サイン波のみ）
    std::cout << "Generating C array for sine wave frequency " << frequency
              << " Hz..." << std::endl;
    if (!generator.saveOneCycleAsCArray(output_file, "sine_wave_data")) {
      std::cerr << "Error: Failed to save C array file" << std::endl;
      return 1;
    }
    std::cout << "Success: C array saved to " << output_file << std::endl;
  } else if (command == "wave") {
    // WAVファイルを生成
    std::cout << "Generating " << wave_type_str << " wave:" << std::endl;
    std::cout << "  Sample rate: " << params.sample_rate << " Hz" << std::endl;
    if (wave_type == WaveGenerator::WaveType::SINE) {
      std::cout << "  Frequency: " << params.frequency << " Hz" << std::endl;
    } else if (wave_type == WaveGenerator::WaveType::WHITE_NOISE) {
      if (frequency_set && frequency > 0) {
        std::cout << "  Upper frequency limit: " << params.frequency << " Hz"
                  << std::endl;
      } else {
        std::cout << "  Full bandwidth white noise" << std::endl;
      }
    }
    std::cout << "  Duration: " << params.duration << " seconds" << std::endl;
    std::cout << "  Amplitude: " << params.amplitude << std::endl;

    std::vector<int16_t> pcm_data = generator.generateWave();

    if (pcm_data.empty()) {
      std::cerr << "Error: Failed to generate wave data" << std::endl;
      return 1;
    }

    if (!generator.saveAsWAV(pcm_data, output_file)) {
      std::cerr << "Error: Failed to save WAV file" << std::endl;
      return 1;
    }
    std::cout << "Success: WAV file saved to " << output_file << std::endl;
  }

  return 0;
}
