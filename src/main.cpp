#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

#include "wave_generator.hpp"

// 文字列を無符号整数に変換（エラーチェック付き）
bool stringToUInt(const char* str, uint32_t& result) {
  if (!str || *str == '\0') {
    return false;
  }

  char* endptr;
  errno = 0;
  unsigned long value = std::strtoul(str, &endptr, 10);

  if (errno != 0 || *endptr != '\0' || value > UINT32_MAX) {
    return false;
  }

  result = static_cast<uint32_t>(value);
  return true;
}

// 文字列を倍精度浮動小数点数に変換（エラーチェック付き）
bool stringToDouble(const char* str, double& result) {
  if (!str || *str == '\0') {
    return false;
  }

  char* endptr;
  errno = 0;
  double value = std::strtod(str, &endptr);

  if (errno != 0 || *endptr != '\0') {
    return false;
  }

  result = value;
  return true;
}

void printUsage(const char* program_name) {
  std::cout << "Usage: " << program_name
            << " <sample_rate> <frequency> <duration> [amplitude] "
               "[output_file] [--c-array] [--c-cycle]"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Parameters:" << std::endl;
  std::cout << "  sample_rate  : サンプリング周波数 (Hz) 例: 44100"
            << std::endl;
  std::cout << "  frequency    : サイン波の周波数 (Hz) 例: 440" << std::endl;
  std::cout << "  duration     : 継続時間 (秒) 例: 1.0" << std::endl;
  std::cout << "  amplitude    : 振幅 (0.0-1.0) デフォルト: 0.8" << std::endl;
  std::cout << "  output_file  : 出力WAVファイル名 デフォルト: output.wav"
            << std::endl;
  std::cout << "  --c-array    : 1週間分のC言語配列ファイルも生成" << std::endl;
  std::cout << "  --c-cycle    : 1サイクル分のC言語配列ファイルも生成"
            << std::endl;
  std::cout << std::endl;
  std::cout << "例:" << std::endl;
  std::cout << "  " << program_name
            << " 44100 440 1.0        # A4音を1秒間、44.1kHzで生成"
            << std::endl;
  std::cout << "  " << program_name
            << " 48000 880 0.5 0.5    # A5音を0.5秒間、48kHz、振幅0.5で生成"
            << std::endl;
  std::cout << "  " << program_name
            << " 44100 440 1.0 0.8 output.wav --c-array  # C配列も生成"
            << std::endl;
  std::cout << "  " << program_name
            << " 44100 440 1.0 0.8 output.wav --c-cycle  # 1サイクル分C配列生成"
            << std::endl;
}

int main(int argc, char const* argv[]) {
  if (argc < 4 || argc > 8) {
    printUsage(argv[0]);
    return 1;
  }

  // --c-arrayおよび--c-cycleオプションをチェック
  bool generate_c_array = false;
  bool generate_c_cycle = false;
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--c-array") {
      generate_c_array = true;
    } else if (std::string(argv[i]) == "--c-cycle") {
      generate_c_cycle = true;
    }
  }

  // コマンドライン引数の解析
  WaveGenerator::WaveParams params;

  // サンプルレートの解析
  if (!stringToUInt(argv[1], params.sample_rate)) {
    std::cerr << "Error: Invalid sample rate: " << argv[1] << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // 周波数の解析
  if (!stringToDouble(argv[2], params.frequency)) {
    std::cerr << "Error: Invalid frequency: " << argv[2] << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // 継続時間の解析
  if (!stringToDouble(argv[3], params.duration)) {
    std::cerr << "Error: Invalid duration: " << argv[3] << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // 振幅の解析（オプション）
  params.amplitude = 0.8;  // デフォルト値
  int arg_index = 4;
  if (argc >= 5 && std::string(argv[4]) != "--c-array" &&
      std::string(argv[4]) != "--c-cycle") {
    if (!stringToDouble(argv[4], params.amplitude)) {
      std::cerr << "Error: Invalid amplitude: " << argv[4] << std::endl;
      printUsage(argv[0]);
      return 1;
    }
    arg_index = 5;
  }

  // 出力ファイル名の解析
  std::string output_file = "output.wav";  // デフォルト値
  if (argc > arg_index && std::string(argv[arg_index]) != "--c-array" &&
      std::string(argv[arg_index]) != "--c-cycle") {
    output_file = std::string(argv[arg_index]);
  }

  // パラメータの妥当性をチェック
  if (!WaveGenerator::validateParams(params)) {
    printUsage(argv[0]);
    return 1;
  }

  // WaveGeneratorを作成してサイン波を生成
  WaveGenerator generator(params);
  std::vector<int16_t> pcm_data = generator.generateSineWave();

  // PCMデータの一部を表示
  generator.printPCMData(pcm_data, 20);

  // WAVファイルとして保存
  if (!generator.saveAsWAV(pcm_data, output_file)) {
    std::cerr << "Error: Failed to save WAV file" << std::endl;
    return 1;
  }

  std::cout << "\n成功: サイン波のPCMデータを生成し、" << output_file
            << " に保存しました。" << std::endl;

  // C配列ファイルの生成（オプション）
  if (generate_c_array) {
    std::string c_array_file = output_file;
    size_t dot_pos = c_array_file.find_last_of('.');
    if (dot_pos != std::string::npos) {
      c_array_file = c_array_file.substr(0, dot_pos) + "_week.c";
    } else {
      c_array_file += "_week.c";
    }

    std::cout << "\n1週間分のC言語配列ファイルを生成中..." << std::endl;
    if (!generator.saveAsCArray(pcm_data, c_array_file, "sine_wave_week")) {
      std::cerr << "Warning: Failed to save C array file" << std::endl;
    } else {
      std::cout << "成功: C言語配列ファイルを " << c_array_file
                << " に保存しました。" << std::endl;
    }
  }

  // 1サイクル分C配列ファイルの生成（オプション）
  if (generate_c_cycle) {
    std::string c_cycle_file = output_file;
    size_t dot_pos = c_cycle_file.find_last_of('.');
    if (dot_pos != std::string::npos) {
      c_cycle_file = c_cycle_file.substr(0, dot_pos) + "_cycle.c";
    } else {
      c_cycle_file += "_cycle.c";
    }

    std::cout << "\n1サイクル分のC言語配列ファイルを生成中..." << std::endl;
    if (!generator.saveOneCycleAsCArray(c_cycle_file, "sine_wave_cycle")) {
      std::cerr << "Warning: Failed to save C cycle array file" << std::endl;
    } else {
      std::cout << "成功: 1サイクル分C言語配列ファイルを " << c_cycle_file
                << " に保存しました。" << std::endl;
    }
  }

  return 0;
}
