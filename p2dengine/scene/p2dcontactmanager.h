#ifndef P2D_CONTACT_MANAGER_H
#define P2D_CONTACT_MANAGER_H

#include "../collision/p2dcoarsecollision.h"

class P2DContact;
class P2DContactFilter;
class P2DContactListener;
class P2DBlockMem;

class P2DContactManager
{
public:
	P2DContactManager();

	// Broad-phase callback.
	void AddPair(void* proxyUserDataA, void* proxyUserDataB);

	void FindNewContacts();

	void Destroy(P2DContact* c);

	void Collide();
            
	// TODO broad phase mod		
	P2DCoarseCollision m_broadPhase;
	P2DContact* m_contactList;
	int32 m_contactCount;
	P2DContactFilter* m_contactFilter;
	P2DContactListener* m_contactListener;
	P2DBlockMem* m_allocator;
};

#endif
