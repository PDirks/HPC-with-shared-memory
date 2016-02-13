/*
 * Peter Dirks
 * cs4080
 * hw2
 * hw2.cpp
 **/

#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

//#include "../include/util.hpp"
#include "../src/util.cpp"

int main(int argc, char * argv[]){

/*
 *  ###  Verify User Input  ###
 */

// file input: [string to match against] [file to load] [K] [# procs]

    if( argc != 5 ){
        throw std::logic_error("[input] insufficient arg count\n");
        return 0;
    }
    if( argv[1] == '\0' || argv[2] == '\0' || argv[3] == '\0' || argv[4] == '\0' ){
        throw std::logic_error("bad input\n");
    }
    std::string::size_type sz;   // alias of size_t

    std::string pattern = argv[1];
    std::string file    = argv[2];

//    uint32_t K      = std::stoi(argv[3], &sz);
//    uint32_t procs  = std::stoi(argv[4], &sz);


/*
 *  ###  Read From File  ###
 */
//    std::vector<csvRow_t> csv_Vector = csv_read(file);
    std::map<std::string, std::vector<float>> csvMap = csv_read(file);
    std::cout << "read done\n";

/*
 *  ###  Calculate & Sort  ###
 */

/*
 *  ###  Output  ###
 */


/*
 *  ###  Cleanup  ###
 */
    /*
    for(int i = csv.size-1; i != 0; i-- ){
        delete csv[i];
    }
    delete csv;
    */

    std::cout << "\tdone!\n";
    return 0;
}// end main
