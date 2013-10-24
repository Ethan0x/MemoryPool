//
//MemoryPool.h
//

#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include "MemoryBlock.h"
#include "MemoryChunk.h"

namespace MemoryPool
{
	static const std::size_t DEFAULT_MEMORY_POOL_SIZE = 1000;									//Initial MemoryPool size in bytes
	static const std::size_t DEFAULT_MEMORY_CHUNK_SIZE = 128;									//Default MemoryChunk size in bytes
	static const std::size_t DEFAYLT_MEMORY_SIZE_TO_ALLOCATE = DEFAULT_MEMORY_CHUNK_SIZE * 2;	//Default minimal memory size to allocate

	//class MemoryPool
	//This class responsible for all MemoryRequests (GetMemory() / FreeMemory()) and manages the allocation of Memory from Operating-System

	class MemoryPool : public MemoryBlock
	{
	public :
		//Contructor Param:
		//sInitialMemoryPoolSize:		The Initial Size (in Bytes) of the Memory Pool
		//sMemoryChunkSize:				The Size (in Bytes) each MemoryChunk can Manage. A low sMemoryChunkSize increases the MemoryPool runtime(bad), 
		//								but decreases the Memory - overhead / fragmentation(good)
		//sMinimalMemorySizeToAllocate:	The Minimal amount of Memory which is allocated (in Bytes).That means, every time you have to allocate more Memory from the OS,at least 
		//								sMinimalMemorySizeToAllocate Bytes are allocated.When you have to request small amount of Memory very often, this will speed up the MemoryPool, 
		//								beacause when you allocate a new Memory from the OS, you will allocate a small "Buffer" automatically, which will prevent you from requesting OS - memory too often.
		//bSetMemoryData :				Set to true, if you want to set all allocated/freed Memory to a specific Value.Very usefull for debugging, but has a negativ impact on the runtime.
		
		MemoryPool(const std::size_t &sInitialMemoryPoolSize = DEFAULT_MEMORY_POOL_SIZE,
			const std::size_t &sMemoryChunkSize = DEFAULT_MEMORY_CHUNK_SIZE,
			const std::size_t &sMinimalMemorySizeToAllocate = DEFAYLT_MEMORY_SIZE_TO_ALLOCATE,
			bool bSetMemoryData = false);
		
		//Destructor
		virtual ~MemoryPool();

		//GetMemory :				Get "sMemorySize" Bytes from the Memory Pool.
		//<param> sMemorySize :		Sizes (in Bytes) of Memory.
		//<Return> :				Pointer to a Memory-Block of "sMemorySize" Bytes, or NULL if an error occured. 
		virtual void *GetMemory(const std::size_t &sMemorySize);
		
		//FreeMemory :				Free the allocated memory again!
		//<param> ptrMemoryBlock :	Pointer to a Block of Memory, which is to be freed (previoulsy allocated via "GetMemory()").
		//<param> sMemorySize :		Sizes (in Bytes) of Memory.
		virtual void FreeMemory(void *ptrMemoryBlock, const std::size_t &sMemoryBlockSize);
		
		//WriteMemoryDumpToFile :	Writes the contents of the MemoryPool to a File. (Note! This file can be quite large ,several MB).
		//<param> strFileName :		FileName of the MemoryDump.
		//<Return> :				true on success, false otherwise 
		bool WriteMemoryDumpToFile(const std::string &strFileName);

		//IsValidPointer :			Check, if a Pointer is in the Memory-Pool.(Note! This Checks only if a pointer is inside the Memory-Pool, and not if the Memory contains meaningfull data.)
		//<param> ptrPointer :		Pointer to a Memory-Block which is to be checked.
		//<Return> :				true, if the Pointer could be found in the Memory-Pool, false otherwise.
		bool IsValidPointer(void* ptrPointer);

	private:
		//Allocatememory :			Will Allocate "sMemorySize" Bytes of Memory from the OS. The Memory will be cut into Pieces and Managed by the MemoryChunk-Linked-List.(See LinkChunksToData() for details)
		//<param> sMemorySize :		The Memory-Size (in Bytes) to allocate
		//<Return> :				true, if the Memory could be allocated, false otherwise (e.g. System is out of Memory, etc.)
		bool AllocateMemory(const std::size_t &sMemorySize);
		void FreeAllAllocatedMemory();		//Free all allocated memory to the OS.
		
		unsigned int CalculateNeededChunks(const std::size_t &sMemorySize);	//return the Number of MemoryChunks needed to Manage "sMemorySize" Bytes.
		std::size_t CalculateBestMemoryBlockSize(const std::size_t &sRequestedMemoryBlockSize);	//return the amount of Memory which is best Managed by the MemoryChunks.

		MemoryChunk *FindChunkSuitableToHoldMemory(const std::size_t &sMemorySize);	//return a Chunk which can hold the requested amount of memory, or NULL, if none was found.
		MemoryChunk *FindChunkHoldingPointerTo(void *ptrMemoryBlock);	//Find a Chunk which "Data"-Member is Pointing to the given "ptrMemoryBlock", or NULL if none was found.
		MemoryChunk *SkipChunks(MemoryChunk *ptrStartChunk, unsigned int uiChunksToSkip);	//Skip the given amount of Chunks, starting from the given ptrStartChunk. \return the Chunk at the "skipping" - Position.
		MemoryChunk *SetChunkDefaults(MemoryChunk *ptrChunk);	//Set "Default"-Values to the given Chunk
		
		void FreeChunks(MemoryChunk *ptrChunk);	//Makes the memory linked to the given Chunk available in the MemoryPool again (by setting the "UsedSize"-Member to 0).
		void DeallocateAllChunks();	//Deallocates all Memory needed by the Chunks back to the OS.
		bool LinkChunksToData(MemoryChunk *ptrNewChunk, unsigned int uiChunkCount, TByte *ptrNewMemBlock);	//Link the given Memory-Block to the Linked-List of MemoryChunks...
		void SetMemoryChunkValues(MemoryChunk *ptrChunk, const std::size_t &sMemBlockSize);	//Set the "UsedSize"-Member of the given "ptrChunk" to "sMemBlockSize".
		bool RecalcChunkMemorySize(MemoryChunk *ptrChunks, unsigned int uiChunkCount);	//Recalcs the "DataSize" - Member of all Chunks whe the Memory - Pool grows(via "AllocateMemory()")
		
		std::size_t MaxValue(const std::size_t &sValueA, const std::size_t &sValueB) const;	//return the greatest of the two input values (A or B)

		MemoryChunk *m_ptrFirstChunk;		//Pointer to the first Chunk in the Linked-List of Memory Chunks
		MemoryChunk *m_ptrLastChunk;		//Pointer to the last Chunk in the Linked-List of Memory Chunks
		MemoryChunk *m_ptrCursorChunk;		//Cursor-Chunk. Used to speed up the navigation in the linked-List.

		std::size_t m_sTotalMemoryPoolSize;	//Total Memory-Pool size in Bytes
		std::size_t m_sUsedMemoryPoolSize;  //amount of used Memory in Bytes
		std::size_t m_sFreeMemoryPoolSize;  //amount of free Memory in Bytes

		std::size_t m_sMemoryChunkSize;     //amount of Memory which can be Managed by a single MemoryChunk.
		unsigned int m_uiMemoryChunkCount;  //Total amount of "SMemoryChunk"-Objects in the Memory-Pool.
		unsigned int m_uiObjectCount;       //Counter for "GetMemory()" / "FreeMemory()"-Operation. Counts (indirectly) the number of "Objects" inside the mem-Pool.

		bool m_bSetMemoryData;                      //Set to "true", if you want to set all (de)allocated Memory to a predefined Value (via "memset()"). Usefull for debugging.
		std::size_t m_sMinimalMemorySizeToAllocate; //The minimal amount of Memory which can be allocated via "AllocateMemory()".
	};
}
#endif	//_MEMORYPOOL_H