//
// Initialize Shared Memory Area and finish all processes
//
#include "shm_k5data.inc"

int main(int argc, char **argv)
{
    int    shrd_param_id;        // Shared Memory ID
    struct SHM_PARAM *param_ptr; // Pointer to the Shared Param

    //
    // ALLOC SHARED MEMORY
    //
    // Shared parameters
    shm_access(
        SHM_PARAM_KEY,            // ACCESS KEY
        sizeof(struct SHM_PARAM), // SIZE OF SHM
        &shrd_param_id,           // SHM ID
        &param_ptr                // Pointer to the SHM
    );

    //
    // WAIT UNTIL FINISH
    //
    // Immediate finish
    if(argc >= 2){
        param_ptr->validity ^= atoi(argv[1]); // Toggle Flag
    }
    else{
        param_ptr->validity |= FINISH;
    }
    printf("SET VALIDITY %X\n", param_ptr->validity);

    return(0);
}
