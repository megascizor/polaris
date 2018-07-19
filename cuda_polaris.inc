//
// Functions for GPU Calculation
//
#include"shm_k5data.inc"
//#include "k5dict.inc"
#include <cuda.h>
#include <cufft.h>


// a x b
__device__ float2 complexMult(float2 a, float2 b)
{
    return make_float2(a.x* b.x - a.y* b.y, a.x* b.y + a.y* b.x);
}

// a x b*
__device__ float2 complexMultConj(float2 a, float2 b)
{
    return make_float2(a.x* b.x + a.y* b.y, a.y* b.x - a.x* b.y);
}

// |a|^2
__device__ float complexMod( float2 a )
{
    return  a.x* a.x + a.y* a.y;
}

// a × b* (Vector)
__global__ void complexMultConjVec(
    float2 *vec_in_a, // Input vector
    float2 *vec_in_b, // Input vector
    float2 *vec_out,  // Output vector
    int    length     // Vector length
)
{
    int tid = blockIdx.x* blockDim.x + threadIdx.x;
    if((tid >= 0) && (tid < length)){
        vec_out[tid] = complexMultConj(vec_in_a[tid], vec_in_b[tid]);
    }
}

// a × a* (Vector)
__global__ void complexPowerVec(		
    float2 *vec_in, // Input vector
    float  vec_out, // Output vector
    int    length   // Number of elements
)
{
    int tid = blockIdx.x* blockDim.x + threadIdx.x;
    if((tid >= 0) && (tid < length)){
        vec_out[tid] = complexMod(vec_in[tid]);
    }
}

// a <- a + b (Vector)
__global__ void accumReal(
    float *vec_in_a, // Accumuration Results to be accumulated
    float *vec_in_b,
    int   length
)
{
    int tid = blockIdx.x* blockDim.x + threadIdx.x;
    if((tid >= 0) && (tid < length)){
        vec_in_a[tid] += vec_in_b[tid];
    }
}

// a* <- a* + b*
__global__ void accumComplex(
    float2 *vec_in_a, // Accumuration Results to be accumulated
    float2 *vec_in_b,
    int    length
)
{
    int tid = blockIdx.x* blockDim.x + threadIdx.x;
    if((tid >= 0) && (tid < length)){
        vec_in_a[tid].x += vec_in_b[tid].x;
        vec_in_a[tid].y += vec_in_b[tid].y;
    }
}

// Accumurate Power Spectra
__global__ void accumPowerSpec(
    float2 *vec_in,  // Input vector to be accumulated
    float  *vec_out, // In/Output vector to accumulate
    int    length    // Length to accumulating vector
)
{
    int ix = blockIdx.x* blockDim.x + threadIdx.x;
    if((ix >= 0) && (ix < length)){
        vec_out[ix] += vec_in[ix].x *  vec_in[ix].x +  vec_in[ix].y *  vec_in[ix].y;
    }
}

// Accumurate Cross-power Spectra
__global__ void accumCrossSpec(
			       
    float2 *vec_in_a, // Input vector
    float2 *vec_in_b, // Input vector
    float2 *vec_out,  // Output vector
    int    length
)
{
    int ix = blockIdx.x * blockDim.x + threadIdx.x;
    if( (ix >= 0) && (ix < length) ){
        vec_out[ix].x += vec_in_a[ix].x * vec_in_b[ix].x + vec_in_a[ix].y * vec_in_b[ix].y;
        vec_out[ix].y += vec_in_a[ix].y * vec_in_b[ix].x - vec_in_a[ix].x * vec_in_b[ix].y;
    }
}

// Accumurate Phase from Cross-power Spectra
// __global__ void accumPhaseSpec(
//     float2 *vec_in,   // Input vector
//     float  *phase_out // Output phase
// )
// {
//     int ix = blockIdx.x * blockDim.x * threadIdx.x;
//     if((ix>=0) && (ix<length)){
//         phase_out = atan2(vec_in.y/vec_in.x);
//     }
// }

// Scale Auto-power Spectra
__global__ void scalePowerSpec(
    float *vec_in,   // Input vector to be accumulated
    float scaleFact, // Scaling Factor
    int   length     // Length to accumulating vector
)
{
    int ix = blockIdx.x* blockDim.x + threadIdx.x;
    if( (ix >= 0) && (ix < length) ){
        vec_in[ix] *= scaleFact;
    }
}

// Scale Cross-power Spectra
__global__ void scaleCrossSpec(
    float2 *vec_in,   // Input vector to be accumulated
    float  scaleFact, // Scaling Factor
    int	   length     // Length to accumulating vector
)
{
    int ix = blockIdx.x* blockDim.x + threadIdx.x;
    if( (ix >= 0) && (ix < length) ){
        vec_in[ix].x *= scaleFact;
        vec_in[ix].y *= scaleFact;
    }
}

// Scale Phase from Cross-power Spectra
// __global__ void scalePhaseSpec(
//     float2 *phase_in,
//     float  scaleFact,
//     int    length
// )
// {
//     int ix = blockIdx.x * blockDim.x + threadIdx.x;
//     if((ix>=0) && (ix<length)){
//         phase_in *= scaleFact;
//     }
// }

// Format segment data using 4-bit quantized K5 data
__global__ void segform4bit(
    unsigned char *k5data_in, // Input K5 Segmanet Data
    float         *segdata,   // Output Segment Data
    int           length      // Length to copy
)
{
    float	bias = 7.5;           // Mean value
    float	scale = 0.0625;       // Scaling by 1/16
    int ix = blockIdx.x* blockDim.x + threadIdx.x;
    unsigned char bitmask = 0x0f; // 4-bit mask

    if((ix >= 0) && (ix < length)){
        segdata[ix] = ((float)((k5data_in[2*ix])&bitmask) - bias) * scale;
        segdata[length + ix] = ((float)((k5data_in[2*ix] >> 4)) - bias) * scale;
        segdata[2*length + ix] = ((float)((k5data_in[2*ix+1]) & bitmask) - bias)* scale;
        segdata[3*length + ix] = ((float)((k5data_in[2*ix+1]>> 4)) - bias) * scale;
    }
}

// Format segment data using 8-bit quantized K5 data
__global__ void segform8bit(
    unsigned char *k5data_in, // Input K5 Segmanet Data
    float         *segdata,   // Output Segment Data
    int           length      // Length to copy
)
{
    float	bias = 127.5;		// Mean value
    float	scale = 0.00390625;	// Scaling by 1/256
    int ix = blockIdx.x* blockDim.x + threadIdx.x;
    if((ix >= 0) && (ix < length)){
        segdata[ix] = ((float)k5data_in[4*ix] - bias)*scale;
        segdata[length + ix] = ((float)k5data_in[4*ix+1] - bias)*scale;
        segdata[2*length + ix] = ((float)k5data_in[4*ix+2] - bias)*scale;
        segdata[3*length + ix] = ((float)k5data_in[4*ix+3] -  bias)*scale;
    }
}
