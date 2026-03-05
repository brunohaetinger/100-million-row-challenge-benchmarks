#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

constexpr size_t DATE_LENGTH = 10;
constexpr size_t CHUNK_SIZE = 64 * 1024 * 1024;

struct AggregatedData {
    std::unordered_map<std::string, std::unordered_map<std::string, int>> data;
    std::mutex mutex;
};

size_t find_next_newline(const char* data, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        if (data[i] == '\n') {
            return i;
        }
    }
    return end;
}

size_t find_prev_newline(const char* data, size_t pos) {
    if (pos == 0) return 0;
    for (size_t i = pos; i > 0; --i) {
        if (data[i - 1] == '\n') {
            return i;
        }
    }
    return 0;
}

void process_chunk(
    const char* file_data,
    size_t start_pos,
    size_t end_pos,
    AggregatedData& result
) {
    std::unordered_map<std::string, std::unordered_map<std::string, int>> local_result;

    size_t pos = start_pos;
    while (pos < end_pos) {
        size_t line_end = find_next_newline(file_data, pos, end_pos);
        if (line_end == end_pos) break;

        size_t line_length = line_end - pos;
        if (line_length == 0) {
            pos = line_end + 1;
            continue;
        }

        const char* line = file_data + pos;

        const char* comma = nullptr;
        for (size_t i = 0; i < line_length; ++i) {
            if (line[i] == ',') {
                comma = line + i;
                break;
            }
        }

        if (!comma) {
            pos = line_end + 1;
            continue;
        }

        const char* url_start = line;
        size_t url_length = comma - line;

        const char* timestamp_start = comma + 1;
        if (timestamp_start >= file_data + line_end) {
            pos = line_end + 1;
            continue;
        }

        size_t path_start = 0;
        for (size_t i = 8; i < url_length; ++i) {
            if (url_start[i] == '/') {
                path_start = i;
                break;
            }
        }

        if (path_start == 0) {
            pos = line_end + 1;
            continue;
        }

        std::string path(url_start + path_start, url_length - path_start);
        std::string date(timestamp_start, DATE_LENGTH);

        local_result[path][date]++;

        pos = line_end + 1;
    }

    std::lock_guard<std::mutex> lock(result.mutex);
    for (auto& path_entry : local_result) {
        for (auto& date_entry : path_entry.second) {
            result.data[path_entry.first][date_entry.first] += date_entry.second;
        }
    }
}

std::string format_json(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& data) {
    std::string result = "{\n";
    
    std::vector<std::string> paths;
    paths.reserve(data.size());
    for (const auto& entry : data) {
        paths.push_back(entry.first);
    }
    std::sort(paths.begin(), paths.end());
    
    bool first_path = true;
    for (const auto& path : paths) {
        if (!first_path) result += ",\n";
        first_path = false;
        
        result += "  \"" + path + "\": {\n";
        
        const auto& dates = data.at(path);
        std::vector<std::string> date_keys;
        date_keys.reserve(dates.size());
        for (const auto& entry : dates) {
            date_keys.push_back(entry.first);
        }
        std::sort(date_keys.begin(), date_keys.end());
        
        bool first_date = true;
        for (const auto& date : date_keys) {
            if (!first_date) result += ",\n";
            first_date = false;
            
            result += "    \"" + date + "\": " + std::to_string(dates.at(date));
        }
        
        result += "\n  }";
    }
    
    result += "\n}";
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];

    int fd = open(input_file.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error: Cannot open file " << input_file << std::endl;
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Error: Cannot get file size" << std::endl;
        close(fd);
        return 1;
    }

    size_t file_size = sb.st_size;
    
    const char* file_data = static_cast<const char*>(
        mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0)
    );
    
    if (file_data == MAP_FAILED) {
        std::cerr << "Error: mmap failed" << std::endl;
        close(fd);
        return 1;
    }

    madvise(const_cast<char*>(file_data), file_size, MADV_SEQUENTIAL);

    size_t num_workers = std::thread::hardware_concurrency();
    if (num_workers == 0) num_workers = 4;

    std::cout << "Processing file: " << input_file << std::endl;
    std::cout << "File size: " << (file_size / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Workers: " << num_workers << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    size_t num_chunks = std::min(file_size / CHUNK_SIZE + 1, (size_t)num_workers);
    size_t chunk_size = file_size / num_chunks;

    std::vector<std::thread> threads;
    AggregatedData result;

    for (size_t i = 0; i < num_chunks; ++i) {
        size_t start_pos = i * chunk_size;
        size_t end_pos = (i == num_chunks - 1) ? file_size : (i + 1) * chunk_size;

        if (i > 0) {
            start_pos = find_next_newline(file_data, start_pos, end_pos);
        }

        if (i < num_chunks - 1) {
            end_pos = find_prev_newline(file_data, end_pos);
        }

        if (start_pos >= end_pos) continue;

        threads.emplace_back(process_chunk, std::ref(file_data), start_pos, end_pos, std::ref(result));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::string output = format_json(result.data);

    std::ofstream out_file("output.json");
    out_file << output;
    out_file.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Processed in " << (duration.count() / 1000.0) << " seconds" << std::endl;
    std::cout << "Output written to: output.json" << std::endl;

    munmap(const_cast<char*>(file_data), file_size);
    close(fd);

    return 0;
}
