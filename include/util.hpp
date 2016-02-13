/*
 * Peter Dirks
 * cs4080
 * hw2
 * util.hpp
 **/
#include <map>
#include <string>
#include <set>
#include <vector>

typedef struct {
    std::string fname;
    std::vector<float> dat;
} csvRow_t;

typedef struct {
    std::string fname;
    float normal;
} norm_t;

std::map<std::string, std::vector<float>> csv_read(std::string file);
//std::vector<csvRow_t> csv_read(std::string file);

std::vector<norm_t> normalize( std::map<std::string, std::vector<float>> master, std::string key);

bool norm_sort(const norm_t &a, const norm_t &b );

void print_norm(const std::vector<norm_t> normalized, const uint32_t K );
