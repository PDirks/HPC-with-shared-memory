/*
 * Peter Dirks
 * cs4080
 * hw2
 * hw2.cpp
 **/

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>      // std::setfill, std::setw
#include <iostream>
#include <map>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

//#include "../include/util.hpp"
#include "../src/util.cpp"

#define AVG 1

/*
 * file input: [string to match against] [file to load] [K] [# procs]

 example command:
 ./pld9bc_hw2_main "intersection/intersection25_rot_000.tif" "../data/8400_HPC.csv" 25 2
*/

int main(int argc, char * argv[]){

/*
 *  ###  Verify User Input  ###
 */
    if( argc != 5 ){
        throw std::logic_error("[input] insufficient arg count\n");
        return 0;
    }
    if( argv[1] == '\0' || argv[2] == '\0' || argv[3] == '\0' || argv[4] == '\0' ){
        throw std::logic_error("bad input\n");
        return 0;
    }
    if( std::stoi(argv[4]) < 1 ){
        throw std::logic_error("[input] not proc count less than 1");
        return 0;
    }

    std::string::size_type sz;   // alias of size_t

    std::string pattern = argv[1];
    std::string file    = argv[2];

    uint32_t K      = std::stoi(argv[3], &sz);
    uint32_t procs  = std::stoi(argv[4], &sz);

    std::chrono::high_resolution_clock c;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point stop;

    pete::util csv(file);
    
/*
 *  ###  Read From File  ###
 */
    double read_avg = 0;
    for( int i = 0; i < AVG; i++){
        start = c.now();
//        csv.import();
        csv.import2();
        stop = c.now();
        read_avg += (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count())/ 1000000;
        #if PROGRESS_FLAG        
        if( i % 1 == 0 ){
            std::cout << "." << std::flush;
        }
        #endif
    }
    read_avg /= AVG;

/*
 *  ###  Calculate & Sort  ###
 */
    double calc_avg = 0;
    for( int i = 0; i < AVG; i++){
        start = c.now();
        csv.parallel_block_normalize( csv.getRefKey2(pattern), K , procs );
        stop = c.now();
        calc_avg += (double) (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()) / 1000000;
        #if PROGRESS_FLAG       
        if( i % 1 == 0 ){
            std::cout << "," << std::flush;
        }
        #endif
    }
    calc_avg /= AVG;

/*
 *  ###  Output  ###
 */
    std::cout << BGREY << "### HW2 REPORT ###" << GREY << std::endl;
    std::cout << "/ " << std::setfill('=') << std::setw(75) << "=" << std::flush;
    std::cout << GREY << "\n/ K: " << BRED << K 
        << GREY << "\n/ procs: " << BRED << unsigned(procs) 
        << GREY << "\n/ fname: " << BRED << file
        << GREY << "\n/ pattern:" << BRED << pattern
        << GREY << "\n/\n/ read avg: " << BRED << read_avg << " sec"
        << GREY << "\n/ calc avg: " << BRED << calc_avg 
        << " sec" << GREY << std::endl;

    std::cout << "/\n/ [index] [filename] [L1-Norm]" << std::endl;
    std::cout << "/ " << std::setfill('-') << std::setw(55) << "-" << std::endl;
    for( uint32_t i = 0; i < K; i++ ){
        if( i < 10 ){
            std::cout << GREY << "/ "
                << " [" << i << "]  | "
                << csv.revBlockIndex[csv.normalized.at(i).row]
                << " | "
                << csv.normalized.at(i).normal
                << GREY << std::endl;
        }
        else{
            std::cout << GREY << "/ "
                << " [" << i << "] | "
                << csv.revBlockIndex[csv.normalized.at(i).row]
                << " | "
                << csv.normalized.at(i).normal
                << GREY << std::endl;
        }
    }
    std::cout << "/ " << std::setfill('-') << std::setw(58) << "-" << std::endl;
    std::cout << "/ " << std::endl;
    std::cout << "/ " << std::setfill('=') << std::setw(75) << "=" << std::endl;

/*
 *  ###  Cleanup  ###
 */

    free( csv.dataBlock );

    return 0;
}// end main
