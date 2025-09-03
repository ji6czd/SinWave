#include "wave_generator.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

WaveGenerator::WaveGenerator(const WaveParams& params) : params_(params) {
  // パラメータの妥当性チェックは呼び出し側で行う前提
}

bool WaveGenerator::validateParams(const WaveParams& params) {
  if (params.sample_rate == 0) {
    std::cerr << "Error: Sample rate must be greater than 0" << std::endl;
    return false;
  }
  if (params.frequency <= 0) {
    std::cerr << "Error: Frequency must be greater than 0" << std::endl;
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

  std::cout << "Generating sine wave:" << std::endl;
  std::cout << "  Sample rate: " << params_.sample_rate << " Hz" << std::endl;
  std::cout << "  Frequency: " << params_.frequency << " Hz" << std::endl;
  std::cout << "  Duration: " << params_.duration << " seconds" << std::endl;
  std::cout << "  Total samples: " << total_samples << std::endl;
  std::cout << "  Amplitude: " << params_.amplitude << std::endl;

  for (size_t i = 0; i < total_samples; ++i) {
    const double time = i * sample_period;
    const double sine_value = std::sin(angular_frequency * time);
    pcm_data[i] = static_cast<int16_t>(sine_value * max_amplitude);
  }

  return pcm_data;
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
  std::cout << "WAV file saved: " << filename << " (" << data_size
            << " bytes of PCM data)" << std::endl;
  return true;
}

void WaveGenerator::printPCMData(const std::vector<int16_t>& pcm_data,
                                 size_t max_samples) {
  std::cout << "\nPCM Data (first " << std::min(max_samples, pcm_data.size())
            << " samples):" << std::endl;
  std::cout << "Sample#\tValue\tHex" << std::endl;
  std::cout << "-------\t-----\t---" << std::endl;

  for (size_t i = 0; i < std::min(max_samples, pcm_data.size()); ++i) {
    std::cout << std::setw(7) << i << "\t" << std::setw(6) << pcm_data[i]
              << "\t0x" << std::hex << std::setw(4) << std::setfill('0')
              << (static_cast<uint16_t>(pcm_data[i]) & 0xFFFF) << std::dec
              << std::setfill(' ') << std::endl;
  }

  if (pcm_data.size() > max_samples) {
    std::cout << "... (" << (pcm_data.size() - max_samples) << " more samples)"
              << std::endl;
  }
}

bool WaveGenerator::saveAsCArray(const std::vector<int16_t>& pcm_data,
                                 const std::string& filename,
                                 const std::string& array_name) {
  std::ofstream file(filename);
  if (!file) {
    std::cerr << "Error: Cannot create file " << filename << std::endl;
    return false;
  }

  // ヘッダー部分
  file << "// Generated C array for sine wave data (1 week duration)\n";
  file << "// Sample rate: " << params_.sample_rate << " Hz\n";
  file << "// Frequency: " << params_.frequency << " Hz\n";
  file << "// Duration: " << params_.duration << " seconds\n";
  file << "// Total samples: " << pcm_data.size() << "\n\n";

  file << "#include <stdint.h>\n\n";
  file << "const int16_t " << array_name << "[] = {\n";

  // データ部分（16個ごとに改行）
  for (size_t i = 0; i < pcm_data.size(); ++i) {
    if (i % 16 == 0) {
      file << "    ";
    }
    file << pcm_data[i];
    if (i < pcm_data.size() - 1) {
      file << ",";
      if ((i + 1) % 16 == 0) {
        file << "\n";
      } else {
        file << " ";
      }
    }
  }

  file << "\n};\n\n";
  file << "const size_t " << array_name << "_size = " << pcm_data.size()
       << ";\n";

  file.close();
  std::cout << "C array file saved: " << filename << " (1 week data, "
            << pcm_data.size() << " samples)" << std::endl;
  return true;
}

bool WaveGenerator::saveOneCycleAsCArray(const std::string& filename,
                                         const std::string& array_name) {
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
  std::cout << "C array file saved: " << filename << " (1 cycle data, "
            << cycle_data.size() << " samples)" << std::endl;
  return true;
}
