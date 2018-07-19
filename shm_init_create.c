//
// Create a Shared Memory Area
//
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>


int shm_init_create(
        key_t  shm_key,  //  INPUT: Keyword for Shared Memory
        size_t shm_size, //  INPUT: Size of the SHM [bytes]
        int    *shrd_id, // OUTPUT: Pointer to the Shared Memory ID
        int    **shm_ptr // OUTPUT: Pointer to the Shared Memory
    )
{
    //
    // Open shared memory
    //
    // Access to current status
    *shrd_id = shmget(shm_key, shm_size, IPC_CREAT|0666);
    if(*shrd_id < 0){
        printf("[shm_init_create]: Can't Access to the Shared Memory !! \n" );
        return(-1);
    }
    *shm_ptr = (int *)shmat(*shrd_id, NULL, 0);
    memset((void *)*shm_ptr, 0, shm_size);

    return(*shrd_id);
}
