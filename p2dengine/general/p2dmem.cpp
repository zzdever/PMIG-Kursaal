#include "p2dmem.h"
#include "p2dmath.h"

// Memory allocators for common use.
void* MemAlloc(int32 size)
{
	return malloc(size);
}

void MemFree(void* mem)
{
	free(mem);
}

// =======STACK=MEM=======
P2DStackMem::P2DStackMem()
{
	m_index = 0;
	m_allocation = 0;
	m_maxAllocation = 0;
	m_entryCount = 0;
}

P2DStackMem::~P2DStackMem()
{
	assert(m_index == 0);
	assert(m_entryCount == 0);
}

void* P2DStackMem::Allocate(int32 size)
{
	assert(m_entryCount < MAX_STACK_ENTRIES);

	P2DStackEntry* entry = m_entries + m_entryCount;
	entry->size = size;
	if (m_index + size > STACK_SIZE) {
		entry->data = (char*)MemAlloc(size);
		entry->usedMalloc = true;
	} else {
		entry->data = m_data + m_index;
		entry->usedMalloc = false;
		m_index += size;
	}

	m_allocation += size;
	m_maxAllocation = P2DMax(m_maxAllocation, m_allocation);
	++m_entryCount;

	return entry->data;
}

void P2DStackMem::Free(void* p)
{
	assert(m_entryCount > 0);
	P2DStackEntry* entry = m_entries + m_entryCount - 1;
	assert(p == entry->data);
	if (entry->usedMalloc){
		MemFree(p);
	} else {
		m_index -= entry->size;
	}
	m_allocation -= entry->size;
	--m_entryCount;

	p = NULL;
}

int32 P2DStackMem::GetMaxAllocation() const
{
	return m_maxAllocation;
}


// =======BLOCK=MEM=======
#include <limits.h>
#include <string.h>
#include <stddef.h>

int32 P2DBlockMem::s_blockSizes[BLOCK_SIZES] = 
{
	16,		// 0
	32,		// 1
	64,		// 2
	96,		// 3
	128,	// 4
	160,	// 5
	192,	// 6
	224,	// 7
	256,	// 8
	320,	// 9
	384,	// 10
	448,	// 11
	512,	// 12
	640,	// 13
};
uint8 P2DBlockMem::s_blockSizeLookup[MAX_BLOCK_SIZE + 1];
bool P2DBlockMem::s_blockSizeLookupInitialized;


P2DBlockMem::P2DBlockMem()
{
    assert(BLOCK_SIZES < UCHAR_MAX);

	m_chunkSpace = CHUNK_ARRAY_INCREMENT;
	m_chunkCount = 0;
	m_chunks = (P2DChunk*)MemAlloc(m_chunkSpace * sizeof(P2DChunk));
	
	memset(m_chunks, 0, m_chunkSpace * sizeof(P2DChunk));
	memset(m_freeLists, 0, sizeof(m_freeLists));

	if (s_blockSizeLookupInitialized == false)
	{
		int32 j = 0;
		for (int32 i = 1; i <= MAX_BLOCK_SIZE; ++i)
		{
			assert(j < BLOCK_SIZES);
			if (i <= s_blockSizes[j])
			{
				s_blockSizeLookup[i] = (uint8)j;
			}
			else
			{
				++j;
				s_blockSizeLookup[i] = (uint8)j;
			}
		}

		s_blockSizeLookupInitialized = true;
	}
}

P2DBlockMem::~P2DBlockMem()
{
	for (int32 i = 0; i < m_chunkCount; ++i)
	{
		MemFree(m_chunks[i].blocks);
	}

	MemFree(m_chunks);
}

void* P2DBlockMem::Allocate(int32 size)
{
	if (size == 0)
		return NULL;

	assert(0 < size);

	if (size > MAX_BLOCK_SIZE)
	{
		return MemAlloc(size);
	}

	int32 index = s_blockSizeLookup[size];
	assert(0 <= index && index < BLOCK_SIZES);

	if (m_freeLists[index])
	{
		P2DBlock* block = m_freeLists[index];
		m_freeLists[index] = block->next;
		return block;
	}
	else
	{
		if (m_chunkCount == m_chunkSpace)
		{
			P2DChunk* oldChunks = m_chunks;
			m_chunkSpace += CHUNK_ARRAY_INCREMENT;
			m_chunks = (P2DChunk*)MemAlloc(m_chunkSpace * sizeof(P2DChunk));
			memcpy(m_chunks, oldChunks, m_chunkCount * sizeof(P2DChunk));
			memset(m_chunks + m_chunkCount, 0, CHUNK_ARRAY_INCREMENT * sizeof(P2DChunk));
			MemFree(oldChunks);
		}

		P2DChunk* chunk = m_chunks + m_chunkCount;
		chunk->blocks = (P2DBlock*)MemAlloc(CHUNK_SIZE);
#if defined(_DEBUG)
		memset(chunk->blocks, 0xcd, CHUNK_SIZE);
#endif
		int32 blockSize = s_blockSizes[index];
		chunk->blockSize = blockSize;
		int32 blockCount = CHUNK_SIZE / blockSize;
		assert(blockCount * blockSize <= CHUNK_SIZE);
		for (int32 i = 0; i < blockCount - 1; ++i)
		{
			P2DBlock* block = (P2DBlock*)((int8*)chunk->blocks + blockSize * i);
			P2DBlock* next = (P2DBlock*)((int8*)chunk->blocks + blockSize * (i + 1));
			block->next = next;
		}
		P2DBlock* last = (P2DBlock*)((int8*)chunk->blocks + blockSize * (blockCount - 1));
		last->next = NULL;

		m_freeLists[index] = chunk->blocks->next;
		++m_chunkCount;

		return chunk->blocks;
	}
}

void P2DBlockMem::Free(void* p, int32 size)
{
	if (size == 0)
	{
		return;
	}

	assert(0 < size);

	if (size > MAX_BLOCK_SIZE)
	{
		MemFree(p);
		return;
	}

	int32 index = s_blockSizeLookup[size];
	assert(0 <= index && index < BLOCK_SIZES);

#ifdef _DEBUG
	// Verify the memory address and size is valid.
	int32 blockSize = s_blockSizes[index];
	bool found = false;
	for (int32 i = 0; i < m_chunkCount; ++i)
	{
		P2DChunk* chunk = m_chunks + i;
		if (chunk->blockSize != blockSize)
		{
			assert(	(int8*)p + blockSize <= (int8*)chunk->blocks ||
						(int8*)chunk->blocks + CHUNK_SIZE <= (int8*)p);
		}
		else
		{
			if ((int8*)chunk->blocks <= (int8*)p && (int8*)p + blockSize <= (int8*)chunk->blocks + CHUNK_SIZE)
			{
				found = true;
			}
		}
	}

	assert(found);

	memset(p, 0xfd, blockSize);
#endif

	P2DBlock* block = (P2DBlock*)p;
	block->next = m_freeLists[index];
	m_freeLists[index] = block;
}

void P2DBlockMem::Clear()
{
	for (int32 i = 0; i < m_chunkCount; ++i)
	{
		MemFree(m_chunks[i].blocks);
	}

	m_chunkCount = 0;
	memset(m_chunks, 0, m_chunkSpace * sizeof(P2DChunk));

	memset(m_freeLists, 0, sizeof(m_freeLists));
}

