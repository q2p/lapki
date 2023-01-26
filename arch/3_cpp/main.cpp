#include <iostream>
#include <chrono>
#include <fstream>
#include <cmath>
#include <cstring>

enum Mem {
  MEM_NONE = 0,
  MEM_RAM = 1,
  MEM_HDD = 2,
  MEM_SSD = 3,
  MEM_FLASH = 4,
};

constexpr double MEBI = 1024*1024;

constexpr const char* PATH_SSD = "C:/Users/danbit/Desktop/ssd.bin";
constexpr const char* PATH_HDD = "W:/hdd.bin";
constexpr const char* PATH_FLASH = "H:/flash.bin";

size_t block_size = 0;
int32_t tests = 0;

void writeCsv(
  std::ofstream& csv, const char* storage, size_t buffer, int32_t i,
  double write_time, double write_total,
  double read_time, double read_total
) {
  double write_avg = write_total / (double) i;
  double write_abs_err = fabs(write_time - write_avg);

  double read_avg = read_total / (double) i;
  double read_abs_err = fabs(read_time - read_avg);

  csv << storage << ";"
      << block_size << ";"
      << "uint8_t;"
      << buffer << ";"
      << i << ";"
      << "high_resolution_clock;"
      << write_time << ";"
      << write_avg << ";"
      << block_size / (write_avg * MEBI) << ";"
      << write_abs_err << ";"
      << write_abs_err / write_avg << ";"
      << read_time << ";"
      << read_avg << ";"
      << block_size / (read_avg * MEBI) << ";"
      << read_abs_err << ";"
      << read_abs_err / read_avg << "\n";
}

template<size_t BUFFER>
void testRam(std::ofstream& csv) {
  auto big = new alignas(16) uint8_t[block_size];
  auto small = new alignas(16) uint8_t[BUFFER];
  auto big_end = big + ((block_size / BUFFER - 1) * BUFFER);

  // Прогрев кэшей
  memset(big, 42, block_size);

  double write_total = 0.0;
  double read_total = 0.0;
  for (int32_t i = 1; i <= tests; i++) {
    for (size_t j = 0; j != BUFFER; j++) {
      small[j] = rand();
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (auto b = big; b != big_end; b += BUFFER) {
      memcpy(b, small, BUFFER);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto write_time = std::chrono::duration<double>(end - start).count();
    write_total += write_time;

    start = std::chrono::high_resolution_clock::now();
    for (auto b = big; b != big_end; b += BUFFER) {
      memcpy(small, b, BUFFER);
    }
    end = std::chrono::high_resolution_clock::now();
    auto read_time = std::chrono::duration<double>(end - start).count();
    read_total += read_time;

    writeCsv(csv, "RAM", BUFFER, i, write_time, write_total, read_time, read_total);
  }

  delete[] small;
  delete[] big;
}

void testStorage(std::ofstream& csv, const char* storage, const char* path) {
  auto big = new char[block_size];

  double write_total = 0.0;
  double read_total = 0.0;

  std::fstream file;

  for (int32_t i = 1; i <= tests; i++) {
    for (size_t j = 0; j != block_size; j++) {
      big[j] = rand();
    }

    file.open(path, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);

    auto start = std::chrono::high_resolution_clock::now();

    // Если файл не закрыть после записи, то
    // буффер не будет сбрасываться. И последующие
    // операции записи и чтения пройдут мгновенно.
    file.write(big, block_size);
    file.flush();
    file.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto write_time = std::chrono::duration<double>(end - start).count();
    write_total += write_time;

    file.open(path, std::ios_base::binary | std::ios_base::in);
    file.seekg(0);

    start = std::chrono::high_resolution_clock::now();

    file.read(big, block_size);

    end = std::chrono::high_resolution_clock::now();
    auto read_time = std::chrono::duration<double>(end - start).count();
    read_total += read_time;

    file.close();

    writeCsv(csv, storage, block_size, i, write_time, write_total, read_time, read_total);
  }

  delete[] big;
}

int main(int argc, char* argv[]) {
  srand(time(NULL));

  auto memtype = Mem::MEM_NONE;

  for (size_t i = 1; i+1 < argc; i += 2) {
    auto key = argv[i];
    auto val = argv[i+1];
    if (strcmp(key, "-m") == 0 || strcmp(key, "--memory-type") == 0) {
      if (strcmp(val, "RAM") == 0) {
        memtype = Mem::MEM_RAM;
      } else if (strcmp(val, "SSD") == 0) {
        memtype = Mem::MEM_SSD;
      } else if (strcmp(val, "HDD") == 0) {
        memtype = Mem::MEM_HDD;
      } else if (strcmp(val, "flash") == 0) {
        memtype = Mem::MEM_FLASH;
      } else {
        std::cout << "Bad memory type\n";
        return 1;
      }
    } else if (strcmp(key, "-b") == 0 || strcmp(key, "--block-size") == 0) {
      auto len = strlen(val);
      block_size = 1;
      if (len) {
        auto last = val[len-1];
        if (last == 'b' || last == 'B') {
          len--;
          if (len) {
            last = val[len-1];
            if (last == 'k' || last == 'K') {
              block_size = 1024;
            } else 
            if (last == 'm' || last == 'M') {
              block_size = 1024*1024;
            }
            len--;
          }
        }
      }
      val[len] = '\0';
      char* end;
      block_size *= std::strtoull(val, &end, 10);
      if (block_size == 0 || val + len != end) {
        std::cout << "Bad block size\n";
        return 1;
      }
    } else if (strcmp(key, "-l") == 0 || strcmp(key, "--launch-count") == 0) {
      char* end;
      tests = std::strtoll(val, &end, 10);
      if (tests == 0 || val + strlen(val) != end) {
        std::cout << "Bad launch count\n";
        return 1;
      }
    } else {
      std::cout << "Bad argument \"" << key << "\"\n";
      return 1;
    }
  }

  if (memtype == Mem::MEM_NONE || block_size == 0 || tests == 0) {
    std::cout << "Not enough arguments\n";
    return 1;
  }

  std::ofstream csv("results.csv", std::ios::ate | std::ios::app);
  if (csv.tellp() == 0) {
    csv << "MemoryType;" << "BlockSize;" << "ElementType;" << "BufferSize;"
      << "LaunchNum;" << "Timer;" << "WriteTime;" << "AverageWriteTime;"
      << "WriteBandwidth;" << "AbsError (write);" << "RelError (write);" << "ReadTime;"
      << "AverageReadTime;" << "ReadBandwidth;" << "AbsError (read);" << "RelError (read)" << "\n";
  }
  csv.precision(8);
  
  switch (memtype) {
    case Mem::MEM_RAM:   {
      if (block_size >= 256) {
        testRam<256>(csv);
      } else if (block_size >= 64) {
        testRam<64>(csv);
      } else if (block_size >= 8) {
        testRam<8>(csv);
      } else if (block_size >= 4) {
        testRam<4>(csv);
      } else {
        testRam<1>(csv);
      }
      break;
    }
    case Mem::MEM_SSD:   testStorage(csv, "SSD",   PATH_SSD);   break;
    case Mem::MEM_HDD:   testStorage(csv, "HDD",   PATH_HDD);   break;
    case Mem::MEM_FLASH: testStorage(csv, "flash", PATH_FLASH); break;
  }

  csv.flush();
  csv.close();

  return 0;
}
