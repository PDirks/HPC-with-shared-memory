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
#include <unistd.h>  // fork

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

const std::string fname = "../data/2100_HPC.csv";
const uint32_t size = 2100;
const std::string refName = "agricultural/agricultural00.tif";
const uint32_t K = 25;
const uint8_t procs = 3;

int main (void){

////  ### read test ###
    std::cout << "read test ";
    std::map<std::string, std::vector<float>> csv_Map = csv_read(fname);
    assert( csv_Map.size() == size );
    std::cout << ".\n";

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
    int shmId; 			// ID of shared memory segment
	key_t shmKey = 123460; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag = IPC_CREAT | 0666; // Flag to create with rw permissions
	
	// This will be shared:
	unsigned long * shm;

    // shmget() returns the identifier of the shared memory segment associated with the value of the argument key.
	// A new shared memory segment, with size equal to the value of size rounded up to a multiple of PAGE_SIZE, is
	// created  if  key has the value IPC_PRIVATE or key isn't IPC_PRIVATE, no shared memory segment corresponding
	// to key exists, and IPC_CREAT is specified in shmflg.
	if ((shmId = shmget(shmKey, 2 * sizeof(unsigned long), shmFlag)) < 0){
		std::cerr << "Init: Failed to initialize shared memory (" << shmId << ")" << std::endl; 
		exit(1);
	}
    
    // shmat() attaches the shared memory segment identified by shmid to the address space of the calling process.
    if ((shm = (unsigned long *)shmat(shmId, NULL, 0)) == (unsigned long *) -1){
		std::cerr << "Init: Failed to attach shared memory (" << shmId << ")" << std::endl; 
		exit(1);
	}
    shm[0] = 0;
    
//// ### fork test ###
    pid_t pid;
    for( uint8_t i = 0; i < procs; i++){
        //std::cout << "[DEBUG] spinning up proc " << std::endl;
        pid = fork();
        if( pid == 0 ){
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
            std::cout << "child id:" << *shm <<std::endl;
            *shm += 1;
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

//// ### normalize test ###
    std::cout << "normalize test (" << pid << ") ";
    std::vector<norm_t> normalized = normalize( csv_Map, refName );

    assert( normalized.at(0).normal == 0 );
    std::sort( normalized.begin(), normalized.end(), norm_sort );

    std::cout << "."<< std::endl;

//// ### print out normalized ###

    //print_norm( normalized, K );

    return 0;
}
