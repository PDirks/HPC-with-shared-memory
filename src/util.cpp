/*
 * Peter Dirks
 * cs4080
 * hw2
 * util.cpp
 **/

#include <cmath>        // std::abs
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../include/util.hpp"

#define DEBUG 1

/*
 * Read from csv into map of csvRow_ts
 */
std::map<std::string, std::vector<float>> csv_read(const std::string file){
    char *toks;
    std::string temp_fname;
    std::string line;
    std::map<std::string, std::vector<float>> csvMap;
    std::ifstream fin ( file, std::ifstream::in );
    if(fin.is_open()){
        while( getline(fin, line) ){    // iterate through line tokenizing floats, add to map
            temp_fname = strtok(const_cast<char*>(line.c_str()), ",");
            toks = strtok(NULL, ",");
            while( toks != NULL ){
//                csvVector[temp_row].push_back( std::strtof(toks,NULL) );
                csvMap[temp_fname].push_back( std::strtof(toks,NULL) );
                   
                toks = strtok(NULL, ",");
            }
        }
        fin.close();                
    }// end file read
    return csvMap;
}// end csv_read

std::vector<norm_t> normalize(std::map<std::string, std::vector<float>> master, const std::string key){
    std::vector<float> reference_vector = master.at(key);
    std::vector<norm_t> normalized;
    const uint32_t size = master.size()-1;

    for( std::map<std::string, std::vector<float>>::iterator it = master.begin(); it != master.end(); ++it ){
        // TODO: unroll loop
        float normal_sum = 0;
        for( int i = 0; i < 4096; i++ ){
            normal_sum += std::abs(it->second[i] - reference_vector[i]) / size;
        }// end vector for
//        normalized.insert(std::pair<std::string, float>(it->first, normal_sum));
        
        norm_t temp_norm = {it->first, normal_sum};
        normalized.push_back(temp_norm);
   }// end iterator for

    return normalized;
}// normalize

bool norm_sort(const norm_t &a, const norm_t &b ){
    return a.normal < b.normal;
}// end norm_sort

void print_norm(const std::vector<norm_t> normalized, const uint32_t K ){
    for( uint32_t i = 1; i < K+1; i++ ){
        std::cout << "\t[" << i << "] "
            << normalized.at(i).fname << " -> "
            << normalized.at(i).normal << std::endl;
    }
}

