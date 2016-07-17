/*
 * Peter Dirks
 * cs4080
 * hw2
 * util.cpp
 **/

#include <algorithm>    // std::sort, std::partial_sort
#include <cmath>        // std::abs
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <unistd.h>  // fork, read, open, close

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "../include/util.hpp"

#define UTIL_DEBUG 1

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
        
        norm_t temp_norm = {it->first, normal_sum};
        normalized.push_back(temp_norm);
   }// end iterator for
    return normalized;
}// normalize



bool norm_sort(const norm2_t &a, const norm2_t &b ){
    return a.normal < b.normal;
}// end norm_sort

void print_norm(const std::vector<norm_t> normalized, const uint32_t K ){
    for( uint32_t i = 1; i < K+1; i++ ){
        std::cout << "\t[" << i << "] "
            << normalized.at(i).fname << " -> "
            << normalized.at(i).normal << std::endl;
    }
}

pete::util::util( std::string f ){ 
    file = f; 
    rawFloatCount = 0; 
}
pete::util::~util(){
}

/*
 * import with getline! It's not fast! Boo!
 **/
void pete::util::import(){
    char *toks;
    std::string temp_fname, line;
    std::ifstream fin   ( file, std::ifstream::in );
    uint32_t cnt = 0;
    if(fin.is_open()){
        while( getline(fin, line) ){    // iterate through line tokenizing floats, add to map
            temp_fname = strtok(const_cast<char*>(line.c_str()), ",");
            toks = strtok(NULL, ",");
            while( toks != NULL ){                
                master[cnt].dat.push_back(std::strtof(toks,NULL));
                toks = strtok(NULL, ",");
            }
            master[cnt].row = cnt;
            master[cnt].fname = temp_fname;
            cnt++;
        }
        fin.close();                
    }// end file read
    else{
        #if DEBUG
        std::cerr << BRED << "[DEBUG] error on file open" << GREY << std::endl;
        #endif
    }
}// end csv_read

/*
 * import with read()! It's fast! Yay!
 **/
void pete::util::import2(){

    std::ifstream in( file.c_str(), std::ifstream::ate | std::ifstream::binary );
    size_t filesize = in.tellg();

#if MMAP
    int fd = open( file.c_str(), O_RDONLY, 0);  // open file
    assert( fd != -1 );
#endif
    //Execute mmap

/*  flags used for mmap...
 *      PROT_READ   - read-only page protection
 *      MAP_PRIVATE - don't write changes to the file (redundant as file was open as O_RDONLY)
 *      MAP_POPULATE- kernel will repopulate file
 **/
#if MMAP
    mmappedData = (uint8_t *)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    assert(mmappedData != MAP_FAILED);
#else
    int filedesc = open(file.c_str(), O_RDONLY, 0);
    if( filedesc < 0 ){
        std::cerr << BRED << "[DEBUG] error on file open" << GREY << std::endl;
    }
    uint8_t *rawBlock = (uint8_t *)calloc( sizeof(uint8_t), filesize );
    if( !rawBlock ){
        std::cerr << BRED << "[DEBUG] error on calloc" << GREY << std::endl;
    }
    if(read( filedesc, rawBlock, filesize) < 0 ){
        std::cerr << BRED << "[DEBUG] error on file read" << GREY << std::endl;
    }
#endif
    dataBlock = (uint8_t *)calloc( filesize*2, sizeof(uint8_t) );

    uint32_t start = 0;
    uint32_t end = 0;
    row_count = 0;
    std::string rowRaw;
    uint8_t rowStart = 1;
    uint32_t dataBlock_offset = 0;
    float temp = 0;
    while( end < filesize ){
#if MMAP
        if( mmappedData[end] == ',' ){
#else
        if( rawBlock[end] == ',' ){
#endif
            if( rowStart == 1 ){

#if MMAP
                rowRaw.assign((char *)&mmappedData[start], end-start);              
#else
                rowRaw.assign( (char *)&rawBlock[start], end-start );
#endif

                blockIndex[rowRaw] = row_count;
                revBlockIndex.push_back(rowRaw);
                rowStart = 0;
                //end++;
            }
            else if( rowStart == 0 ){
#if MMAP
                rowRaw.assign( (char *)mmappedData+start, end-start );
#else
                rowRaw.assign( (char *)&rawBlock[start], end-start );
#endif
    

                temp = atof(rowRaw.c_str());
                memcpy( &dataBlock[dataBlock_offset], &temp, sizeof(float) );
                rawFloatCount++;
                #if DEBUG
                if( row_count+1 == 4501 )
                std::cout << " adding " << temp << 
                    " to offset " << dataBlock_offset/sizeof(float) << 
                    " (" << (dataBlock_offset % (4097*sizeof(float)))/sizeof(float) <<
                    ")" << std::endl;
                #endif
                dataBlock_offset += sizeof(float);
            }
            start = ++end;
        }
#if MMAP
        else if( mmappedData[end] == '\n' ){
            rowRaw.assign( (char *)mmappedData+start, end-start );
#else
        else if( rawBlock[end] == '\n' ){
            rowRaw.assign( (char *)&rawBlock[start], end-start );
#endif

            temp = atof(rowRaw.c_str());
            memcpy( &dataBlock[dataBlock_offset], &temp, sizeof(float) );
            row_count++;
            start = ++end;
            rawFloatCount++;
            #if DEBUG
            //std::cout << " adding " << temp << " to offset " << dataBlock_offset << std::endl;
            #endif

            dataBlock_offset += sizeof(float);
            rowStart = 1;
        }
        else{
            end++;
        }
    }

//    std::string rowRaw( (char *)mmappedData, 100 );
//    std::cout << "[0]" << rowRaw << std::endl;
    
    //Cleanup

#if MMAP
    int rc = munmap(mmappedData, filesize);
    assert(rc == 0);
#else
    if( close(filedesc) < 0 ){}
    free(rawBlock);
#endif

}// end import2

void pete::util::normalize(const uint32_t key){
    csvRow_t reference_vector = master.at(key);
    const uint32_t size = master.size()-1;

    for( std::map<uint32_t, csvRow_t>::iterator it = master.begin(); it != master.end(); ++it ){
        // TODO: unroll loop
        float normal_sum = 0;
        for( int i = 0; i < 4096; i++ ){
            normal_sum += std::abs(it->second.dat[i] - reference_vector.dat[i]) / size;
        }// end vector for
        
        norm2_t temp_norm = {it->first, normal_sum};
        normalized.push_back(temp_norm);
    }// end iterator for
}// normalize

/*
 *  Parallel Normalize with a string-float map! Slow! Boo!
 **/
void pete::util::parallel_normalize( const uint32_t key, const uint32_t K , const uint8_t procs ){
    const csvRow_t reference_vector = master.at(key);

//// ### shm setup ###
    int shmId; 			// ID of shaBRED memory segment
	key_t shmKey = 222260; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag = IPC_CREAT | 0666; // Flag to create with rw permissions
	
	// This will be shaBRED:
	norm2_t * shm;
    std::vector<norm2_t> return_norms;

    // shmget() returns the identifier of the shaBRED memory segment associated with the value of the argument key.
    #if DEBUG
    std::cout << CYAN << "[DEBUG] mem size: " << sizeof(norm2_t) * K * procs << " bytes" << GREY << std::endl;
    #endif
	if ((shmId = shmget(shmKey, sizeof(norm2_t) * K * procs, shmFlag)) < 0){ // TODO look at shm sizing
        #if DEBUG
		std::cerr << BRED <<"[DEBUG] Init: Failed to initialize shaBRED memory (" << shmId << ")" << GREY << std::endl; 
        #endif
		exit(1);
	}
    #if DEBUG
    std::cout << CYAN << "[DEBUG] shmId " << shmId << GREY << std::endl;
    #endif
    // shmat() attaches the shaBRED memory segment identified by shmid to the address space of the calling process.
    if ((shm = (norm2_t *)shmat(shmId, NULL, 0)) == (norm2_t *) -1){
        #if DEBUG
		std::cerr << BRED << "[DEBUG] Init: Failed to attach shaBRED memory (" << shmId << ")" << GREY << std::endl; 
        #endif
		exit(1);
	}
    
//// ### fork test ###
    pid_t pid;
    uint8_t proc_id;
    for( uint8_t i = 0; i < procs; i++){
        //std::cout << "[DEBUG] spinning up proc " << std::endl;
        pid = fork();
        if( pid == 0 ){
            proc_id = i;
            break;
        }
    }
    if( pid < 0 ){
//        #if DEBUG
        std::cerr << BRED << "[DEBUG] Could not fork!!! ("<< pid <<")" << GREY << std::endl;
//        #endif
		exit(1);
    }

// ### CHILD PROC ###
    if( pid == 0 ){
//        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] starting child proc " << unsigned(proc_id) << ", " << GREY << std::endl;
//        #endif

        // compute local normal
        const uint32_t size = master.size()-1;
        norm2_t local_norm[ (proc_id + 1) * (size/procs) + 1 ];
        uint32_t local_index = 0;

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] proc " << unsigned(proc_id) << " computing local normal" << "(" << (proc_id + 1) * (size/procs) + 1 << ")" << GREY << std::endl;
        #endif

        std::map<uint32_t, csvRow_t>::iterator it = master.begin();
        std::advance(it, (proc_id) * (size/procs));
        for( ; it != master.end(); ++it ){
            if( local_index == (size/procs) ){
                break;
            }
            // TODO: unroll loop
            float normal_sum = 0;
            for( int i = 0; i < 4096; i++ ){
                normal_sum += std::abs(it->second.dat[i] - reference_vector.dat[i]) / size;
            }// end vector for
            
            norm2_t temp_norm = {it->first, normal_sum};
            local_norm[local_index] = temp_norm;

            local_index++;
        }// end iterator for

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] proc " << unsigned(proc_id) << " computing done" << GREY << std::endl;
        #endif

//        std::sort( &local_norm[0], &local_norm[local_index-1], norm_sort );
        std::partial_sort( &local_norm[0], &local_norm[0]+K, &local_norm[local_index-1], norm_sort );

        // jump to correct block in shared memory
        shm += proc_id * K;    

        // store local_norm into shaBRED mem
        std::copy( local_norm, local_norm+K, shm);    //TODO memcpy or copy??

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] ending child proc " << unsigned(proc_id) << GREY << std::endl;
        #endif
		_exit(0);
    }// END CHILD
// ### PARENT PROC ###
    #if BOILER_PLATE_JOIN
    int status;	// catch the status of the child
    #endif

    while (true) {
        int status;
        pid_t done = wait(&status);
        if (done == -1) {
            if (errno == ECHILD) break; // no more child processes
        } else {
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                std::cerr << "pid " << done << " failed" << std::endl;
                exit(1);
            }
        }
    }

// read normalized vector out of shared memory
    normalized.assign( shm, shm+(K*procs) );
//    std::sort( normalized.begin(), normalized.end(), norm_sort );
    std::partial_sort( normalized.begin(), normalized.begin()+K, normalized.end(), norm_sort );

// remove shm!
    shmctl(shmId,IPC_RMID,0);

}// end parallel_normalize

/**************************************************************************
 *  parallel_block_normalize()
 *      Parallel Block Normalize! Normalize with a big data block! 
 *      Fast! Yay!
 **************************************************************************/
void pete::util::parallel_block_normalize( const uint32_t key, const uint32_t K , const uint8_t procs ){
//    const csvRow_t reference_vector = master.at(key); // old
    float *reference_block = (float *)calloc( sizeof(float), 4097 );
    memcpy( reference_block, dataBlock + (key-1) * (ROWSIZE), sizeof(float) * 4097 );
//    std::cout << " REF LINE: " << key << std::endl;

//// ### shm setup ###
    int shmId; 			// ID of shaBRED memory segment
	key_t shmKey = 222260; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag = IPC_CREAT | 0666; // Flag to create with rw permissions
	
	// This will be shared:
	norm2_t * shm;
    std::vector<norm2_t> return_norms;

    // shmget() returns the identifier of the shaBRED memory segment associated with the value of the argument key.
    #if DEBUG
    std::cout << CYAN << "[DEBUG] mem size: " << sizeof(norm2_t) * K * procs << " bytes" << GREY << std::endl;
    #endif
	if ((shmId = shmget(shmKey, sizeof(norm2_t) * K * procs, shmFlag)) < 0){ // TODO look at shm sizing
        #if DEBUG
		std::cerr << BRED <<"[DEBUG] Init: Failed to initialize shared memory (" << shmId << ")" << GREY << std::endl; 
        #endif
		exit(1);
	}
    #if DEBUG
    std::cout << CYAN << "[DEBUG] shmId " << shmId << GREY << std::endl;
    #endif
    // shmat() attaches the shaBRED memory segment identified by shmid to the address space of the calling process.
    if ((shm = (norm2_t *)shmat(shmId, NULL, 0)) == (norm2_t *) -1){
        #if DEBUG
		std::cerr << BRED << "[DEBUG] Init: Failed to attach shared memory (" << shmId << ")" << GREY << std::endl; 
        #endif
		exit(1);
	}
    
//// ### fork test ###
    pid_t pid;
    uint8_t proc_id;
//    std::vector<pid_t> pids;
    for( uint8_t i = 0; i < procs; i++){
        //std::cout << "[DEBUG] spinning up proc " << std::endl;
        pid = fork();
//        pids.push_back(pid);
        if( pid == 0 ){
            proc_id = i;
            break;
        }
    }
    if( pid < 0 ){
        #if DEBUG
        std::cerr << BRED << "[DEBUG] Could not fork!!! ("<< pid <<")" << GREY << std::endl;
        #endif
		exit(1);
    }

// ### CHILD PROC ###
    if( pid == 0 ){
        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] starting child proc " << unsigned(proc_id) << GREY << std::endl;
        #endif

        // compute local normal
        const uint32_t size = rawFloatCount;
        norm2_t *local_norm = (norm2_t *)calloc( size/procs, sizeof(norm2_t) );
        uint32_t local_index = 0;
        float normal_temp = 0;
        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] proc " << unsigned(proc_id) << " computing local normal" << "(" << (proc_id + 1) * (size/procs) + 1 << ")" << GREY << std::endl;
        #endif

        for( uint32_t row = proc_id * (row_count/procs); row < (proc_id + 1) * (row_count/procs); row++ ){// end row loop
            float normal_sum = 0;
            for( uint32_t j = 0; j < 4097; j++){        // TODO unroll

                memcpy( &normal_temp, dataBlock + (row * 4097) * sizeof(float) + (j * sizeof(float)) , sizeof(float));
                normal_sum += std::abs( reference_block[j] - normal_temp );
            
/*                if( local_index+1 == 4502)
                std::cout << "\t ["<< row << "~" << j <<"] "<< normal_sum <<" += ref " 
                    << reference_block[j] << " - active " 
                    << normal_temp << " (" 
                    << (row * 4097) * sizeof(float) + (j * sizeof(float)) << ") " 
                    << std::endl;*/
            }

            norm2_t temp_norm = {row, normal_sum/4984};
            
//            if( temp_norm.row == 4501 )
//            std::cout << "...normalizing [" << temp_norm.row << "] -> " << temp_norm.normal << std::endl;

            //std::cout << row << " " << std::endl;

            local_norm[local_index++] = temp_norm;
        }// end row loop
        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] proc " << unsigned(proc_id) << " computing done" << GREY << std::endl;
        #endif

        std::sort( &local_norm[0], &local_norm[local_index-1], norm_sort );

//        std::cout << "...normalizing " << local_norm[0].normal << ", " << local_norm[1].normal << ", " << local_norm[2].normal << std::endl;

        shm += proc_id * K;    // jump to correct block in shared memory

        std::copy( local_norm, local_norm+K, shm);  // store local_norm into shaBRED mem

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] ending child proc " << unsigned(proc_id) << GREY << std::endl;
        #endif

        // Child Cleanup
        free( local_norm );
		_exit(0);
    }// END CHILD
// ### PARENT PROC ###
    #if BOILER_PLATE_JOIN
    int status;	// catch the status of the child
    #endif

    while (true){
        siginfo_t exit_info;
        int retVal = waitid(P_ALL, -1, &exit_info, WEXITED);
        #if BOILER_PLATE_JOIN
        do { 
	        pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
	        if (w == -1){
                #if DEBUG
		        std::cerr << BRED << "[DEBUG] Error waiting for child process ("<< pid <<")" << GREY << std::endl;
                #endif
		        break;
	        }
	        if (WIFEXITED(status)){
		        if (status > 0){
                    #if DEBUG
			        std::cerr << CYAN << "[DEBUG] Child process ("<< pid <<") exited with non-zero status of " << WEXITSTATUS(status) << GREY << std::endl;
                    #endif
			        continue;
		        }
		        else{
                    #if DEBUG
			        std::cout << CYAN << "[DEBUG] Child process ("<< pid <<") exited with status of " << WEXITSTATUS(status) << GREY << std::endl;
                    #endif
			        continue;
		        }
	        }
	        else if (WIFSIGNALED(status)){
                #if DEBUG
		        std::cout << CYAN << "[DEBUG] Child process ("<< pid <<") killed by signal (" << WTERMSIG(status) << ")" << GREY << std::endl;
                #endif
		        continue;			
	        }
	        else if (WIFSTOPPED(status)){
                #if DEBUG
		        std::cout << CYAN << "[DEBUG] Child process ("<< pid <<") stopped by signal (" << WSTOPSIG(status) << ")" << GREY << std::endl;
                #endif
		        continue;			
	        }
	        else if (WIFCONTINUED(status)){
                #if DEBUG
		        std::cout << CYAN << "[DEBUG] Child process ("<< pid <<") continued" << GREY << std::endl;
                #endif
		        continue;
	        }
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status) );
        #endif
        if (retVal == -1 && errno == ECHILD){
            #if DEBUG
            std::cout << MAGENTA << "[DEBUG] all children done " << GREY << std::endl;
            #endif
            break;
        }
    }

// read normalized vector out of shared memory
    normalized.assign( shm, shm+(K*procs) );
    std::sort( normalized.begin(), normalized.end(), norm_sort );
//    std::cout << "...normalizing " << normalized[0].normal << ", " << normalized[1].normal << ", " << normalized[2].normal << std::endl;

/*
 * Cleanup
 */
    shmctl(shmId,IPC_RMID,0);
    free( reference_block );

}// end parallel_normalize


/*
 * getRefKey - get row object from row id
 **/
csvRow_t pete::util::getRefKey(const std::string rowName){  // TODO fix for block read
    csvRow_t ref;

    uint32_t i = 0;
    for( ; i < master.size(); ){
        if( master[i].fname.compare( rowName ) == 0 ){
            ref = master[i];
            break;
        }
        else i++;
    }
    if( master[i].fname.compare( rowName ) != 0 ){
        std::cout << BRED << "[ERROR] could not find key: " << rowName << GREY << std::endl;
        throw std::logic_error("[input] bad key\n");
    }

    return ref;
}// end getRefKey

uint32_t pete::util::getRefKey2(const std::string rowName){  // TODO fix for block read

    return blockIndex.at( rowName );

}
