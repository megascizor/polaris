//
// Access to the Shared Memory
//
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

int shm_access(
        key_t  shm_key,  //  INPUT: Keyword for Shared Memory
        size_t shm_size, //  INPUT: Size of the SHM [bytes]
        int    *shrd_id, // OUTPUT: Pointer to the Shared Memory ID
        int    **shm_ptr // OUTPUT: Pointer to the Top of SHM
    )
{
    //
    // OPEN SHARED MEMORY
    //
    // Access to current status
    *shrd_id = shmget(shm_key, shm_size, 0444);
    if(*shrd_id < 0){
        printf("Can't Access to the Shared Memory !! \n");
        return(-1);
    }
    *shm_ptr = (int *)shmat(*shrd_id, NULL, 0);


    return(*shrd_id);
}
