//
//MemoryPool.cc
//

#include "HeaderFiles.h"
#include "MemoryPool.h"

namespace MemoryPool
{
	static const int FREED_MEMORY_CONTENT = 0xAA;	//Value for feed memory
	static const int NEW_ALLOCATED_MEMORY_CONTENT = 0xFF;	//Initial value for new allocated memory

	//
	//Constructor
	//
	MemoryPool::MemoryPool(const std::size_t &sInitialMemoryPoolSize, const std::size_t &sMemoryChunkSize,
		const std::size_t &sMinimalMemorySizeToAllocate, bool bSetMemoryData)
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

	//
	//FreeMemory
	//
	void MemoryPool::FreeMemory(void *ptrMemoryBlock, const std::size_t &sMemoryBlockSize)
	{
		//Search all Chunks for the one holding the "ptrMemoryBlock"-Pointer("SMemoryChunk->Data == ptrMemoryBlock"), so it beecomes available to the MemoryPool again.
		MemoryChunk *ptrChunk = FindChunkHoldingPointerTo(ptrMemoryBlock);
		if (ptrChunk)
		{
			//std::cerr << "Freed Chunks OK (Used memPool Size : " << m_sUsedMemoryPoolSize << ")" << std::endl ;
			FreeChunks(ptrChunk);
		}
		else
		{
			assert(false && "ERROR : Requested Pointer not in Memory Pool");
		}
		assert((m_uiMemoryChunkCount > 0) && "ERROR : Request to delete more Memory then allocated.");
		m_uiMemoryChunkCount--;
	}

	//
	//AllocateMemory
	//
	bool MemoryPool::AllocateMemory(const std::size_t &sMemorySize)
	{
		unsigned int uiNeedChunks = CalculateNeededChunks(sMemorySize);
		std::size_t sBestMemBlockSize = CalculateBestMemoryBlockSize(sMemorySize);

		TByte *ptrNewMemBlock = (TByte*)malloc(sBestMemBlockSize); //allocate from the OS
		MemoryChunk *ptrNewChunks = (MemoryChunk*)malloc((uiNeedChunks * sizeof(MemoryChunk)));	//allocate chunk array to manage the memory
		assert(((ptrNewMemBlock) && (ptrNewChunks)) && "Error : System ran out of Memory");

		m_sTotalMemoryPoolSize += sBestMemBlockSize;	//adjust internal values
		m_sFreeMemoryPoolSize -= sBestMemBlockSize;
		m_uiMemoryChunkCount += uiNeedChunks;

		if (m_bSetMemoryData)
		{
			memset(((void*)ptrNewMemBlock), NEW_ALLOCATED_MEMORY_CONTENT, sBestMemBlockSize);	//set the memory content to a defined value is useful for debug
		}

		return LinkChunksToData(ptrNewChunks, uiNeedChunks, ptrNewMemBlock);
	}

	//
	//CalculateNeededChunks
	//
	unsigned int MemoryPool::CalculateNeededChunks(const std::size_t &sMemorySize)
	{
		float f = (float)(((float)sMemorySize) / ((float)m_sMemoryChunkSize));
		return ((unsigned int)ceil(f));
	}

	//
	//CalculateBestMemoryBlockSize
	//
	std::size_t MemoryPool::CalculateBestMemoryBlockSize(const std::size_t &sRequestedMemoryBlockSize)
	{
		unsigned int uiNeededChunks = CalculateNeededChunks(sRequestedMemoryBlockSize);
		return std::size_t(uiNeededChunks * m_sMemoryChunkSize);
	}

	//
	//FreeChunks
	//
	void MemoryPool::FreeChunks(MemoryChunk *ptrChunk)
	{
		//Make the Used Memory of the given Chunk available to the Memory Pool again.
		MemoryChunk *ptrCurrentChunk = ptrChunk;
		unsigned int uiChunkCount = CalculateNeededChunks(ptrCurrentChunk->UsedSize);

		for (unsigned int i = 0; i < uiChunkCount; i++)
		{
			if (ptrCurrentChunk)
			{
				//Step 1 : Set the allocated Memory to 'FREEED_MEMORY_CONTENT' (Note : This is fully Optional, but usefull for debugging)
				if (m_bSetMemoryData)
				{
					memset(((void*)ptrCurrentChunk->Data), FREED_MEMORY_CONTENT, m_sMemoryChunkSize);
				}
				//Step 2 : Set the Used-Size to Zero
				ptrCurrentChunk->UsedSize = 0;
				//Step 3 : Adjust Memory-Pool Values and goto next Chunk
				m_sFreeMemoryPoolSize -= m_sMemoryChunkSize;
				ptrCurrentChunk = ptrCurrentChunk->Next;
			}
		}
	}

	//
	//FindChunkSuitableToHoldMemory
	//
	MemoryChunk *MemoryPool::FindChunkSuitableToHoldMemory(const std::size_t &sMemorySize)
	{
		//Find a Chunk to hold *at least* "sMemorySize" Bytes.
		unsigned int uiChunksToSkip = 0;
		bool bContinueSearch = true;
		MemoryChunk *ptrChunk = m_ptrCursorChunk;	//start search at cursor pos

		for (unsigned int i = 0; i < m_uiMemoryChunkCount; i++)
		{
			if (ptrChunk)
			{
				if (ptrChunk == m_ptrLastChunk)	//end of list reached, start over from the beginning 
				{
					ptrChunk = m_ptrFirstChunk;
				}

				if (ptrChunk->DataSize >= sMemorySize)
				{
					if (ptrChunk->UsedSize == 0)
					{
						m_ptrCursorChunk = ptrChunk;
						return ptrChunk;
					}

				}

				uiChunksToSkip = CalculateNeededChunks(ptrChunk->UsedSize);
				if (uiChunksToSkip == 0)
				{
					uiChunksToSkip = 1;
				}
				ptrChunk = SkipChunks(ptrChunk, uiChunksToSkip);
			}
			else
			{
				bContinueSearch = false;
			}
		}

		return NULL;
	}

	//
	//SkipChunks
	//
	MemoryChunk *MemoryPool::SkipChunks(MemoryChunk *ptrStartChunk, unsigned int uiChunksToSkip)
	{
		MemoryChunk *ptrCurrentChunk = ptrStartChunk;
		for (unsigned int i = 0; i < uiChunksToSkip; i++)
		{
			if (ptrCurrentChunk)
			{
				ptrCurrentChunk = ptrCurrentChunk->Next;
			}
			else
			{
				//Will occur, if you try to Skip more Chunks than actually available from your "ptrStartChunk" 
				assert(false && "Error : Chunk == NULL was not expected.");
				break;
			}
		}

		return ptrCurrentChunk;
	}

	//
	//SetMemoryChunkValues
	//
	void MemoryPool::SetMemoryChunkValues(MemoryChunk *ptrChunk, const std::size_t &sMemBlockSize)
	{
		if ((ptrChunk)) // && (ptrChunk != m_ptrLastChunk))
		{
			ptrChunk->UsedSize = sMemBlockSize;
		}
		else
		{
			assert(false && "Error : Invalid NULL-Pointer passed");
		}
	}

	//
	//WriteMemoryDumpToFile
	//
	bool MemoryPool::WriteMemoryDumpToFile(const std::string &strFileName)
	{
		bool bWriteSuccesfull = false;
		std::ofstream ofOutPutFile;
		ofOutPutFile.open(strFileName.c_str(), std::ofstream::out | std::ofstream::binary);

		MemoryChunk *ptrCurrentChunk = m_ptrFirstChunk;

		while (ptrCurrentChunk)
		{
			if (ofOutPutFile.good())
			{
				ofOutPutFile.write(((char*)ptrCurrentChunk->Data), ((std::streamsize)m_sMemoryChunkSize));
				bWriteSuccesfull = true;
			}
			ptrCurrentChunk = ptrCurrentChunk->Next;
		}
		ofOutPutFile.close();
		return bWriteSuccesfull;
	}

	//
	//LinkChunksToData
	//
	bool MemoryPool::LinkChunksToData(MemoryChunk *ptrNewChunks, unsigned int uiChunkCount, TByte *ptrNewMemBlock)
	{
		MemoryChunk *ptrNewChunk = NULL;
		unsigned int uiMemOffset = 0;
		bool bAllocationChunkAssigned = false;
		for (unsigned int i = 0; i < uiChunkCount; i++)
		{
			if (!m_ptrFirstChunk)
			{
				m_ptrFirstChunk = SetChunkDefaults(&(ptrNewChunks[0]));
				m_ptrLastChunk = m_ptrFirstChunk;
				m_ptrCursorChunk = m_ptrFirstChunk;
			}
			else
			{
				ptrNewChunk = SetChunkDefaults(&(ptrNewChunks[i]));
				m_ptrLastChunk->Next = ptrNewChunk;
				m_ptrLastChunk = ptrNewChunk;
			}

			uiMemOffset = (i * ((unsigned int)m_sMemoryChunkSize));
			m_ptrLastChunk->Data = &(ptrNewMemBlock[uiMemOffset]);
			
			//The first Chunk assigned to the new Memory-Block will be a "AllocationChunk".This means, this Chunks stores the
			//"original" Pointer to the MemBlock and is responsible for "free()"ing the Memory later
			if (!bAllocationChunkAssigned)
			{
				m_ptrLastChunk->IsAllocationChunk = true;
				bAllocationChunkAssigned = true;
			}
		}

		return RecalcChunkMemorySize(m_ptrFirstChunk, m_uiMemoryChunkCount);
	}

	//
	//RecalcChunkMemorySize
	//
	bool MemoryPool::RecalcChunkMemorySize(MemoryChunk *ptrChunk, unsigned int uiChunkCount)
	{
		unsigned int uiMemoryOffSet = 0;
		for (unsigned int i = 0; i < uiChunkCount; i++)
		{
			if (ptrChunk)
			{
				uiMemoryOffSet = (i * ((unsigned int)m_sMemoryChunkSize));
				ptrChunk->DataSize = (((unsigned int)m_sTotalMemoryPoolSize) - uiMemoryOffSet);
				ptrChunk = ptrChunk->Next;
			}
			else
			{
				assert(false && "Error : ptrChunk == NULL");
				return false;
			}
		}

		return true;
	}

	//
	//SetChunkDefaults
	//
	MemoryChunk *MemoryPool::SetChunkDefaults(MemoryChunk *ptrChunk)
	{
		if (ptrChunk)
		{
			ptrChunk->Data = NULL;
			ptrChunk->DataSize = 0;
			ptrChunk->UsedSize = 0;
			ptrChunk->IsAllocationChunk = false;
			ptrChunk->Next = NULL;
		}

		return ptrChunk;
	}

	//
	//FindChunkHoldingPointerTo
	//
	MemoryChunk *MemoryPool::FindChunkHoldingPointerTo(void *ptrMemoryBlock)
	{
		MemoryChunk *ptrTempChunk = m_ptrFirstChunk;
		while (ptrTempChunk)
		{
			if (ptrTempChunk->Data == ((TByte*)ptrMemoryBlock))
			{
				break;
			}
			ptrTempChunk = ptrTempChunk->Next;
		}

		return ptrTempChunk;
	}

	//
	//FreeAllAllocatedMemory
	//
	void MemoryPool::FreeAllAllocatedMemory()
	{
		MemoryChunk *ptrChunk = m_ptrFirstChunk;
		while (ptrChunk)
		{
			if (ptrChunk->IsAllocationChunk)
			{
				free((void*)(ptrChunk->Data));
			}
			ptrChunk = ptrChunk->Next;
		}
	}

	//
	//DeallocateAllChunks
	//
	void MemoryPool::DeallocateAllChunks()
	{
		MemoryChunk *ptrChunk = m_ptrFirstChunk;
		MemoryChunk *ptrChunkToDelete = NULL;

		while (ptrChunk)
		{
			if (ptrChunk->IsAllocationChunk)
			{
				if (ptrChunkToDelete)
				{
					free((void*)ptrChunkToDelete);
				}
				ptrChunkToDelete = ptrChunk;
			}
			ptrChunk = ptrChunk->Next;
		}
	}

	//
	//IsValidPointer
	//
	bool MemoryPool::IsValidPointer(void *ptrPointer)
	{
		MemoryChunk *ptrChunk = m_ptrFirstChunk;
		while (ptrChunk)
		{
			if (ptrChunk->Data == ((TByte*)ptrPointer))
			{
				return true;
			}
			ptrChunk = ptrChunk->Next;
		}
		return false;
	}

	//
	//MaxValue
	//
	std::size_t MemoryPool::MaxValue(const std::size_t &sValueA, const std::size_t &sValueB)const
	{
		if (sValueA > sValueB)
		{
			return sValueA;
		}
		return sValueB;
	}


}