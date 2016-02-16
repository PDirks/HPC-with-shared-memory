/*
 * Peter Dirks
 * cs4080
 * hw2
 * test.cpp
 **/
#include <algorithm>
#include <chrono>
#include <iomanip>      // std::setfill, std::setw
#include <map>
#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#include <unistd.h>     // fork

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "../src/util.cpp"

#define assert(e) if((e) != true){ \
                   fprintf(stderr,"%s,%d: assertion '%s' failed\n",__FILE__, __LINE__, #e); \
                   fflush(stderr); fflush(stdout); abort();}


const std::string fname = "../data/6300_HPC.csv";
const uint32_t size = 6300;
const std::string refName = "agricultural/agricultural00.tif";
const uint32_t K = 25;
const uint8_t procs = 1;

void sanity_check( const std::vector<norm2_t> normalized );

int main (void){

    std::cout << BGREY << "### HW2 tests ###" << GREY << std::endl;
    std::cout << std::setfill('=') << std::setw(35) << "=" << std::endl;
    std::cout << "/ K: " << K 
        << "\n/ procs: " << unsigned(procs) 
        << "\n/ fname: " << fname << std::endl;
    std::cout << std::setfill('=') << std::setw(35) << "=" << std::endl;

////  ### read test ###
    std::cout << GREEN << "[TEST] read " << GREY << std::flush;
    //std::map<std::string, std::vector<float>> csv_Map = csv_read(fname);
    //assert( csv_Map.size() == size );    
    pete::util csv(fname);
    csv.import();
    #if DEBUG
    std::cout << "\n" << CYAN << "[DEBUG] size... " << csv.master.size() << "vs" << size << " " << GREY << std::flush;
    #endif
    assert( csv.master.size() == size );
    std::cout << GREEN << "." << GREY << std::endl;    

//// ### normalize test ###
#if 0
    std::cout << GREEN << "[TEST] normalize " << GREY;

    std::chrono::high_resolution_clock c;
    std::chrono::high_resolution_clock::time_point start = c.now();

    csv.normalize( csv.getRefKey(refName).row );

    std::chrono::high_resolution_clock::time_point stop = c.now();

    assert( csv.normalized.at(0).normal == 0 );
    std::sort( csv.normalized.begin(), csv.normalized.end(), norm_sort );

    std::cout << GREEN << ". (" 
        << (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000 
        << " sec)" << GREY
        << std::endl;
#endif
//// ### parallel normalize
    std::cout << GREEN << "[TEST] normalize " << GREY << std::flush;

    std::chrono::high_resolution_clock c;
    std::chrono::high_resolution_clock::time_point start = c.now();

    //start = c.now();
    csv.parallel_normalize( csv.getRefKey(refName).row, K , procs );
    //stop = c.now();

    std::chrono::high_resolution_clock::time_point stop = c.now();

    assert( csv.normalized.at(0).normal == 0 );
    //std::sort( csv.normalized.begin(), csv.normalized.end(), norm_sort );

    std::cout << GREEN << ". (" 
        << (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000 
        << " sec)" << GREY
        << std::endl;

//// ### sanity check ###
    std::cout << GREEN << "[TEST] sanity check " << GREY << std::flush;
    sanity_check(csv.normalized);
    std::cout << GREEN << "." << GREY << std::endl;

//// ### print out normalized ###

    //print_norm( normalized, K );

    return 0;
}

void sanity_check( const std::vector<norm2_t> normalized ){

    float sols[] = { 0, 0.884873, 1.09125, 1.0933, 1.31354, 1.32358, 1.36153, 1.4418, 1.46398, 1.48857, 1.5071,
        1.52222, 1.53879, 1.57182, 1.5952, 1.64235, 1.64909, 1.64935, 1.71369, 1.71775, 1.73443,
        1.74639, 1.78687, 1.80131, 1.80849, 1.82537 };

    for(int i = 1; i < 25; i ++){
        #if DEBUG
        //std::cout << normalized.at(0).normal << ", " << normalized.at(1).normal << ", " << normalized.at(2).normal << std::endl;
        std::cout << "[DEBUG] sanity check: " << normalized.at(i).normal << " vs " << sols[i] << std::endl;
        #endif
        assert( std::abs( normalized.at(i).normal - sols[i] ) < 0.0001 );
    }

}

