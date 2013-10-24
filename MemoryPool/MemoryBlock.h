//
//MemoryBlock.h
//
//MemoryBlock.h contains the MemoryBlock class defintion, and it's the abstract 
//interface for the actual MemoryPool class 
//

#ifndef	_MEMORYBLOCK_H
#define _MEMORYBLOCK_H

#include "HeaderFiles.h"

namespace MemoryPool
{
	typedef unsigned char TByte;		//brief Byte = 8 bit

	//Abstract Base-Class interface for the memory pool 
	//Introduces basic operations like geting/freeing memory
	class MemoryBlock
	{
	public:
		virtual ~MemoryBlock() {};

		virtual void* GetMemory(const std::size_t &sMemorySize) = 0;
		virtual void FreeMemory(void *ptrMemoryBlock, const std::size_t &sMemoryBlockSize) = 0;
	};
}


#endif//_MEMORYBLOCK_H
