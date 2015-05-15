#include "p2dcoarsecollision.h"

P2DCoarseCollision::P2DCoarseCollision()
{
	m_proxyCount = 0;

	m_pairCapacity = 16;
	m_pairCount = 0;
	m_pairBuffer = (P2DPair*)MemAlloc(m_pairCapacity * sizeof(P2DPair));

	m_moveCapacity = 16;
	m_moveCount = 0;
	m_moveBuffer = (int32*)MemAlloc(m_moveCapacity * sizeof(int32));
}

P2DCoarseCollision::~P2DCoarseCollision()
{
	MemFree(m_moveBuffer);
	MemFree(m_pairBuffer);
}

int32 P2DCoarseCollision::CreateProxy(const P2DAABB& aabb, void* userData)
{
	int32 proxyId = m_tree.CreateProxy(aabb, userData);
	++m_proxyCount;
	BufferMove(proxyId);
	return proxyId;
}

void P2DCoarseCollision::DestroyProxy(int32 proxyId)
{
	UnBufferMove(proxyId);
	--m_proxyCount;
	m_tree.DestroyProxy(proxyId);
}

void P2DCoarseCollision::MoveProxy(int32 proxyId, const P2DAABB& aabb, const P2DVec2& displacement)
{
	bool buffer = m_tree.MoveProxy(proxyId, aabb, displacement);
	if (buffer)
	{
		BufferMove(proxyId);
	}
}

void P2DCoarseCollision::TouchProxy(int32 proxyId)
{
	BufferMove(proxyId);
}

void P2DCoarseCollision::BufferMove(int32 proxyId)
{
	if (m_moveCount == m_moveCapacity)
	{
		int32* oldBuffer = m_moveBuffer;
		m_moveCapacity *= 2;
		m_moveBuffer = (int32*)MemAlloc(m_moveCapacity * sizeof(int32));
		memcpy(m_moveBuffer, oldBuffer, m_moveCount * sizeof(int32));
		MemFree(oldBuffer);
	}

	m_moveBuffer[m_moveCount] = proxyId;
	++m_moveCount;
}

void P2DCoarseCollision::UnBufferMove(int32 proxyId)
{
	for (int32 i = 0; i < m_moveCount; ++i)
	{
		if (m_moveBuffer[i] == proxyId)
		{
			m_moveBuffer[i] = e_nullProxy;
		}
	}
}

// This is called from P2DCoarseCollision::Query when we are gathering pairs.
bool P2DCoarseCollision::QueryCallback(int32 proxyId)
{
	// A proxy cannot form a pair with itself.
	if (proxyId == m_queryProxyId)
	{
		return true;
	}

	// Grow the pair buffer as needed.
	if (m_pairCount == m_pairCapacity)
	{
		P2DPair* oldBuffer = m_pairBuffer;
		m_pairCapacity *= 2;
        m_pairBuffer = (P2DPair*)MemAlloc(m_pairCapacity * sizeof(P2DPair));
        memcpy(m_pairBuffer, oldBuffer, m_pairCount * sizeof(P2DPair));
		MemFree(oldBuffer);
	}

	m_pairBuffer[m_pairCount].proxyIdA = P2DMin(proxyId, m_queryProxyId);
	m_pairBuffer[m_pairCount].proxyIdB = P2DMax(proxyId, m_queryProxyId);
	++m_pairCount;

	return true;
}
