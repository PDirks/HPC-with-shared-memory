/*
 * Peter Dirks
 * cs4080
 * hw2
 * test.cpp
 **/
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#include "../src/util.cpp"

#define assert(e) if((e) != true){ \
                   fprintf(stderr,"%s,%d: assertion '%s' failed\n",__FILE__, __LINE__, #e); \
                   fflush(stderr); fflush(stdout); abort();}

const std::string fname = "../data/2100_HPC.csv";
const uint32_t size = 2100;
const std::string refName = "agricultural/agricultural00.tif";
const uint32_t K = 25;

int main (void){

// read test
    std::cout << "read test ";
    std::map<std::string, std::vector<float>> csv_Map = csv_read(fname);
    assert( csv_Map.size() == size );
    std::cout << ".\n";
    
// normalize test
    std::cout << "normalize test";
    std::vector<norm_t> normalized = normalize( csv_Map, refName );

    assert( normalized.at(0).normal == 0 );
    std::sort( normalized.begin(), normalized.end(), norm_sort );

    std::cout << ".\n";

// print out normalized

    print_norm( normalized, K );

    return 0;
}
