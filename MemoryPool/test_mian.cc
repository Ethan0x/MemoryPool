//
//test_main.cc
//
//Test Program Main

#include "HeaderFiles.h"
#include "MemoryPool.h"

#include <windows.h>
#include <time.h>

MemoryPool::MemoryPool *g_ptrMemPool = NULL;	//Global MemoryPool
unsigned int TestCount = 500000;				//allocations 
unsigned int ArraySize = 10000;					//size of the test array

class TestClass_OverLoad
{
public:
	TestClass_OverLoad()
	{
		m_cMyArray[0] = 'H';
		m_cMyArray[1] = 'e';
		m_cMyArray[2] = 'l';
		m_cMyArray[3] = 'l';
		m_cMyArray[4] = 'o';
		m_cMyArray[5] = NULL;
		m_strMyString = "This is a small Test-String";
		m_iMyInt = 12345;

		m_fFloatValue = 23456.7890f;
		m_fDoubleValue = 6789.012345;

		Next = this;
	}

	virtual ~TestClass_OverLoad() {};

	void *operator new(std::size_t ObjectSize)
	{
		return g_ptrMemPool->GetMemory(ObjectSize);
	}

	void operator delete(void *ptrObject, std::size_t ObjectSize)
	{
		g_ptrMemPool->FreeMemory(ptrObject, ObjectSize);
	}

private:
	char m_cMyArray[25];
	unsigned char m_BigArray[10000];
	std::string m_strMyString;
	int m_iMyInt;
	TestClass_OverLoad *Next;
	float m_fFloatValue;
	double m_fDoubleValue;
};

class TestClass
{
public:
	TestClass()
	{
		m_cMyArray[0] = 'H';
		m_cMyArray[1] = 'e';
		m_cMyArray[2] = 'l';
		m_cMyArray[3] = 'l';
		m_cMyArray[4] = 'o';
		m_cMyArray[5] = NULL;
		m_strMyString = "This is a small Test-String";
		m_iMyInt = 12345;

		m_fFloatValue = 23456.7890f;
		m_fDoubleValue = 6789.012345;

		Next = this;
	}

	virtual ~TestClass() {};

private:
	char m_cMyArray[25];
	unsigned char m_BigArray[10000];
	std::string m_strMyString;
	int m_iMyInt;
	TestClass *Next;
	float m_fFloatValue;
	double m_fDoubleValue;
};

//
//CreateGlobalMemPool
//
void CreateGlobalMemPool()
{
	std::cerr << "Creating MemoryPool....";
	g_ptrMemPool = new MemoryPool::MemoryPool();
	std::cerr << "OK" << std::endl;
}

//
//DestroyGlobalMemPool
//
void DestroyGlobalMemPool()
{
	std::cerr << "Deleting MemPool....";
	if (g_ptrMemPool) delete g_ptrMemPool;
	std::cerr << "OK" << std::endl;
}

//
//TestAllocationSpeedClassMemPool
//
void TestAllocationSpeedClassMemPool()
{
	std::cerr << "Allocating Memory (Object Size : " << sizeof(TestClass_OverLoad) << ")...";
	clock_t start, finish;
	double totaltime;
	start = clock();
	for (unsigned int j = 0; j < TestCount; j++)
	{
		TestClass_OverLoad *ptrTestClass = new TestClass_OverLoad;
		delete ptrTestClass;
	}
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	
	std::cerr << "OK" << std::endl;

	std::cerr << "Result for MemPool(Class Test) : " << totaltime << " s" << std::endl;
}

//
//TestAllocationSpeedClassHeap
//
void TestAllocationSpeedClassHeap()
{
	std::cerr << "Allocating Memory (Object Size : " << sizeof(TestClass) << ")...";
	clock_t start, finish;
	double totaltime;
	start = clock();
	for (unsigned int j = 0; j < TestCount; j++)
	{
		TestClass *ptrTestClass = new TestClass;
		delete ptrTestClass;
	}
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cerr << "OK" << std::endl;

	std::cerr << "Result for Heap(Class Test)    : " << totaltime << " s" << std::endl;
}

//
//TestAllocationSpeedArrayMemPool
//
void TestAllocationSpeedArrayMemPool()
{
	std::cerr << "Allocating Memory (Object Size : " << ArraySize << ")...";
	clock_t start, finish;
	double totaltime;
	start = clock();
	for (unsigned int j = 0; j < TestCount; j++)
	{
		char *ptrArray = (char *)g_ptrMemPool->GetMemory(ArraySize);
		g_ptrMemPool->FreeMemory(ptrArray, ArraySize);
	}
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cerr << "OK" << std::endl;

	std::cerr << "Result for MemPool(Array-Test) : " << totaltime << " ms" << std::endl;
}

//
//TestAllocationSpeedArrayHeap
//
void TestAllocationSpeedArrayHeap()
{
	std::cerr << "Allocating Memory (Object Size : " << ArraySize << ")...";
	clock_t start, finish;
	double totaltime;
	start = clock();
	for (unsigned int j = 0; j < TestCount; j++)
	{
		char *ptrArray = (char *)malloc(ArraySize);
		free(ptrArray);
	}
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cerr << "OK" << std::endl;

	std::cerr << "Result for Heap(Array-Test)    : " << totaltime << " ms" << std::endl;
}

//
//WriteMemoryDumpToFile
//
void WriteMemoryDumpToFile()
{
	std::cerr << "Writing MemoryDump to File...";
	g_ptrMemPool->WriteMemoryDumpToFile("MemoryDump.bin");
	std::cerr << "OK" << std::endl;
}

//
//main
//

int main(int argc, const char *argv[])
{
	std::cout << "MemoryPool Program started..." << std::endl;
	CreateGlobalMemPool();

	TestAllocationSpeedArrayMemPool();
	TestAllocationSpeedArrayHeap();

	TestAllocationSpeedClassMemPool();
	TestAllocationSpeedClassHeap();

	WriteMemoryDumpToFile();

	DestroyGlobalMemPool();

	std::cout << "MemoryPool Program finished..." << std::endl;
	system("PAUSE");
	return 0;
}