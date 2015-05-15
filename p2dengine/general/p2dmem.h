#ifndef P2D_MEM_H
#define P2D_MEM_H

#include "p2dparams.h"
#include <memory.h>

const int32 STACK_SIZE = 100 * 1024;
const int32 MAX_STACK_ENTRIES = 32;

void* MemAlloc(int32 size);
void MemFree(void* mem);


// =======STACK=MEM=======
struct P2DStackEntry
{
	char* data;
	int32 size;
	bool usedMalloc;
};

// This is a stack allocator used for fast per step allocations.
// You must nest allocate/free pairs. The code will assert
// if you try to interleave multiple allocate/free pairs.
class P2DStackMem
{
public:
	P2DStackMem();
	~P2DStackMem();

	void* Allocate(int32 size);
	void Free(void* p);

	int32 GetMaxAllocation() const;

private:

	char m_data[STACK_SIZE];
	int32 m_index;

	int32 m_allocation;
	int32 m_maxAllocation;

	P2DStackEntry m_entries[MAX_STACK_ENTRIES];
	int32 m_entryCount;
};


// =======BLOCK=MEM=======
const int32 CHUNK_SIZE = 16 * 1024;
const int32 MAX_BLOCK_SIZE = 640;
const int32 BLOCK_SIZES = 14;
const int32 CHUNK_ARRAY_INCREMENT = 128;

struct P2DBlock
{
	P2DBlock* next;
};

struct P2DChunk
{
	int32 blockSize;
	P2DBlock* blocks;
};



/// This is a small object allocator used for allocating small
/// objects that persist for more than one time step.
/// See: http://www.codeproject.com/useritems/Small_Block_Allocator.asp
class P2DBlockMem
{
public:
	P2DBlockMem();
	~P2DBlockMem();

	/// Allocate memory. This will use MemAlloc if the size is larger than MAX_BLOCK_SIZE.
	void* Allocate(int32 size);

	/// Free memory. This will use MemFree if the size is larger than MAX_BLOCK_SIZE.
	void Free(void* p, int32 size);

	void Clear();

private:

	P2DChunk* m_chunks;
	int32 m_chunkCount;
	int32 m_chunkSpace;

	P2DBlock* m_freeLists[BLOCK_SIZES];

	static int32 s_blockSizes[BLOCK_SIZES];
	static uint8 s_blockSizeLookup[MAX_BLOCK_SIZE + 1];
	static bool s_blockSizeLookupInitialized;
};


// =======GROWABLE=STACK=======
/// This is a growable LIFO stack with an initial capacity of N.
/// If the stack size exceeds the initial capacity, the heap is used
/// to increase the size of the stack.
template <typename T, int32 N>
class P2DGrowableStack
{
public:
	P2DGrowableStack()
	{
		m_stack = m_array;
		m_count = 0;
		m_capacity = N;
	}

	~P2DGrowableStack()
	{
		if (m_stack != m_array)
		{
			MemFree(m_stack);
			m_stack = NULL;
		}
	}

	void Push(const T& element)
	{
		if (m_count == m_capacity)
		{
			T* old = m_stack;
			m_capacity *= 2;
			m_stack = (T*)MemAlloc(m_capacity * sizeof(T));
			memcpy(m_stack, old, m_count * sizeof(T));
			if (old != m_array)
			{
				MemFree(old);
			}
		}

		m_stack[m_count] = element;
		++m_count;
	}

	T Pop()
	{
		assert(m_count > 0);
		--m_count;
		return m_stack[m_count];
	}

	int32 GetCount()
	{
		return m_count;
	}

private:
	T* m_stack;
	T m_array[N];
	int32 m_count;
	int32 m_capacity;
};


#endif


