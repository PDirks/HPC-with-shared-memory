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

#define DEBUG 0

#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define BROWN   "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define GREY    "\033[0;37m"

#define BRED    "\033[1;31m"
#define BGREEN  "\033[1;32m"
#define BBLUE   "\033[1;34m"
#define BCYAN   "\033[1;36m"
#define BGREY   "\033[1;37m"

typedef struct {
    std::string fname;
    uint32_t row;
    std::vector<float> dat;
} csvRow_t;


typedef struct {
    std::string fname;
    float normal;
} norm_t;


typedef struct {
    uint32_t row;
    float normal;
} norm2_t;

namespace pete{

class util{
    public:
        std::string file;
        std::map<uint32_t, csvRow_t> master ;  

        std::vector<norm2_t> normalized ;

        util( std::string f );
        ~util();
        
        void import();
        void normalize(const uint32_t key);
        void parallel_normalize( const uint32_t key, const uint32_t K , const uint8_t procs );
        csvRow_t getRefKey(const std::string rowName);
};
}; // end namespace pete

std::map<std::string, std::vector<float>> csv_read(const std::string file);
//std::vector<csvRow_t> csv_read(std::string file);

std::vector<norm_t> normalize(std::map<std::string, std::vector<float>> master, const std::string key, uint8_t procs, uint8_t offset, uint32_t K);
std::vector<norm_t> normalize(std::map<std::string, std::vector<float>> master, const std::string key);

bool norm_sort(const norm2_t &a, const norm2_t &b );

void print_norm(const std::vector<norm_t> normalized, const uint32_t K );
