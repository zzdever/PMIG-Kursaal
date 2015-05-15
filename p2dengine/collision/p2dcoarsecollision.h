#ifndef P2D_COARSE_COLLISION_H
#define P2D_COARSE_COLLISION_H

#include "../general/p2dparams.h"
#include "p2dcollision.h"
#include "p2dbtree.h"
#include <algorithm>

struct P2DPair
{
	int32 proxyIdA;
	int32 proxyIdB;
};

/// The coarse collision is used for computing pairs and performing volume queries and ray casts.
/// This coarse collision does not persist pairs. Instead, this reports potentially new pairs.
/// It is up to the client to consume the new pairs and to track subsequent overlap.
class P2DCoarseCollision
{
public:

	enum
	{
		e_nullProxy = -1
	};

	P2DCoarseCollision();
	~P2DCoarseCollision();

	/// Create a proxy with an initial AABB. Pairs are not reported until
	/// UpdatePairs is called.
	int32 CreateProxy(const P2DAABB& aabb, void* userData);

	/// Destroy a proxy. It is up to the client to remove any pairs.
	void DestroyProxy(int32 proxyId);

	/// Call MoveProxy as many times as you like, then when you are done
	/// call UpdatePairs to finalized the proxy pairs (for your time step).
	void MoveProxy(int32 proxyId, const P2DAABB& aabb, const P2DVec2& displacement);

	/// Call to trigger a re-processing of it's pairs on the next call to UpdatePairs.
	void TouchProxy(int32 proxyId);

	/// Get the fat AABB for a proxy.
	const P2DAABB& GetFatAABB(int32 proxyId) const;

	/// Get user data from a proxy. Returns NULL if the id is invalid.
	void* GetUserData(int32 proxyId) const;

	/// Test overlap of fat AABBs.
	bool TestOverlap(int32 proxyIdA, int32 proxyIdB) const;

	/// Get the number of proxies.
	int32 GetProxyCount() const;

	/// Update the pairs. This results in pair callbacks. This can only add pairs.
	template <typename T>
	void UpdatePairs(T* callback);

	/// Query an AABB for overlapping proxies. The callback class
	/// is called for each proxy that overlaps the supplied AABB.
	template <typename T>
	void Query(T* callback, const P2DAABB& aabb) const;

	/// Ray-cast against the proxies in the tree. This relies on the callback
	/// to perform a exact ray-cast in the case were the proxy contains a shape.
	/// The callback also performs the any collision filtering. This has performance
	/// roughly equal to k * log(n), where k is the number of collisions and n is the
	/// number of proxies in the tree.
	/// @param input the ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
	/// @param callback a callback class that is called for each proxy that is hit by the ray.
	template <typename T>
    void RayCast(T* callback, const P2DRayCastInput& input) const;

	/// Get the height of the embedded tree.
	int32 GetTreeHeight() const;

	/// Get the balance of the embedded tree.
	int32 GetTreeBalance() const;

	/// Get the quality metric of the embedded tree.
	float32 GetTreeQuality() const;

	/// Shift the world origin. Useful for large worlds.
	/// The shift formula is: position -= newOrigin
	/// @param newOrigin the new origin with respect to the old origin
	void ShiftOrigin(const P2DVec2& newOrigin);

private:

	friend class P2DBTree;

	void BufferMove(int32 proxyId);
	void UnBufferMove(int32 proxyId);

	bool QueryCallback(int32 proxyId);

	P2DBTree m_tree;

	int32 m_proxyCount;

	int32* m_moveBuffer;
	int32 m_moveCapacity;
	int32 m_moveCount;

    P2DPair* m_pairBuffer;
	int32 m_pairCapacity;
	int32 m_pairCount;

	int32 m_queryProxyId;
};

/// This is used to sort pairs.
inline bool P2DPairLessThan(const P2DPair& pair1, const P2DPair& pair2)
{
	if (pair1.proxyIdA < pair2.proxyIdA)
	{
		return true;
	}

	if (pair1.proxyIdA == pair2.proxyIdA)
	{
		return pair1.proxyIdB < pair2.proxyIdB;
	}

	return false;
}

inline void* P2DCoarseCollision::GetUserData(int32 proxyId) const
{
	return m_tree.GetUserData(proxyId);
}

inline bool P2DCoarseCollision::TestOverlap(int32 proxyIdA, int32 proxyIdB) const
{
	const P2DAABB& aabbA = m_tree.GetFatAABB(proxyIdA);
	const P2DAABB& aabbB = m_tree.GetFatAABB(proxyIdB);
	return P2DTestOverlap(aabbA, aabbB);
}

inline const P2DAABB& P2DCoarseCollision::GetFatAABB(int32 proxyId) const
{
	return m_tree.GetFatAABB(proxyId);
}

inline int32 P2DCoarseCollision::GetProxyCount() const
{
	return m_proxyCount;
}

inline int32 P2DCoarseCollision::GetTreeHeight() const
{
	return m_tree.GetHeight();
}

inline int32 P2DCoarseCollision::GetTreeBalance() const
{
	return m_tree.GetMaxBalance();
}

inline float32 P2DCoarseCollision::GetTreeQuality() const
{
	return m_tree.GetAreaRatio();
}

template <typename T>
void P2DCoarseCollision::UpdatePairs(T* callback)
{
	// Reset pair buffer
	m_pairCount = 0;

	// Perform tree queries for all moving proxies.
	for (int32 i = 0; i < m_moveCount; ++i)
	{
		m_queryProxyId = m_moveBuffer[i];
		if (m_queryProxyId == e_nullProxy)
		{
			continue;
		}

		// We have to query the tree with the fat AABB so that
		// we don't fail to create a pair that may touch later.
		const P2DAABB& fatAABB = m_tree.GetFatAABB(m_queryProxyId);

		// Query tree, create pairs and add them pair buffer.
		m_tree.Query(this, fatAABB);
	}

	// Reset move buffer
	m_moveCount = 0;

	// Sort the pair buffer to expose duplicates.
	std::sort(m_pairBuffer, m_pairBuffer + m_pairCount, P2DPairLessThan);

	// Send the pairs back to the client.
	int32 i = 0;
	while (i < m_pairCount)
	{
		P2DPair* primaryPair = m_pairBuffer + i;
		void* userDataA = m_tree.GetUserData(primaryPair->proxyIdA);
		void* userDataB = m_tree.GetUserData(primaryPair->proxyIdB);

		callback->AddPair(userDataA, userDataB);
		++i;

		// Skip any duplicate pairs.
		while (i < m_pairCount)
		{
			P2DPair* pair = m_pairBuffer + i;
			if (pair->proxyIdA != primaryPair->proxyIdA || pair->proxyIdB != primaryPair->proxyIdB)
			{
				break;
			}
			++i;
		}
	}

	// Try to keep the tree balanced.
	//m_tree.Rebalance(4);
}

template <typename T>
inline void P2DCoarseCollision::Query(T* callback, const P2DAABB& aabb) const
{
	m_tree.Query(callback, aabb);
}

template <typename T>
inline void P2DCoarseCollision::RayCast(T* callback, const P2DRayCastInput& input) const
{
	m_tree.RayCast(callback, input);
}

inline void P2DCoarseCollision::ShiftOrigin(const P2DVec2& newOrigin)
{
	m_tree.ShiftOrigin(newOrigin);
}

#endif
