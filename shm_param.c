//
// Open Shared Parameter Area
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
    if(shm_init_create(SHM_PARAM_KEY, sizeof(struct SHM_PARAM), &shrd_param_id, &param_ptr) == -1){
        perror("Can't Create Shared Parameter!!\n");
        return(-1);
    }
    param_ptr->shrd_param_id = shrd_param_id;
    printf("Allocated %d bytes for Shared Param [%d]!\n" ,sizeof(struct SHM_PARAM), param_ptr->shrd_param_id);

    // Wait until finish
    while((param_ptr->validity & ABSFIN) == 0 ){
        if((param_ptr->integ_rec > 0) && (param_ptr->current_rec >= param_ptr->integ_rec - 1)){
            param_ptr->validity |= FINISH;
        }

        // Soft finish (Release SHM after 5 seconds)
        if((param_ptr->validity & FINISH) != 0){
            sleep(5);
            break;
        }
        sleep(1);
    }

    // Release the shared memory
    erase_shm(param_ptr);
    return(0);
}
