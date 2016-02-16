/*
 * Peter Dirks
 * cs4080
 * hw2
 * util.cpp
 **/

#include <algorithm>    // std::sort
#include <cmath>        // std::abs
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <unistd.h>  // fork

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

#if 0
std::vector<norm_t> normalize(std::map<std::string, std::vector<float>> master, const std::string key, uint8_t procs, uint8_t offset, uint32_t K){
    
    // TODO find size function
    uint32_t size = 2100;

//// ### semaphore setup ###
    int semId; 			        // ID of semaphore set
	key_t semKey = 123459; 		// key to pass to semget(), key_t is an IPC key type defined in sys/types
	int semFlag = IPC_CREAT | 0666; // Flag to create with rw permissions

	int semCount = 1;    		// number of semaphores to pass to semget()
	int numOps = 1; 	    	// number of operations to do
	// Create the semaphore
	// The return value is the semaphore set identifier
	// The flag is a or'd values of CREATE and ugo permission of RW, 
	//			just like used for creating a file
   	if ((semId = semget(semKey, semCount, semFlag)) == -1){
   		std::cerr << "Failed to semget(" << semKey << "," << semCount << "," << semFlag << ")" << std::endl;
		exit(1);
	}
	else{
   		std::cout << "Successful semget resulted in (" << semId << std::endl;
	}
	// Initialize the semaphore
	union semun {
		int val;
		struct semid_ds *buf;
		ushort * array;
	} argument;

	argument.val = 1; // NOTE: We are setting this to one to make it a MUTEX
	if( semctl(semId, 0, SETVAL, argument) < 0){
		std::cerr << "Init: Failed to initialize (" << semId << ")" << std::endl; 
		exit(1);
	}
	else{
		std::cout << "Init: Initialized (" << semId << ")" << std::endl; 
	}

//// ### shm setup ###
    int shmId; 			// ID of shaBRED memory segment
	key_t shmKey = 222260; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag = IPC_CREAT | 0666; // Flag to create with rw permissions
	
	// This will be shaBRED:
	unsigned long * shm;
//    std::vector<norm_t> return_norms[procs];

    // shmget() returns the identifier of the shaBRED memory segment associated with the value of the argument key.
	// A new shaBRED memory segment, with size equal to the value of size rounded up to a multiple of PAGE_SIZE, is
	// created  if  key has the value IPC_PRIVATE or key isn't IPC_PRIVATE, no shaBRED memory segment corresponding
	// to key exists, and IPC_CREAT is specified in shmflg.
    std::cout << "mem size: " << K * procs * (sizeof(long) + 40) << " bytes" << std::endl;
	if ((shmId = shmget(shmKey, K * procs * (sizeof(long) + 40), shmFlag)) < 0){ // TODO look at shm sizing
		std::cerr << "Init: Failed to initialize shaBRED memory (" << shmId << ")" << std::endl; 
		exit(1);
	}
    std::cout << "shmId " << shmId << std::endl;
    // shmat() attaches the shaBRED memory segment identified by shmid to the address space of the calling process.
    if ((shm = (unsigned long *)shmat(shmId, NULL, 0)) == (unsigned long *) -1){
		std::cerr << "Init: Failed to attach shaBRED memory (" << shmId << ")" << std::endl; 
		exit(1);
	}
    
//// ### fork test ###
    pid_t pid;
    uint8_t proc_id;
    for( uint8_t i = 0; i < procs; i++){
        //std::cout << "[DEBUG] spinning up proc " << std::endl;
        pid = fork();
        if( pid == 0 ){
            proc_id = i - 1;
            break;
        }
    }
    if( pid < 0 ){
        std::cerr << "Could not fork!!! ("<< pid <<")" << std::endl;
		exit(1);
    }

    if( pid == 0 ){ // CHILD PROC
		struct sembuf operations[1];    // init sembuf
		operations[0].sem_num   = 0;
		operations[0].sem_op    = 1;	// this the operation... the value is added to semaphore (a V-Op = +1)
		operations[0].sem_flg   = IPC_NOWAIT;	// set to IPC_NOWAIT to allow the calling process to fast-fail

        int retval = semop(semId, operations, numOps);

		if(0 == retval){

            // find shm offset for local child segment...
            shm += proc_id * (size / procs);

    //// ### NORMALIZE! ###

            std::vector<float> reference_vector = master.at(key);
            std::vector<norm_t> local_normalized;
            const uint32_t size = master.size();
            uint32_t count = 0;

            std::map<std::string, std::vector<float>>::iterator it = master.begin();
            std::advance( it, offset * (size / procs) );
            for( ; it != master.end(); ++it ){      // TODO: look into break condition
                // hacky way to read segments of map
                if( count >= ((uint32_t)offset + 1) / (uint32_t)procs ){
                    break;
                }

                // TODO: unroll loop
                float normal_sum = 0;
                for( int i = 0; i < 4096; i++ ){
                    normal_sum += std::abs(it->second[i] - reference_vector[i]) / size;
                }// end vector for
                
                norm_t temp_norm = {it->first, normal_sum};
                local_normalized.push_back(temp_norm);
           }// end iterator for

            // now need to copy local normalized over to shm...



        }        
        else{
            std::cerr << "[DEBUG]In the child (if): Failed P-operation on (" << semId << ")" << std::endl; 
			_exit(1);
        }

        // Release the semaphore (V-op)
		operations[0].sem_op = 1; 	// this the operation... the value is added to semaphore (a V-Op = 1)
		retval = semop(semId, operations, numOps);
		if(0 == retval){
			std::cout << "[DEBUG]In the child (if): Successful V-operation on (" << semId << ")" << std::endl; 
		}
		else{
			std::cerr << "In the child (if): Failed V-operation on (" << semId << ")" << std::endl; 
		}

		_exit(0);
    }// end child

	int status;	// catch the status of the child
	do { 
		pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1){
			std::cerr << "Error waiting for child process ("<< pid <<")" << std::endl;
			break;
		}
		
		if (WIFEXITED(status)){
			if (status > 0){
				std::cerr << "[DEBUG]Child process ("<< pid <<") exited with non-zero status of " << WEXITSTATUS(status) << std::endl;
				continue;
			}
			else{
				std::cout << "[DEBUG]Child process ("<< pid <<") exited with status of " << WEXITSTATUS(status) << std::endl;
				continue;
			}
		}
		else if (WIFSIGNALED(status)){
			std::cout << "[DEBUG]Child process ("<< pid <<") killed by signal (" << WTERMSIG(status) << ")" << std::endl;
			continue;			
		}
		else if (WIFSTOPPED(status)){
			std::cout << "[DEBUG]Child process ("<< pid <<") stopped by signal (" << WSTOPSIG(status) << ")" << std::endl;
			continue;			
		}
		else if (WIFCONTINUED(status)){
			std::cout << "[DEBUG]Child process ("<< pid <<") continued" << std::endl;
			continue;
		}
	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

    std::vector<norm_t> normalized;

    return normalized;
}// normalize
#endif

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
}
pete::util::~util(){;}

void pete::util::import(){
    char *toks;
    std::string temp_fname;
    std::string line;
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

//            std::cout << unsigned(proc_id) << "-> idx: " << local_index << std::endl;

            local_index++;
        }// end iterator for

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] proc " << unsigned(proc_id) << " computing done" << GREY << std::endl;
        #endif

        std::sort( &local_norm[0], &local_norm[local_index-1], norm_sort );

        std::cout << unsigned(proc_id) << "->" <<local_norm[0].normal << ", " << local_norm[1].normal << ", " << local_norm[2].normal << "\n" << std::endl;

        // jump to correct block in shared memory
        shm += proc_id * K;    

        std::cout << "[DEBUG] jumped to " << proc_id * K << std::endl;

        // store local_norm into shaBRED mem
        std::copy( local_norm, local_norm+K, shm);    //TODO memcpy or copy??

        #if DEBUG
        std::cout << MAGENTA << "[DEBUG] ending child proc " << unsigned(proc_id) << GREY << std::endl;
        #endif
		_exit(0);
    }// END CHILD
// ### PARENT PROC ###
	int status;	// catch the status of the child
	do { 
		pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1){
            #if DEBUG
			std::cerr << BRED << "[DEBUG] Error waiting for child process ("<< pid <<")" << GREY << std::endl;
            #endif
			break;
		}
        std::cout << "~~~[DEBUG] wait signal " << w << std::endl;
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
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

    #if DEBUG
    std::cout << MAGENTA << "[DEBUG] all children done " << GREY << std::endl;
    #endif


// std::cout << "...to" << K*procs << std::endl;
// read normalized vector out of shared memory
    normalized.assign( shm, shm+(K*procs) );
    std::sort( normalized.begin(), normalized.end(), norm_sort );

// remove shm!
    shmctl(shmId,IPC_RMID,0);

}// end parallel_normalize


/*
 * getRefKey - get row object from row id
 **/
csvRow_t pete::util::getRefKey(const std::string rowName){
    csvRow_t ref;

    for( uint32_t i = 0; i < master.size(); i++ ){
        if( master[i].fname.compare( rowName ) == 0 ){
            ref = master[i];
            break;
        }
    }

    return ref;
}// end getRefKey

