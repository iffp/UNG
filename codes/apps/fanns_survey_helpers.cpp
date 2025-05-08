#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


std::vector<std::vector<float>> read_fvecs(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Unable to open file for reading.\n";
        return {};
    }
    std::vector<std::vector<float>> dataset;
    while (file) {
        int d;
        if (!file.read(reinterpret_cast<char*>(&d), sizeof(int))) break;  // Read dimension
        std::vector<float> vec(d);
        if (!file.read(reinterpret_cast<char*>(vec.data()), d * sizeof(float))) break;  // Read vector data
        dataset.push_back(std::move(vec));
    }
    file.close();
    return dataset;
}

std::vector<std::vector<int>> read_ivecs(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Unable to open file for reading.\n";
        return {};
    }
    std::vector<std::vector<int>> dataset;
    while (file) {
        int d;
        if (!file.read(reinterpret_cast<char*>(&d), sizeof(int))) break;  // Read dimension
        std::vector<int> vec(d);
        if (!file.read(reinterpret_cast<char*>(vec.data()), d * sizeof(int))) break;  // Read vector data
        dataset.push_back(move(vec));
    }
    file.close();
    return dataset;
}

std::vector<int> read_one_int_per_line(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    std::vector<int> result;
    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        ++line_number;
        std::stringstream ss(line);
        int value;
        if (!(ss >> value)) {
            throw std::runtime_error("Non-integer or empty line at line " + std::to_string(line_number));
        }
        std::string extra;
        if (ss >> extra) {
            throw std::runtime_error("More than one value on line " + std::to_string(line_number));
        }
        result.push_back(value);
    }
    return result;
}

std::vector<std::vector<int>> read_multiple_ints_per_line(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    std::vector<std::vector<int>> data;
    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        ++line_number;
        std::vector<int> row;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            try {
                if (!token.empty()) {
                    row.push_back(std::stoi(token));
                }
            } catch (...) {
                throw std::runtime_error("Invalid integer on line " + std::to_string(line_number));
            }
        }
        data.push_back(std::move(row));
    }
    return data;
}

std::vector<std::pair<int, int>> read_two_ints_per_line(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    std::vector<std::pair<int, int>> result;
    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        ++line_number;
        std::stringstream ss(line);
        std::string first, second;
        if (!std::getline(ss, first, '-') || !std::getline(ss, second) || !ss.eof()) {
            throw std::runtime_error("Invalid format at line " + std::to_string(line_number));
        }
        try {
            int a = std::stoi(first);
            int b = std::stoi(second);
            result.emplace_back(a, b);
        } catch (...) {
            throw std::runtime_error("Invalid integer value at line " + std::to_string(line_number));
        }
    }
    return result;
}

void peak_memory_footprint()
{

    unsigned iPid = (unsigned)getpid();

    std::cout << "PID: " << iPid << std::endl;

    std::string status_file = "/proc/" + std::to_string(iPid) + "/status";
    std::ifstream info(status_file);
    if (!info.is_open())
    {
        std::cout << "memory information open error!" << std::endl;
    }
    std::string tmp;
    while (getline(info, tmp))
    {
        if (tmp.find("Name:") != std::string::npos || tmp.find("VmPeak:") != std::string::npos || tmp.find("VmHWM:") != std::string::npos)
            std::cout << tmp << std::endl;
    }
    info.close();
}

