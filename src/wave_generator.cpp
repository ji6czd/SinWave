#include "wave_generator.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

WaveGenerator::WaveGenerator(const WaveParams& params) : params_(params) {
  // パラメータの妥当性チェックは呼び出し側で行う前提
}

bool WaveGenerator::validateParams(const WaveParams& params) {
  if (params.sample_rate == 0) {
    std::cerr << "Error: Sample rate must be greater than 0" << std::endl;
    return false;
  }
  if (params.wave_type == WaveType::SINE && params.frequency <= 0) {
    std::cerr << "Error: Frequency must be greater than 0 for sine wave"
              << std::endl;
    return false;
  }
  if (params.wave_type == WaveType::WHITE_NOISE && params.frequency > 0 &&
      params.frequency >= params.sample_rate / 2) {
    std::cerr
        << "Error: Upper frequency limit must be less than Nyquist frequency ("
        << (params.sample_rate / 2) << " Hz)" << std::endl;
    return false;
  }
  if (params.duration <= 0) {
    std::cerr << "Error: Duration must be greater than 0" << std::endl;
    return false;
  }
  if (params.amplitude < 0.0 || params.amplitude > 1.0) {
    std::cerr << "Error: Amplitude must be between 0.0 and 1.0" << std::endl;
    return false;
  }
  return true;
}

std::vector<int16_t> WaveGenerator::generateSineWave() {
  const size_t total_samples =
      static_cast<size_t>(params_.sample_rate * params_.duration);
  std::vector<int16_t> pcm_data(total_samples);

  const double angular_frequency = 2.0 * M_PI * params_.frequency;
  const double sample_period = 1.0 / params_.sample_rate;
  const int16_t max_amplitude =
      static_cast<int16_t>(params_.amplitude * 32767.0);

  for (size_t i = 0; i < total_samples; ++i) {
    const double time = i * sample_period;
    const double sine_value = std::sin(angular_frequency * time);
    pcm_data[i] = static_cast<int16_t>(sine_value * max_amplitude);
  }

  return pcm_data;
}

std::vector<int16_t> WaveGenerator::generateWhiteNoise() {
  const size_t total_samples =
      static_cast<size_t>(params_.sample_rate * params_.duration);
  std::vector<int16_t> pcm_data(total_samples);

  // 乱数生成器の初期化
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis(-1.0, 1.0);

  const int16_t max_amplitude =
      static_cast<int16_t>(params_.amplitude * 32767.0);

  // 上限周波数が指定されている場合は簡易的なローパスフィルタを適用
  if (params_.frequency > 0 && params_.frequency < params_.sample_rate / 2) {
    // 簡易的なローパスフィルタ係数（カットオフ周波数ベース）
    const double cutoff_ratio = params_.frequency / (params_.sample_rate / 2);
    const double alpha = std::exp(-2.0 * M_PI * cutoff_ratio);

    double prev_sample = 0.0;
    for (size_t i = 0; i < total_samples; ++i) {
      double noise_sample = dis(gen);
      // 1次ローパスフィルタ
      prev_sample = alpha * prev_sample + (1.0 - alpha) * noise_sample;
      pcm_data[i] = static_cast<int16_t>(prev_sample * max_amplitude);
    }
  } else {
    // フィルタなしのホワイトノイズ
    for (size_t i = 0; i < total_samples; ++i) {
      const double noise_sample = dis(gen);
      pcm_data[i] = static_cast<int16_t>(noise_sample * max_amplitude);
    }
  }

  return pcm_data;
}

std::vector<int16_t> WaveGenerator::generateWave() {
  switch (params_.wave_type) {
    case WaveType::SINE:
      return generateSineWave();
    case WaveType::WHITE_NOISE:
      return generateWhiteNoise();
    default:
      std::cerr << "Error: Unknown wave type" << std::endl;
      return std::vector<int16_t>();
  }
}

bool WaveGenerator::saveAsWAV(const std::vector<int16_t>& pcm_data,
                              const std::string& filename) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Cannot create file " << filename << std::endl;
    return false;
  }

  WAVHeader header;
  const uint32_t data_size = pcm_data.size() * sizeof(int16_t);

  // WAVヘッダーの設定
  std::memcpy(header.riff, "RIFF", 4);
  header.chunk_size = 36 + data_size;
  std::memcpy(header.wave, "WAVE", 4);
  std::memcpy(header.fmt, "fmt ", 4);
  header.fmt_size = 16;
  header.format = 1;    // PCM
  header.channels = 1;  // モノラル
  header.sample_rate = params_.sample_rate;
  header.byte_rate = params_.sample_rate * sizeof(int16_t);
  header.block_align = sizeof(int16_t);
  header.bits_per_sample = 16;
  std::memcpy(header.data, "data", 4);
  header.data_size = data_size;

  // ヘッダーを書き込み
  file.write(reinterpret_cast<const char*>(&header), sizeof(header));

  // PCMデータを書き込み
  file.write(reinterpret_cast<const char*>(pcm_data.data()), data_size);

  file.close();
  return true;
}

bool WaveGenerator::saveOneCycleAsCArray(const std::string& filename,
                                         const std::string& array_name) {
  // ホワイトノイズでは1サイクルの概念がないため、サイン波のみサポート
  if (params_.wave_type != WaveType::SINE) {
    std::cerr << "Error: C array output is only supported for sine waves"
              << std::endl;
    return false;
  }

  // 1サイクルに必要なサンプル数を計算
  const size_t samples_per_cycle =
      static_cast<size_t>(params_.sample_rate / params_.frequency);

  if (samples_per_cycle == 0) {
    std::cerr << "Error: Cannot generate cycle data - frequency too high for "
                 "sample rate"
              << std::endl;
    return false;
  }

  std::vector<int16_t> cycle_data(samples_per_cycle);

  const double angular_frequency = 2.0 * M_PI * params_.frequency;
  const double sample_period = 1.0 / params_.sample_rate;
  const int16_t max_amplitude =
      static_cast<int16_t>(params_.amplitude * 32767.0);

  // 1サイクル分のデータを生成
  for (size_t i = 0; i < samples_per_cycle; ++i) {
    const double time = i * sample_period;
    const double sine_value = std::sin(angular_frequency * time);
    cycle_data[i] = static_cast<int16_t>(sine_value * max_amplitude);
  }

  std::ofstream file(filename);
  if (!file) {
    std::cerr << "Error: Cannot create file " << filename << std::endl;
    return false;
  }

  // ヘッダー部分
  file << "// Generated C array for sine wave data (1 cycle)\n";
  file << "// Sample rate: " << params_.sample_rate << " Hz\n";
  file << "// Frequency: " << params_.frequency << " Hz\n";
  file << "// Amplitude: " << params_.amplitude << "\n";
  file << "// Samples per cycle: " << samples_per_cycle << "\n";
  file << "// Cycle duration: " << (1.0 / params_.frequency) << " seconds\n\n";

  file << "#include <stdint.h>\n\n";
  file << "const int16_t " << array_name << "[] = {\n";

  // データ部分（16個ごとに改行）
  for (size_t i = 0; i < cycle_data.size(); ++i) {
    if (i % 16 == 0) {
      file << "    ";
    }
    file << cycle_data[i];
    if (i < cycle_data.size() - 1) {
      file << ",";
      if ((i + 1) % 16 == 0) {
        file << "\n";
      } else {
        file << " ";
      }
    }
  }

  file << "\n};\n\n";
  file << "const size_t " << array_name << "_size = " << cycle_data.size()
       << ";\n";
  file << "const double " << array_name << "_frequency = " << params_.frequency
       << ";\n";
  file << "const uint32_t " << array_name
       << "_sample_rate = " << params_.sample_rate << ";\n";

  file.close();
  return true;
}
