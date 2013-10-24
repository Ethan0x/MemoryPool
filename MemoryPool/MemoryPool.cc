//
//MemoryPool.cc
//

#include "HeaderFiles.h"
#include "MemoryPool.h"

namespace MemoryPool
{
	static const int FREED_MEMORY_CONTENT         = 0xAA;	//Value for feed memory
	static const int NEW_ALLOCATED_MEMORY_CONTENT = 0xFF;	//Initial value for new allocated memory

	//
	//Constructor
	//
	MemoryPool::MemoryPool(const std::size_t &sInitialMemoryPoolSize,
		const std::size_t &sMemoryChunkSize,
		const std::size_t &sMinimalMemorySizeToAllocate,
		bool bSetMemoryData)
	{
		m_ptrFirstChunk = NULL;
		m_ptrLastChunk = NULL;
		m_ptrCursorChunk = NULL;

		m_sTotalMemoryPoolSize = 0;
		m_sUsedMemoryPoolSize = 0;
		m_sFreeMemoryPoolSize = 0;

		m_sMemoryChunkSize = sMemoryChunkSize;
		m_uiMemoryChunkCount = 0;
		m_uiObjectCount = 0;

		m_bSetMemoryData = bSetMemoryData;
		m_sMinimalMemorySizeToAllocate = sMinimalMemorySizeToAllocate;

		AllocateMemory(sInitialMemoryPoolSize);	// Allocate the Initial amount of Memory from the OS
	}

	//
	//Destructor
	//
	MemoryPool::~MemoryPool()
	{
		FreeAllAllocatedMemory();
		DeallocateAllChunks();
		assert((m_uiObjectCount == 0) && "WARNING : Memory-Leak : You have not freed all allocated Memory");	// Check for possible Memory-Leaks
	}

	//
	//GetMemory
	//
	void *MemoryPool::GetMemory(const std::size_t &sMemorySize)
	{
		std::size_t sBestMemBlockSize = CalculateBestMemoryBlockSize(sMemorySize);
		MemoryChunk *ptrChunk = NULL;
		while (!ptrChunk)
		{
			ptrChunk = FindChunkSuitableToHoldMemory(sBestMemBlockSize);	//Is a Chunks available to hold the requested amount of Memory
			if (!ptrChunk)
			{
				//No chunk can be found,so MemoryPool is to small. We have to request more Memory from the OS
				sBestMemBlockSize = MaxValue(sBestMemBlockSize, CalculateBestMemoryBlockSize(m_sMinimalMemorySizeToAllocate));
				AllocateMemory(sBestMemBlockSize);
			}
		}

		//Finally, a suitable Chunk was found.Adjust the Values of the internal "TotalSize"/"UsedSize" Members and the Values of the MemoryChunk itself.
		m_sUsedMemoryPoolSize += sBestMemBlockSize;
		m_sFreeMemoryPoolSize -= sBestMemBlockSize;
		m_uiMemoryChunkCount++;
		SetMemoryChunkValues(ptrChunk, sBestMemBlockSize);

		return ((void*)ptrChunk->Data);
	}


}