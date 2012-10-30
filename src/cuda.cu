//2-opt in cuda

#include <stdlib.h>
#include <stdio.h>


__global__ void kernel(int *array)
{
  int index = blockIdx.x * blockDim.x + threadIdx.x;

  array[index] = 7;
}
