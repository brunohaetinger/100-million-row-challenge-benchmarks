#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace py = pybind11;

constexpr size_t DATE_LEN = 10;
constexpr size_t BUFFER_SIZE = 64 * 1024 * 1024;

struct Result {
    std::unordered_map<std::string, std::unordered_map<std::string, int>> data;
};

size_t find_next_newline(const char* data, size_t start, size_t max_len) {
    for (size_t i = start; i < max_len; ++i) {
        if (data[i] == '\n') {
            return i;
        }
    }
    return max_len;
}

void process_chunk(
    const char* file_data, 
    size_t start_pos, 
    size_t end_pos, 
    Result& result
) {
    size_t pos = start_pos;
    while (pos < end_pos) {
        size_t line_end = find_next_newline(file_data, pos, end_pos);
        if (line_end == end_pos) break;

        size_t line_len = line_end - pos;
        if (line_len < 30) {
            pos = line_end + 1;
            continue;
        }

        const char* line = file_data + pos;
        const char* comma = nullptr;
        for (size_t i = 0; i < line_len; ++i) {
            if (line[i] == ',') {
                comma = line + i;
                break;
            }
        }
        if (!comma) {
            pos = line_end + 1;
            continue;
        }

        const char* url = line;
        size_t url_len = comma - line;

        const char* timestamp = comma + 1;
        if (timestamp + DATE_LEN > line + line_len) {
            pos = line_end + 1;
            continue;
        }

        size_t path_start = 0;
        for (size_t i = 8; i < url_len; ++i) {
            if (url[i] == '/') {
                path_start = i;
                break;
            }
        }
        if (path_start == 0) {
            pos = line_end + 1;
            continue;
        }

        std::string path(url + path_start, comma - (url + path_start));
        std::string date(timestamp, DATE_LEN);

        result.data[path][date]++;
        pos = line_end + 1;
    }
}

py::dict process_file(const std::string& file_path, int num_workers) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Cannot open file: " + file_path);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        throw std::runtime_error("Cannot get file size");
    }

    size_t file_size = sb.st_size;
    const char* file_data = (const char*)mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (file_data == MAP_FAILED) {
        close(fd);
        throw std::runtime_error("mmap failed");
    }

    madvise((void*)file_data, file_size, MADV_SEQUENTIAL);

    size_t chunk_size = file_size / num_workers;
    std::vector<std::thread> threads;
    std::vector<Result> results(num_workers);

    for (size_t i = 0; i < num_workers; ++i) {
        size_t start_pos = i * chunk_size;
        size_t end_pos = (i == num_workers - 1) ? file_size : (i + 1) * chunk_size;

        if (i > 0) {
            start_pos = find_next_newline(file_data, start_pos, file_size);
        }

        threads.emplace_back(process_chunk, file_data, start_pos, end_pos, std::ref(results[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    // Merge results
    Result final_result;
    for (auto& res : results) {
        for (auto& path_entry : res.data) {
            for (auto& date_entry : path_entry.second) {
                final_result.data[path_entry.first][date_entry.first] += date_entry.second;
            }
        }
    }

    munmap((void*)file_data, file_size);
    close(fd);

    // Convert to pybind11 dict
    py::dict py_dict;
    
    std::vector<std::string> paths;
    for (const auto& entry : final_result.data) {
        paths.push_back(entry.first);
    }
    std::sort(paths.begin(), paths.end());

    for (const auto& path : paths) {
        py::dict py_dates;
        
        std::vector<std::string> dates;
        for (const auto& entry : final_result.data.at(path)) {
            dates.push_back(entry.first);
        }
        std::sort(dates.begin(), dates.end());

        for (const auto& date : dates) {
            py_dates[py::str(date.c_str())] = py::int_(final_result.data.at(path).at(date));
        }
        
        py_dict[py::str(path.c_str())] = py_dates;
    }

    return py_dict;
}

PYBIND11_MODULE(fast_processor, m) {
    m.doc() = "High-performance file processing module";
    
    m.def("process_file", &process_file, "Process file using C++",
          py::arg("file_path"), py::arg("num_workers"));
}
