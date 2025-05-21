#pragma once

#include "baseTypes.h"
#include "entity.h"


void collisionMgrInit(uint32_t maxObjects);
void collisionMgrShutdown();

// Add/Remove should ONLY be called from the object manager's add/remove functions
void collisionMgrAdd(Entity* obj);
void collisionMgrRemove(Entity* obj);

void handleAllCollisions(Entity* obj);
