//
// Store K5-sampled data into shared memory
//
#include "shm_k5data.inc"
// #include "k5dict.inc"
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/u3tdsio.h>

#define DEV_FILE "/dev/u3tds0" // Device driver of VSSP64


int main(int argc, char **argv)
{
    int           shrd_param_id;                 // Shared Memory ID
    int           shrd_k5data_id;                // Shared Memory ID
    struct        SHM_PARAM *param_ptr;          // Pointer to the Shared Param
    struct        sembuf sops;                   // Semaphore
    char          dev[] = DEV_FILE;              // Device for K5/VSSP
    int           fd_in;                         // ID of the K5/VSSP32 device
    int           rv;                            // Return Value from K5/VSSP32
    unsigned char *k5head_ptr;                   // Pointer to the shared K5 header
    unsigned char *k5data_ptr;                   // Pointer to the shared K5 data
    unsigned char *shm_write_ptr;                // Writing Pointer

    // FILE          *dumpfile_ptr;                 // Dump File
    int           K5HEAD_CH[] = {1, 2};          // 0:1ch, 1:4ch
    unsigned int  year;
    int           index;
    int           num_read_cycle, read_fraction; // K5/VSSP64 Reading cycle and fraction
    unsigned int  fsample, qbit, num_IF, filter;
    char          buf[K5FIFO_SIZE];
    
    /* -------------------------------- */
    unsigned int  err, status;
    unsigned int  date, doy, sec;
    unsigned int  dev_time;
    struct        tm tm;
    time_t        t=time(NULL);
    /* -------------------------------- */

    // Open K5/VSSP64 device
    fd_in = open(dev, O_RDONLY);
    if((fd_in = open(dev, O_RDONLY)) == -1){
        perror("  Error : Can't open K5/VSSP64 device");
        return(-1);
    }

    // Access to the SHARED MEMORY
    // SHARED PARAMETERS
    if(shm_access(SHM_PARAM_KEY, sizeof(struct SHM_PARAM), &shrd_param_id, &param_ptr) == -1){
        perror("  Error : Can't access to the shared memory!!");
        return(-1);
    }
    printf("Succeeded to access the shared parameter [%d]!\n",  param_ptr->shrd_param_id);

    // SHARED K5 Header and Data
    k5head_ptr = shmat( param_ptr->shrd_k5head_id, NULL, 0 );
    k5data_ptr = shmat( param_ptr->shrd_k5data_id, NULL, 0 );
    param_ptr->validity |= ENABLE; // Set Shared memory readiness bit to 1
    // dumpfile_ptr = fopen("K5.dump", "w");
    
    // Start Sampling
    rv = ioctl(fd_in, TDSIO_INIT);
    printf("TDSIO_INIT result in %d\n", rv);
    rv = ioctl(fd_in, TDSIO_BUFFER_CLEAR);

    // changed - tu - 17/06/16
    // Get current time
    localtime_r(&t, &tm);
    sec = ((tm.tm_hour*60) + tm.tm_min)*60 + tm.tm_sec;
    doy = tm.tm_yday + 1;
    date = TDS_MAKE_TIME(doy,sec); // Make the time suitable for sampler
    printf("time(doy:sec) set to %d:%d (0x%08x)\n", doy, sec, date);
    rv = ioctl(fd_in, TDSIO_SET_TIME, &date); // Set the time on the sampler
    // printf("TDSIO_SET_TIME result in %d\n", rv);

    // Get current year
    year = (tm.tm_year - 100) & 0xff;
    rv = ioctl(fd_in, TDSIO_SET_YEAR, &year); // Set the year on the sampler
    // printf("TDSIO_SET_YEAR result in %d\n", rv);

    // Get the time from sampler
    rv = ioctl(fd_in, TDSIO_GET_TIME, &dev_time);
    // printf("TDSIO_GET_TIME result in %d\n", rv);
    rv = ioctl(fd_in, TDSIO_GET_YEAR, &dev_time);
    // printf("TDSIO_GET_YEAR reslut in %d\n", rv);
    //

    //---- Set sampling frequency
    index = param_ptr->fsample/ 1000000;
    fsample = TDSIO_SAMPLING_1MHZ;
    while(index > 1){
        fsample++;
        index = index >> 1;
    }
    rv = ioctl(fd_in, TDSIO_SET_FSAMPLE, &fsample);

    // Set Quantization Bits
    index = param_ptr->qbit;
    qbit = TDSIO_SAMPLING_1BIT;
    while(index > 1){
        qbit++;
        index = index >> 1;
    }
    rv = ioctl(fd_in, TDSIO_SET_RESOLUTIONBIT, &qbit);


    // Set Antialias Filter
    if((param_ptr->filter <= 0) || (param_ptr->filter > 16)){
        filter = TDSIO_SAMPLING_THRU;
    }
    else{
        index = 16 / param_ptr->filter;
        filter = TDSIO_SAMPLING_16M;
        while(index > 1){
            filter++;
            index = index >> 1;
        }
    }
    rv = ioctl(fd_in, TDSIO_SET_FILTER, &filter);

    // Set Number of IFs
    // num_IF  = TDSIO_SAMPLING_4CH;
    num_IF = TDSIO_SAMPLING_2CH;
    rv = ioctl(fd_in, TDSIO_SET_CHMODE, &num_IF);

    // Check Sampling Modes
    rv = ioctl(fd_in,TDSIO_GET_FSAMPLE, &fsample);
    printf("Fsample=%u\n", fsample);
    rv = ioctl(fd_in,TDSIO_GET_RESOLUTIONBIT,  &qbit);
    printf("Qbit =%u\n", qbit);
    rv = ioctl(fd_in,TDSIO_GET_CHMODE,  &num_IF);
    printf("Num of IF =%u\n", num_IF);
    rv = ioctl(fd_in,TDSIO_GET_FILTER,  &filter);
    printf("Filter =%u\n", filter);

    /* -------------------------------- */
    rv = ioctl(fd_in, TDSIO_GET_STATUS, &status);
    printf("Status = 0x%02x\n" ,status);
    rv = ioctl(fd_in, TDSIO_GET_ERROR, &err);
    printf("k5sample_store : error = %d\n" ,err);
    /* -------------------------------- */

    //-------- Start Sampling
    // rv = ioctl(fd_in, TDSIO_SAMPLING_START);
    if(rv = ioctl(fd_in, TDSIO_SAMPLING_START) == -1){
        printf("Error : Can't Start Sampling.\n");
        return(-1);
    }
    else{
        printf("TDSIO_SAMPLING_START result in %d\n", rv); // Size of 1-sec sampling data [bytes]
    }
    param_ptr->sd_len = MAX_SAMPLE_BUF;
    param_ptr->num_st = K5HEAD_CH[num_IF];            // Number of IFs
    num_read_cycle = param_ptr->sd_len / K5FIFO_SIZE; // Number of read cycles
    read_fraction  = param_ptr->sd_len % K5FIFO_SIZE; // Fraction bytes
    param_ptr->validity |= ACTIVE;                    // Set Sampling Activity Bit to 1

    while(param_ptr->validity & ACTIVE){
        if( param_ptr->validity & (FINISH + ABSFIN) ){
            break;
        }
        // start = clock();

        // Read Header
        rv = read(fd_in, k5head_ptr, K5HEAD_SIZE);
        // printf("READ result in %d : %X\n", rv, (int)k5head_ptr);

        // Read Data
        shm_write_ptr = k5data_ptr; // Initialize pointer

        // First half
        // start = clock();
        for(index=0; index<num_read_cycle/2 + 2; index++){
            rv = read(fd_in, shm_write_ptr, K5FIFO_SIZE);
            shm_write_ptr += rv;
        }


        // Semaphore for First Half--------
        // printf("k5sample: 1st. before %f\n", clock()/CLOCKS_PER_SEC*1000);
        for(index=0; index<Nif; index++){
            sops.sem_num = (ushort)index;
            sops.sem_op = (short)1;
            sops.sem_flg = (short)0;
            semop(param_ptr->sem_data_id, &sops, 1);
        }

        // printf("k5sample: 1st.  after %f\n", clock());
        // end = clock();
        // printf("k5sample: 1st. Half %f[msec]\n", (double)(end-start)/CLOCKS_PER_SEC*1000);

        // Last half
        // start = clock();
        for(index=num_read_cycle/2 + 2; index<num_read_cycle; index++){
            rv = read(fd_in, shm_write_ptr, K5FIFO_SIZE);
            shm_write_ptr += rv;
        }
        rv = read(fd_in, shm_write_ptr, read_fraction);
        shm_write_ptr += rv;

        // Semaphore for First Half--------
        // printf("k5sample: 2nd. before %f\n", clock()/CLOCKS_PER_SEC*1000);
        for(index=Nif; index<2*Nif; index++){
            sops.sem_num = (ushort)index;
            sops.sem_op = (short)1;
            sops.sem_flg = (short)0;
            semop(param_ptr->sem_data_id, &sops, 1);
        }
        // printf("k5sample: 2nd. after\n");
        // end = clock();
        // printf("k5sample: 2nd Half %f[msec]\n", (double)(end-start)/CLOCKS_PER_SEC*1000);
        // fwrite(k5data_ptr, MAX_SAMPLE_BUF, 1, dumpfile_ptr);
    }

    // Stop Sampling
    // fclose(dumpfile_ptr);
    rv = ioctl(fd_in, TDSIO_SAMPLING_STOP);
    close(fd_in);
    param_ptr->validity &= (~ACTIVE); // Set Sampling Activity Bit to 0


    return(0);
}
