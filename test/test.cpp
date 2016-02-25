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

const std::string fname = "../data/8400_HPC.csv";
const uint32_t size = 8400;
//const std::string refName = "intersection/intersection25_rot_000.tif";
const std::string refName = "agricultural/agricultural00_rot_000.tif";
const uint32_t ref_line = 4501;
const uint32_t K = 25;
const uint8_t procs = 1;

void sanity_check( const std::vector<norm2_t> normalized );
void sanity_check2( uint8_t *dataBlock );
void sanity_check3( const std::vector<norm2_t> normalized, pete::util csv );

int main (void){

    std::chrono::high_resolution_clock c;
    std::chrono::high_resolution_clock::time_point stop;
    std::chrono::high_resolution_clock::time_point start;

    std::cout << BGREY << "### HW2 tests ###" << GREY << std::endl;
    std::cout << std::setfill('=') << std::setw(35) << "=" << std::endl;
    std::cout << "/ K: " << BRED<< K << GREY
        << "\n/ procs: " << BRED << unsigned(procs) << GREY
        << "\n/ refName: " << BRED << refName << GREY
        << "\n/ fname: " << BRED << fname << GREY << std::endl;
    std::cout << std::setfill('=') << std::setw(35) << "=" << std::endl;

    pete::util csv(fname);
////  ### read test ###

#if SIMPLE
//    sanity_check2(csv.dataBlock);
    std::cout << GREEN << "[TEST] import1 " << GREY << std::flush;
    //std::map<std::string, std::vector<float>> csv_Map = csv_read(fname);
    //assert( csv_Map.size() == size );    
    
    csv.import();
    #if DEBUG
    std::cout << "\n" << CYAN << "[DEBUG] size... " << csv.master.size() << "vs" << size << " " << GREY << std::flush;
    #endif
    assert( csv.master.size() == size );
    std::cout << GREEN << "." << GREY << std::endl;    
#endif

#if !SIMPLE
    std::cout << GREEN << "[TEST] import2 " << GREY << std::flush;
    start = c.now();   
    csv.import2();
    stop = c.now();
    std::cout << GREEN << ". (" 
        << (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000  
        << " sec)"<< GREY << std::endl;    
#endif

//// ### normalize test ###
#if SIMPLE
    std::cout << GREEN << "[TEST] normalize1 " << GREY;

    start = c.now();
    csv.normalize( csv.getRefKey(refName).row );
    stop = c.now();

    assert( csv.normalized.at(0).normal == 0 );
    std::sort( csv.normalized.begin(), csv.normalized.end(), norm_sort );

    std::cout << GREEN << ". (" 
        << (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000 
        << " sec)" << GREY
        << std::endl;
#endif
#if !SIMPLE
//// ### parallel normalize
    std::cout << GREEN << "[TEST] normalize2 " << GREY << std::flush;

    start = c.now();
    csv.parallel_block_normalize( ref_line, K , procs );
    stop = c.now();


    assert( csv.normalized.at(0).normal == 0 );
    //std::sort( csv.normalized.begin(), csv.normalized.end(), norm_sort );

    std::cout << GREEN << ". (" 
        << (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000 
        << " sec)" << GREY
        << std::endl;
#endif
//// ### sanity check ###
    std::cout << GREEN << "[TEST] sanity check " << GREY << std::flush;
//    sanity_check(csv.normalized);
//    sanity_check3(csv.normalized, csv);
    std::cout << GREEN << "." << GREY << std::endl;

//// ### print out normalized ###

    //print_norm( normalized, K );

    return 0;
}

void sanity_check( const std::vector<norm2_t> normalized ){

    float sols[] = { 0, 0.684165, 0.743850, 0.789222, 1.01962, 1.08140, 1.11942, 1.15477, 1.16681, 1.16774 };

    for(int i = 1; i < 10; i++){
        //#if DEBUG
        std::cout << normalized.at(0).normal << ", " << normalized.at(1).normal << ", " << normalized.at(2).normal << std::endl;
        std::cout << "[DEBUG] sanity check: expected "  << sols[i] << " vs reality " << normalized.at(i).normal << std::endl;
        //#endif
        assert( std::abs( normalized.at(i).normal - sols[i] ) < .001 );
    }
}// end sanity_check

void sanity_check2( uint8_t *dataBlock ){

    float sols[] = {0,0,1.36346,0,0,1.72232,0,0,0};
    for(int i = 0; i < 3; i++){
        float tempf = 0;

        memcpy( &tempf, &dataBlock[ (4097 * i) ], sizeof(float) );
        #if DEBUG
        std::cout << "[" << i*3 << "] -> " 
            << tempf
            << " (" << (4097 * i)*4 << ") "
            << std::endl;
        #endif
        assert(std::abs( tempf - sols[i*3] ) < 0.0001);

        memcpy( &tempf, &dataBlock[ (4097 * i)*4 + 4 ], sizeof(float) );
        #if DEBUG
        std::cout << "[" << i*3+1 << "] -> " 
            << tempf
            << " (" << (4097 * i)*4 + 4 << ") "
            << std::endl;
        #endif
        assert(std::abs( tempf - sols[i*3+1]) < 0.0001);

        memcpy( &tempf, &dataBlock[ (4097 * i)*4 + 8 ], sizeof(float) );
        #if DEBUG
        std::cout << "[" << i*3+2 << "] -> " 
            << tempf
            << " (" << (4097 * i)*4 + 8 << ") "
            << std::endl;
        #endif
        assert(std::abs( tempf - sols[i*3+2]) < 0.0001);
    }

}// end sanity_check2

void sanity_check3( const std::vector<norm2_t> normalized, pete::util csv ){
//    float sols[] = { 0, 0.684165, 0.743850, 0.789222, 1.01962, 1.08140, 1.11942, 1.15477, 1.16681, 1.16774 };

    for(uint32_t i = 0; i < K; i++){
        std::cout << "[" << csv.revBlockIndex[i] << "] -> "<<normalized.at(i).normal << " (" << i << ")" << std::endl;
    }

    std::string sols[] = {  "intersection/intersection25_rot_000.tif", "intersection/intersection25_rot_090.tif",
                            "intersection/intersection25_rot_270.tif", "intersection/intersection25_rot_180.tif",
                            "intersection/intersection30_rot_000.tif", "intersection/intersection30_rot_180.tif",
                            "intersection/intersection30_rot_090.tif", "intersection/intersection20_rot_180.tif",
                            "intersection/intersection10_rot_180.tif", "intersection/intersection05_rot_090.tif" };

    for(int i = 0; i < 9; i++){
        //#if DEBUG
/*        std::cout << "\t" << csv.revBlockIndex[normalized.at(0).row] << ", " 
            << csv.revBlockIndex[normalized.at(1).row] << ", " 
            << csv.revBlockIndex[normalized.at(2).row] << std::endl;*/

        std::cout << "\t" << csv.revBlockIndex[normalized.at(0).row] << ", " 
            << csv.revBlockIndex[normalized.at(1).row]
            << std::endl;

        std::cout << "[DEBUG] sanity check: expected "  << sols[i] << " vs reality " << csv.revBlockIndex[normalized.at(i).row] << std::endl;
        //#endif
        assert( sols[i].compare( csv.revBlockIndex[ normalized.at(i).row ] ) == 0 );
    }


}
