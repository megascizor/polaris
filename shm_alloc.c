//
// Allocate Shared Memory Area
//
#include "shm_k5data.inc"

int main(int argc, char **argv)
{
    int	          shrd_param_id;        // Shared Memory ID
    struct        SHM_PARAM *param_ptr; // Pointer to the Shared Param
    struct        sembuf sops;          // Semaphore for data area
    unsigned char *k5head_ptr;          // Pointer to the Shared Data
    unsigned char *k5data_ptr;          // Pointer to the Shared Data
    float	      *segdata_ptr;         // Pointer to the Shared segment data
    float 		  *aspec_ptr;           // Pointer to the	auto-power spectra
    float	      *xspec_ptr;           // Pointer to the cross-power spectra
    int	          index;

    // Access to the Shared Param
    if(shm_access(SHM_PARAM_KEY, sizeof(struct SHM_PARAM), &shrd_param_id, &param_ptr) == -1){
        perror("  Error : shm_alloc() can't access to the shared memory!!");
        return(-1);
    };

    //
    // ALLOC SHARED MEMORY
    //
    // Semaphore for data area
    param_ptr->sem_data_id = semget(SEM_DATA_KEY, SEM_NUM, IPC_CREAT | 0666);
    for(index=0; index<SEM_NUM; index++){
        sops.sem_num = (ushort)index;
        sops.sem_op = (short)0;
        sops.sem_flg = IPC_NOWAIT;
        semop( param_ptr->sem_data_id, &sops, 1);
    }

    // Shared K5-HEADER area
    if(shm_init_create(K5HEAD_KEY, 5HEAD_SIZE, &(param_ptr->shrd_k5head_id), &k5head_ptr) == -1){
        perror("Can't Create Shared K5 header!!\n");
        return(-1);
    }
    memset(k5head_ptr, 0x00, K5HEAD_SIZE);
    printf("Allocated %d bytes for Shared K5 header [%d]!\n", K5HEAD_SIZE, param_ptr->shrd_k5head_id);

    // Shared K5-DATA area
    if(shm_init_create(K5DATA_KEY, MAX_SAMPLE_BUF, &(param_ptr->shrd_k5data_id), &k5data_ptr) == -1){
        perror("Can't Create Shared K5 data!!\n");
        return(-1);
    }
    memset(k5data_ptr, 0x00, MAX_SAMPLE_BUF);
    printf("Allocated %d bytes for Shared K5 data [%d]!\n", MAX_SAMPLE_BUF, param_ptr->shrd_k5data_id);

    // SHARED  Auto-power-spectram data area
    if(shm_init_create(ASPEC_KEY, ASPEC_SIZE, &(param_ptr->shrd_aspec_id), &aspec_ptr) == -1){
        perror("Can't Create Shared ASPEC data!!\n"); return(-1);
    }
    printf("Allocated %lu bytes for Shared Aspec data [%d]!\n", ASPEC_SIZE, param_ptr->shrd_aspec_id);

    // Shared Cross-power-spectram data area
    if(shm_init_create(XSPEC_KEY, XSPEC_SIZE, &(param_ptr->shrd_xspec_id), &xspec_ptr) == -1){
        perror("Can't Create Shared XSPEC data!!\n");
        return(-1);
    }
    printf("Allocated %lu bytes for Shared Xspec data [%d]!\n", XSPEC_SIZE, param_ptr->shrd_xspec_id);


    exit(0);
}
