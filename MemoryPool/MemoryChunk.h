//
//MemoryChunk.h
//
//Contains the MemoryChunk definition
//It hold and manage the actual allocated memory, every momerychunk will point 
//to a momoryblock, and other memorychunk creating a linked list od memorychunks
//

#ifndef _MEMORYCHUNK_H
#define _MEMORYCHUNK_H

#include "MemoryBlock.h"

namespace MemoryPool
{

	typedef struct MemoryChunk
	{
		TByte *Data;			//The actual data
		std::size_t DataSize;	//size of the "data" block
		std::size_t UsedSize;	//actual used size
		bool IsAllocationChunk;	//True:when this MemoryChunk points to a data block,which can be deallocated via free();
		MemoryChunk *Next;		//pointer to the next Memorychunk in the list, may be NULL
	}MemoryChunk;
}

#endif //_MEMORYCHUNK_H