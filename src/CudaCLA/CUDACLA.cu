#include "stdio.h"
#include "string.h"
/////////////////////////////////////
//Database Compare TC matrix Calculation
/////////////////////////////////////
int* d_TargetMagArray;
int* d_TargetMatrix;
int* d_RefMatrix;
float* d_TCMatrix;
__constant__ int constMagArray[129];
int* d_MaxTCIndex;
float* d_MaxTCValue;

#define SUCCESS 1
#define UNKNOWN_ERROR 0
#define MEMORY_ALLOC_FAIL 2

extern "C" int GetDeviceInfo(char* pDeviceName, size_t* pTotalMem, int* pClockRate, int* pNumProcessor)
{
	int nDevice;
	if(cudaGetDeviceCount(&nDevice)!=cudaSuccess)
		nDevice=-1;
	if(nDevice>0)
	{
		cudaDeviceProp myProp;
		cudaSetDevice(0);
		cudaGetDeviceProperties(&myProp,0);
		strcpy(pDeviceName,myProp.name);
		*pTotalMem=myProp.totalGlobalMem;
		*pClockRate=myProp.clockRate;
		*pNumProcessor=myProp.multiProcessorCount;
	}

	return nDevice;
}

__global__ void TCMatrixKernel(int nTargetCount, int nTargetRealCount, int nTargetDim, int* d_TargetMagArray, int* d_TargetMatrix,
						int nRefCount, int nRefDim, int* d_RefMatrix,float* d_TCMatrix,int Offset,int TCMatrixPitch)
{
	if(blockIdx.x+Offset<nRefCount)
	{
		__shared__ int sharedQuery[512];
		int RefMag=constMagArray[blockIdx.x];
		int TargetMag;
		if(threadIdx.x<RefMag)
		{
			sharedQuery[threadIdx.x]=d_RefMatrix[(blockIdx.x+Offset)*nRefDim + threadIdx.x];
		}
		__syncthreads();
		int TargetIndex,RefIndex,UnionCount;
		int buffer;
		int index;
		
		for(int i=0;i<(nTargetCount+511)/512;i++)
		{
			index=threadIdx.x+i*512;
			if(index<nTargetRealCount)
			{
				TargetMag=d_TargetMagArray[index];
				RefIndex=UnionCount=0;
				for(TargetIndex=0;TargetIndex<TargetMag ;TargetIndex++)
				{
					buffer=d_TargetMatrix[TargetIndex*nTargetCount+index];
					while(sharedQuery[RefIndex]<buffer && RefIndex<RefMag)
						RefIndex++;
					if(buffer==sharedQuery[RefIndex])
						UnionCount++;
				}
				d_TCMatrix[blockIdx.x*TCMatrixPitch+index]=(float)UnionCount/(RefMag+TargetMag-UnionCount);
			}
		}
	}
	
}
__global__ void ExcludeSelf(int nTargetRealCount, float* d_TCMatrix, int Offset, int TCMatrixPitch)
{
	if(threadIdx.x<128)
	{
		int index=Offset+threadIdx.x;
		if(index<nTargetRealCount)
		{
			d_TCMatrix[threadIdx.x*TCMatrixPitch+index]=0;
		}
	}
}
__global__ void GetMaxTCKernel(int nTargetRealCount, float* d_MaxTCValue, int* d_MaxTCIndex, float* d_TCMatrix, int TCMatrixPitch)
{
	__shared__ int sMaxTCIndex[256];
	__shared__ float sMaxTCValue[256];
	float buffer;
	
	unsigned int tid=threadIdx.x;
	if(tid<nTargetRealCount)
	{
		sMaxTCValue[tid]=d_TCMatrix[blockIdx.x*TCMatrixPitch+tid];
		sMaxTCIndex[tid]=tid;
	}
	tid+=blockDim.x;
	while(tid<nTargetRealCount)
	{
		buffer=d_TCMatrix[blockIdx.x*TCMatrixPitch+tid];
		if(buffer>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=buffer;
			sMaxTCIndex[threadIdx.x]=tid;
		}
		tid+=blockDim.x;
	}
	__syncthreads();
	if(threadIdx.x<128)
	{
		if(sMaxTCValue[threadIdx.x+128]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+128];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+128];

		}
		__syncthreads();
	}
	if(threadIdx.x<64)
	{
		if(sMaxTCValue[threadIdx.x+64]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+64];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+64];

		}
		__syncthreads();
	}
	if(threadIdx.x<32)
	{
		if(sMaxTCValue[threadIdx.x+32]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+32];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+32];

		}
	}
	if(threadIdx.x<16)
	{
		if(sMaxTCValue[threadIdx.x+16]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+16];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+16];

		}
	}
	if(threadIdx.x<8)
	{
		if(sMaxTCValue[threadIdx.x+8]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+8];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+8];

		}
	}
	if(threadIdx.x<4)
	{
		if(sMaxTCValue[threadIdx.x+4]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+4];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+4];

		}
	}
	if(threadIdx.x<2)
	{
		if(sMaxTCValue[threadIdx.x+2]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+2];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+2];

		}
	}
	if(threadIdx.x<1)
	{
		if(sMaxTCValue[threadIdx.x+1]>sMaxTCValue[threadIdx.x])
		{
			sMaxTCValue[threadIdx.x]=sMaxTCValue[threadIdx.x+1];
			sMaxTCIndex[threadIdx.x]=sMaxTCIndex[threadIdx.x+1];

		}
		d_MaxTCValue[blockIdx.x]=sMaxTCValue[threadIdx.x];
		d_MaxTCIndex[blockIdx.x]=sMaxTCIndex[threadIdx.x];
	}
	

}
/////////////////////////////////////////////////////
// Calculate TC Histogram
////////////////////////////////////////////////////

__global__ void ComputeTCHist(float* d_Data, int nRefCount, int* d_TCHist,int batch)
{
	__shared__ unsigned int s_Hist[3840];
	int WarpId=threadIdx.x>>5;
	int ThreadIndex=threadIdx.x-WarpId*32;
	int bin;
	int i,j,index;
	for(j=0;j<5;j++)
	{
		for(i=0;i<20;i++)
			s_Hist[i*192+threadIdx.x]=0;
		__syncthreads();
		for(i=0;i<batch;i++)
		{
			index=threadIdx.x+192*i+blockIdx.x*batch*192;
			if(index<nRefCount)
			{
				bin=floorf(d_Data[index]*99.0001f);
				
				if(bin>=j*20 && bin<j*20+20)
				{
					bin=bin-j*20;
					s_Hist[WarpId*640+bin*32+ThreadIndex]++;
				}
				
			}
		}
		__syncthreads();
		for(i=1;i<6;i++)
		{
			s_Hist[threadIdx.x]+=s_Hist[threadIdx.x+i*640];
			s_Hist[threadIdx.x+192]+=s_Hist[threadIdx.x+i*640+192];
			s_Hist[threadIdx.x+384]+=s_Hist[threadIdx.x+i*640+384];
			if(threadIdx.x<64)
				s_Hist[threadIdx.x+576]+=s_Hist[threadIdx.x+i*640+576];
		}
		__syncthreads();
		if(threadIdx.x<20)
		{
			for(i=1;i<32;i++)
			{
				//bank conflict
				s_Hist[threadIdx.x*32]+=s_Hist[threadIdx.x*32+i];
			}
			d_TCHist[blockIdx.x*128+threadIdx.x+j*20]=s_Hist[threadIdx.x*32];
			
		}
		__syncthreads();
	}
}
__global__ void addByte(int* d_Data,int dim,int N)
{
	int i;
	if(threadIdx.x<N)
	{
		for(i=1;i<dim;i++)
		{
			d_Data[threadIdx.x]+=d_Data[threadIdx.x+N*i];
		}
	}
}
extern "C" int SPADBTC(int nTargetCount, int nTargetRealCount, int nTargetDim, int* pTargetMagArray, int* pTargetMatrix,
						int nRefCount, int nRefRealCount, int nRefDim, int* pRefMagArray, int* pRefMatrix, int SelfCompare, int* res,int* pProgress)
{
	
	int TCMatrixPitch=(nTargetCount+15)/16*16;
	float* d_MaxTCArray;
	int* d_MaxTCIndexArray;
	cudaError_t msg;
	msg=cudaMalloc((void**)&d_TargetMagArray,sizeof(int)*nTargetCount);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL;
	msg=cudaMalloc((void**)&d_TargetMatrix,sizeof(int)*nTargetCount*nTargetDim);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+1;
	msg=cudaMalloc((void**)&d_RefMatrix,sizeof(int)*nRefCount*nRefDim);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+2;
	msg=cudaMalloc((void**)&d_TCMatrix,sizeof(float)*TCMatrixPitch*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+3;
	msg=cudaMalloc((void**)&d_MaxTCIndexArray,sizeof(float)*(nRefCount+127)/128*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+4;
	msg=cudaMalloc((void**)&d_MaxTCArray,sizeof(float)*(nRefCount+127)/128*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+5;

	cudaMemcpy(d_TargetMagArray, pTargetMagArray, sizeof(int)*nTargetCount, cudaMemcpyHostToDevice);
	cudaMemcpy(d_TargetMatrix, pTargetMatrix, sizeof(int)*nTargetCount*nTargetDim, cudaMemcpyHostToDevice);
	cudaMemcpy(d_RefMatrix, pRefMatrix, sizeof(int)*nRefCount*nRefDim, cudaMemcpyHostToDevice);
	int threadsPerBlock = 512;
    int blocksPerGrid = 128;
	int Offset;int i;
	

	for(i=0;i<(nRefCount+127)/128;i++)
	{
		*pProgress=i+1;
		Offset=i*128;
		if(Offset+128<nRefCount)
			msg=cudaMemcpyToSymbol(constMagArray,pRefMagArray+Offset,sizeof(int)*128,0,cudaMemcpyHostToDevice);
		else
			msg=cudaMemcpyToSymbol(constMagArray,pRefMagArray+Offset,sizeof(int)*(nRefCount-Offset),0,cudaMemcpyHostToDevice);
		if(msg!=cudaSuccess)
			return UNKNOWN_ERROR;
		TCMatrixKernel<<<blocksPerGrid, threadsPerBlock>>>(nTargetCount, nTargetRealCount, nTargetDim, d_TargetMagArray, d_TargetMatrix,
							nRefCount, nRefDim, d_RefMatrix, d_TCMatrix, Offset,TCMatrixPitch);
		cudaThreadSynchronize();
		if(SelfCompare)
		{
			ExcludeSelf<<<1,192>>>(nTargetRealCount, d_TCMatrix, Offset, TCMatrixPitch);
			cudaThreadSynchronize();
		}
		GetMaxTCKernel<<<128,256>>>(nTargetRealCount, d_MaxTCArray+Offset, d_MaxTCIndexArray+Offset, d_TCMatrix, TCMatrixPitch);
	}
	
	//Histogram Calculation
	cudaThreadSynchronize();
	int* d_TCHist;
	msg=cudaMalloc((void**)&d_TCHist,sizeof(int)*128*16);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+6;
	
	ComputeTCHist<<<16,192>>>(d_MaxTCArray,nRefCount,d_TCHist,(nRefCount+3071)/3072);
	cudaThreadSynchronize();

	addByte<<<1,192>>>(d_TCHist,16,128);
	cudaThreadSynchronize();
	cudaMemcpy(res, d_TCHist, sizeof(int)*100, cudaMemcpyDeviceToHost);

	cudaFree(d_TCHist);
	cudaFree(d_TCMatrix);
	cudaFree(d_TargetMagArray);
	cudaFree(d_TargetMatrix);
	cudaFree(d_RefMatrix);
	cudaFree(d_MaxTCArray);
	cudaFree(d_MaxTCIndexArray);

	return SUCCESS;

}

///////////////////////////////////////////////////////
//TC calculated based on integer fingerprint and lookup table
/////////////////////////////////////////////////////////
__global__ void IntTCMatrixKernel(int nTargetCount, int nTargetRealCount, int nTargetDim, int* d_TargetMagArray, int* d_TargetMatrix,
						int nRefCount, int nRefDim, int* d_RefMatrix,float* d_TCMatrix,int Offset,int TCMatrixPitch /*,int* d_LookupTable*/)
{
	if(blockIdx.x+Offset<nRefCount)
	{
		extern __shared__ int sharedQuery[];
		int RefMag=constMagArray[blockIdx.x];
		int TargetMag;
		int index,i,j,UnionCount,buffer;
		if(threadIdx.x<nRefDim)
		{
			sharedQuery[threadIdx.x]=d_RefMatrix[(blockIdx.x+Offset)*nRefDim + threadIdx.x];
		}
		__syncthreads();
		
		for(i=0;i<(nTargetCount+255)/256;i++)
		{
			index=threadIdx.x+i*256;
			if(index<nTargetCount)
			{
				UnionCount=0;
				TargetMag=d_TargetMagArray[index];
				for(j=0;j<nTargetDim;j++)
				{
					buffer=d_TargetMatrix[j*nTargetCount+index]&sharedQuery[j];
					UnionCount+=__popc(buffer);
				}
				d_TCMatrix[blockIdx.x*TCMatrixPitch+index]=(float)UnionCount/(RefMag+TargetMag-UnionCount);
			}		
		}
	}
}

extern "C" int INTDBTC(int nTargetCount, int nTargetRealCount, int nTargetDim, int* pTargetMagArray, int* pTargetMatrix,
						int nRefCount, int nRefRealCount, int nRefDim, int* pRefMagArray, int* pRefMatrix, int SelfCompare, int* res, int* pProgress)
{
	float* d_MaxTCArray;
	int* d_MaxTCIndexArray;
	int TCMatrixPitch=(nTargetCount+15)/16*16;

	cudaError_t msg;
	msg=cudaMalloc((void**)&d_TargetMagArray,sizeof(int)*nTargetCount);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL;
	msg=cudaMalloc((void**)&d_TargetMatrix,sizeof(int)*nTargetCount*nTargetDim);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+1;
	msg=cudaMalloc((void**)&d_RefMatrix,sizeof(int)*nRefCount*nRefDim);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+2;
	msg=cudaMalloc((void**)&d_TCMatrix,sizeof(float)*TCMatrixPitch*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+3;

	msg=cudaMalloc((void**)&d_MaxTCIndexArray,sizeof(float)*(nRefCount+127)/128*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+4;

	msg=cudaMalloc((void**)&d_MaxTCArray,sizeof(float)*(nRefCount+127)/128*128);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+5;
	

	cudaMemcpy(d_TargetMagArray, pTargetMagArray, sizeof(int)*nTargetCount, cudaMemcpyHostToDevice);
	cudaMemcpy(d_TargetMatrix, pTargetMatrix, sizeof(int)*nTargetCount*nTargetDim, cudaMemcpyHostToDevice);
	cudaMemcpy(d_RefMatrix, pRefMatrix, sizeof(int)*nRefCount*nRefDim, cudaMemcpyHostToDevice);
	int threadsPerBlock = 256;
    int blocksPerGrid = 128;
	int Offset;int i;
	
	
	for(i=0;i<(nRefCount+127)/128;i++)
	{
		*pProgress=i+1;
		Offset=i*128;
		if(Offset+128<nRefCount)
			msg=cudaMemcpyToSymbol(constMagArray,pRefMagArray+Offset,sizeof(int)*128,0,cudaMemcpyHostToDevice);
		else
			msg=cudaMemcpyToSymbol(constMagArray,pRefMagArray+Offset,sizeof(int)*(nRefCount-Offset),0,cudaMemcpyHostToDevice);
		if(msg!=cudaSuccess)
			return UNKNOWN_ERROR;
		IntTCMatrixKernel<<<blocksPerGrid, threadsPerBlock,nRefDim*sizeof(int)>>>(nTargetCount, nTargetRealCount, nTargetDim, d_TargetMagArray, d_TargetMatrix,
							nRefCount, nRefDim, d_RefMatrix, d_TCMatrix, Offset,TCMatrixPitch);
		cudaThreadSynchronize();
		if(SelfCompare)
		{
			ExcludeSelf<<<1,192>>>(nTargetRealCount, d_TCMatrix, Offset, TCMatrixPitch);
			cudaThreadSynchronize();
		}
		GetMaxTCKernel<<<128,256>>>(nTargetRealCount, d_MaxTCArray+Offset, d_MaxTCIndexArray+Offset, d_TCMatrix, TCMatrixPitch);
		
	}
	cudaThreadSynchronize();
	int* d_TCHist;
	msg=cudaMalloc((void**)&d_TCHist,sizeof(int)*128*16);
	if(msg!=cudaSuccess)
		return MEMORY_ALLOC_FAIL+7;
	

	
	ComputeTCHist<<<16,192>>>(d_MaxTCArray,nRefCount,d_TCHist,(nRefCount+3071)/3072);
	cudaThreadSynchronize();

	addByte<<<1,192>>>(d_TCHist,16,128);
	cudaThreadSynchronize();
	cudaMemcpy(res, d_TCHist, sizeof(int)*100, cudaMemcpyDeviceToHost);


	cudaFree(d_TCHist);
	cudaFree(d_TCMatrix);
	cudaFree(d_TargetMagArray);
	cudaFree(d_TargetMatrix);
	cudaFree(d_RefMatrix);
	cudaFree(d_MaxTCArray);
	cudaFree(d_MaxTCIndexArray);

	return SUCCESS;

}
