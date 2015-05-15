#ifndef P2D_BTREE_H
#define P2D_BTREE_H

#include "p2dcollision.h"
#include "../general/p2dmem.h"

#define NULL_NODE (-1)

/// A node in the dynamic tree. The client does not interact with this directly.
struct P2DBTreeNode
{
	bool IsLeaf() const
	{
		return child1 == NULL_NODE;
	}

	/// Enlarged AABB
	P2DAABB aabb;

	void* userData;

	union
	{
		int32 parent;
		int32 next;
	};

	int32 child1;
	int32 child2;

	// leaf = 0, free node = -1
	int32 height;
};

/// A dynamic AABB tree broad-phase, inspired by Nathanael Presson's btDbvt.
/// A dynamic tree arranges data in a binary tree to accelerate
/// queries such as volume queries and ray casts. Leafs are proxies
/// with an AABB. In the tree we expand the proxy AABB by P2D_fatAABBFactor
/// so that the proxy AABB is bigger than the client object. This allows the client
/// object to move by small amounts without triggering a tree update.
///
/// Nodes are pooled and relocatable, so we use node indices rather than pointers.
class P2DBTree
{
public:
	/// Constructing the tree initializes the node pool.
	P2DBTree();

	/// Destroy the tree, freeing the node pool.
	~P2DBTree();

	/// Create a proxy. Provide a tight fitting AABB and a userData pointer.
	int32 CreateProxy(const P2DAABB& aabb, void* userData);

	/// Destroy a proxy. This asserts if the id is invalid.
	void DestroyProxy(int32 proxyId);

	/// Move a proxy with a swepted AABB. If the proxy has moved outside of its fattened AABB,
	/// then the proxy is removed from the tree and re-inserted. Otherwise
	/// the function returns immediately.
	/// @return true if the proxy was re-inserted.
	bool MoveProxy(int32 proxyId, const P2DAABB& aabb1, const P2DVec2& displacement);

	/// Get proxy user data.
	/// @return the proxy user data or 0 if the id is invalid.
	void* GetUserData(int32 proxyId) const;

	/// Get the fat AABB for a proxy.
	const P2DAABB& GetFatAABB(int32 proxyId) const;

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

	/// Validate this tree. For testing.
	void Validate() const;

	/// Compute the height of the binary tree in O(N) time. Should not be
	/// called often.
	int32 GetHeight() const;

	/// Get the maximum balance of an node in the tree. The balance is the difference
	/// in height of the two children of a node.
	int32 GetMaxBalance() const;

	/// Get the ratio of the sum of the node areas to the root area.
	float32 GetAreaRatio() const;

	/// Build an optimal tree. Very expensive. For testing.
	void RebuildBottomUp();

	/// Shift the world origin. Useful for large worlds.
	/// The shift formula is: position -= newOrigin
	/// @param newOrigin the new origin with respect to the old origin
	void ShiftOrigin(const P2DVec2& newOrigin);

private:

	int32 AllocateNode();
	void FreeNode(int32 node);

	void InsertLeaf(int32 node);
	void RemoveLeaf(int32 node);

	int32 Balance(int32 index);

	int32 ComputeHeight() const;
	int32 ComputeHeight(int32 nodeId) const;

	void ValidateStructure(int32 index) const;
	void ValidateMetrics(int32 index) const;

	int32 m_root;

	P2DBTreeNode* m_nodes;
	int32 m_nodeCount;
	int32 m_nodeCapacity;

	int32 m_freeList;

	/// This is used to incrementally traverse the tree for re-balancing.
	uint32 m_path;

	int32 m_insertionCount;
};

inline void* P2DBTree::GetUserData(int32 proxyId) const
{
	assert(0 <= proxyId && proxyId < m_nodeCapacity);
	return m_nodes[proxyId].userData;
}

inline const P2DAABB& P2DBTree::GetFatAABB(int32 proxyId) const
{
	assert(0 <= proxyId && proxyId < m_nodeCapacity);
	return m_nodes[proxyId].aabb;
}

template <typename T>
inline void P2DBTree::Query(T* callback, const P2DAABB& aabb) const
{
	P2DGrowableStack<int32, 256> stack;
	stack.Push(m_root);

	while (stack.GetCount() > 0)
	{
		int32 nodeId = stack.Pop();
		if (nodeId == NULL_NODE)
		{
			continue;
		}

		const P2DBTreeNode* node = m_nodes + nodeId;

		if (P2DTestOverlap(node->aabb, aabb))
		{
			if (node->IsLeaf())
			{
				bool proceed = callback->QueryCallback(nodeId);
				if (proceed == false)
				{
					return;
				}
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}
}

template <typename T>
inline void P2DBTree::RayCast(T* callback, const P2DRayCastInput &input) const
{
	P2DVec2 p1 = input.p1;
	P2DVec2 p2 = input.p2;
	P2DVec2 r = p2 - p1;
	assert(r.LengthSquared() > 0.0f);
	r.Normalize();

	// v is perpendicular to the segment.
	P2DVec2 v = P2DVecCross(1.0f, r);
	P2DVec2 abs_v = P2DAbs(v);

	// Separating axis for segment (Gino, p80).
	// |dot(v, p1 - c)| > dot(|v|, h)

	float32 maxFraction = input.maxFraction;

	// Build a bounding box for the segment.
	P2DAABB segmentAABB;
	{
		P2DVec2 t = p1 + maxFraction * (p2 - p1);
		segmentAABB.lowerBound = P2DMin(p1, t);
		segmentAABB.upperBound = P2DMax(p1, t);
	}

	P2DGrowableStack<int32, 256> stack;
	stack.Push(m_root);

	while (stack.GetCount() > 0)
	{
		int32 nodeId = stack.Pop();
		if (nodeId == NULL_NODE)
		{
			continue;
		}

		const P2DBTreeNode* node = m_nodes + nodeId;

		if (P2DTestOverlap(node->aabb, segmentAABB) == false)
		{
			continue;
		}

		// Separating axis for segment (Gino, p80).
		// |dot(v, p1 - c)| > dot(|v|, h)
		P2DVec2 c = node->aabb.GetCenter();
		P2DVec2 h = node->aabb.GetExtents();
		float32 separation = P2DAbs(P2DVecDot(v, p1 - c)) - P2DVecDot(abs_v, h);
		if (separation > 0.0f)
		{
			continue;
		}

		if (node->IsLeaf())
		{
            P2DRayCastInput subInput;
			subInput.p1 = input.p1;
			subInput.p2 = input.p2;
			subInput.maxFraction = maxFraction;

			float32 value = callback->RayCastCallback(subInput, nodeId);

			if (value == 0.0f)
			{
				// The client has terminated the ray cast.
				return;
			}

			if (value > 0.0f)
			{
				// Update segment bounding box.
				maxFraction = value;
				P2DVec2 t = p1 + maxFraction * (p2 - p1);
				segmentAABB.lowerBound = P2DMin(p1, t);
				segmentAABB.upperBound = P2DMax(p1, t);
			}
		}
		else
		{
			stack.Push(node->child1);
			stack.Push(node->child2);
		}
	}
}

#endif
