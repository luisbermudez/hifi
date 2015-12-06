//
//  ToggleCollisionDispatcher.h
//  libraries/physics/src
//
//  Created by Luis Bermudez 2015.12.05
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ToggleCollisionDispatcher_h
#define hifi_ToggleCollisionDispatcher_h

#include <btBulletDynamicsCommon.h>

class ToggleCollisionDispatcher: public btCollisionDispatcher {
public:
    using btCollisionDispatcher::btCollisionDispatcher;
    
    bool needsCollision(const btCollisionObject* body0,const btCollisionObject* body1) { return !(_disableCollisions) && btCollisionDispatcher::needsCollision( body0, body1 ); };
    
    void setDisableCollisions(const bool flag) { _disableCollisions = flag; }
    
    bool _disableCollisions = false;
    
private:
    
};

#endif
