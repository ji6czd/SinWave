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
            << " array -f<frequency> [-a<amplitude>] <output.c>" << std::endl;
  std::cout << "  " << program_name
            << " wave -f<frequency> -d<duration> [-a<amplitude>] "
               "[-s<sample_rate>] <output.wav>"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Commands:" << std::endl;
  std::cout << "  array                : 1周期分のC言語配列を生成" << std::endl;
  std::cout << "  wave                 : WAVファイルを生成" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -f<frequency>        : 周波数 (Hz) 例: -f440" << std::endl;
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
  std::cout << "  " << program_name << " array -f48000 -a0.8 output.c"
            << std::endl;
  std::cout << "  " << program_name << " wave -f48000 -d0.5 -a0.8 output.wav"
            << std::endl;
  std::cout << "  " << program_name << " wave -f440 -d1.0 -s48000 tuning.wav"
            << std::endl;
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
  if (argc < 3) {
    printUsage(argv[0]);
    return 1;
  }

  std::string command = argv[1];
  if (command != "array" && command != "wave") {
    std::cerr << "Error: Invalid command '" << command << "'" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // パラメータの初期化
  double frequency = 0.0;
  double duration = 0.0;
  double amplitude = 0.8;
  uint32_t sample_rate = 44100;
  std::string output_file;

  bool frequency_set = false;
  bool duration_set = false;

  // オプション解析
  for (int i = 2; i < argc - 1; ++i) {
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
  if (!frequency_set) {
    std::cerr << "Error: Frequency (-f) is required" << std::endl;
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

  if (!WaveGenerator::validateParams(params)) {
    return 1;
  }

  // WaveGeneratorを作成
  WaveGenerator generator(params);

  if (command == "array") {
    // 1周期分のC配列を生成
    std::cout << "Generating C array for frequency " << frequency << " Hz..."
              << std::endl;
    if (!generator.saveOneCycleAsCArray(output_file, "sine_wave_data")) {
      std::cerr << "Error: Failed to save C array file" << std::endl;
      return 1;
    }
    std::cout << "Success: C array saved to " << output_file << std::endl;
  } else if (command == "wave") {
    // WAVファイルを生成
    std::cout << "Generating sine wave:" << std::endl;
    std::cout << "  Sample rate: " << params.sample_rate << " Hz" << std::endl;
    std::cout << "  Frequency: " << params.frequency << " Hz" << std::endl;
    std::cout << "  Duration: " << params.duration << " seconds" << std::endl;
    std::cout << "  Amplitude: " << params.amplitude << std::endl;

    std::vector<int16_t> pcm_data = generator.generateSineWave();

    if (!generator.saveAsWAV(pcm_data, output_file)) {
      std::cerr << "Error: Failed to save WAV file" << std::endl;
      return 1;
    }
    std::cout << "Success: WAV file saved to " << output_file << std::endl;
  }

  return 0;
}
