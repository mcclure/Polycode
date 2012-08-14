/*
Copyright (C) 2011 by Ivan Safrin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "PolyPhysicsScene.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#include "PolyCollisionScene.h"
#include "PolyCoreServices.h"
#include "PolyVector3.h"
#include "PolyPhysicsSceneEntity.h"
#include "PolyCore.h"

using namespace Polycode;

PhysicsSceneEvent::PhysicsSceneEvent() : Event () {
	eventType = "PhysicsSceneEvent";
}

PhysicsSceneEvent::~PhysicsSceneEvent() {

}


PhysicsScene::PhysicsScene(int maxSubSteps, Vector3 size, bool virtualScene) : CollisionScene(size, virtualScene, true), physicsWorld(NULL), solver(NULL), broadphase(NULL), ghostPairCallback(NULL) {
	this->maxSubSteps = maxSubSteps;
	initPhysicsScene(size);	
}

PhysicsScene::~PhysicsScene() {
	// These MUST be deleted first
	for(int i=0; i < collisionChildren.size(); i++) {
		delete collisionChildren[i];
	}	
	delete physicsWorld;
	
	// Prevent double free by ~CollisionScene
	collisionChildren.clear();
	world = NULL;
	
	delete solver;
	delete broadphase;
	delete ghostPairCallback;
}

void worldTickCallback(btDynamicsWorld *world, btScalar timeStep) {
	PhysicsScene *physicsScene = (PhysicsScene*)world->getWorldUserInfo();
	physicsScene->processWorldCollisions();
}

void PhysicsScene::initPhysicsScene(Vector3 size) {
		
	collisionConfiguration = new btDefaultCollisionConfiguration();	
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);	
	
	solver = new btSequentialImpulseConstraintSolver();
	
	btVector3 worldMin(-size.x * 0.5, -size.y * 0.5, -size.z * 0.5);
	btVector3 worldMax(size.x * 0.5, size.y * 0.5, size.z * 0.5);
	axisSweep = new btAxisSweep3(worldMin,worldMax);	
	
	broadphase = new btDbvtBroadphase();	
	physicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	
//	physicsWorld->getSolverInfo().m_solverMode |= SOLVER_RANDMIZE_ORDER;
	physicsWorld->setGravity(btVector3(0,-10,0));
	ghostPairCallback = new btGhostPairCallback();
	axisSweep->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	
	world = physicsWorld;
	
	physicsWorld->setInternalTickCallback(worldTickCallback, this);
}

void PhysicsScene::setGravity(Vector3 gravity) {
	physicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

PhysicsSceneEntity *PhysicsScene::getPhysicsEntityByCollisionObject(btCollisionObject *object) {
	for(int i=0; i < physicsChildren.size(); i++) {
		PhysicsSceneEntity *entity = physicsChildren[i];
		if(entity->rigidBody == object) {
			return entity;
		}
	}
	return  NULL;
}

void PhysicsScene::processWorldCollisions() {

	int numManifolds = physicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
	
		
		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;
				const btScalar appliedImpulse = pt.m_appliedImpulse;
				
				PhysicsSceneEvent *event = new PhysicsSceneEvent();
				event->positionOnA = Vector3(ptA.x(), ptA.y(), ptA.z());
				event->positionOnB = Vector3(ptB.x(), ptB.y(), ptB.z());
				event->worldNormalOnB = Vector3(normalOnB.x(), normalOnB.y(), normalOnB.z());
				event->appliedImpulse = appliedImpulse;				
								
				event->entityA = getPhysicsEntityByCollisionObject(obA);	
				event->entityB = getPhysicsEntityByCollisionObject(obB);
												
				dispatchEvent(event, PhysicsSceneEvent::COLLISION_EVENT);
			}
		}
	}

}

void PhysicsScene::Update() {
	
	for(int i=0; i < physicsChildren.size(); i++) {
//		if(physicsChildren[i]->enabled)
			physicsChildren[i]->Update();
	}
	
	
	Number elapsed = CoreServices::getInstance()->getCore()->getElapsed();
	if(maxSubSteps > 0) {
		physicsWorld->stepSimulation(elapsed, maxSubSteps);	
	} else {
		physicsWorld->stepSimulation(elapsed);		
	}
	CollisionScene::Update();
	
}

void PhysicsScene::setVelocity(SceneEntity *entity, Vector3 velocity) {
	PhysicsSceneEntity *physicsEntity = getPhysicsEntityBySceneEntity(entity);
	if(physicsEntity) {
		physicsEntity->setVelocity(velocity);
	}
}

void PhysicsScene::warpEntity(SceneEntity *entity, Vector3 position, bool resetRotation) {
	PhysicsSceneEntity *physicsEntity = getPhysicsEntityBySceneEntity(entity);
	if(physicsEntity) {
		physicsEntity->rigidBody->setActivationState(DISABLE_DEACTIVATION);	
		physicsEntity->warpTo(position, resetRotation);
	}
}

void PhysicsScene::applyImpulse(SceneEntity *entity, Vector3 force, Vector3 point) {
	PhysicsSceneEntity *physicsEntity = getPhysicsEntityBySceneEntity(entity);	
	if(physicsEntity) {
		physicsEntity->rigidBody->setActivationState(DISABLE_DEACTIVATION);		
		physicsEntity->applyImpulse(force, point);
	}
}

PhysicsCharacter *PhysicsScene::addCharacterChild(SceneEntity *newEntity,Number mass, Number friction, Number stepSize, int group) {
	addEntity(newEntity);	
	PhysicsCharacter *newPhysicsEntity = new PhysicsCharacter(newEntity, mass, friction, stepSize);
	
	physicsWorld->addCollisionObject(newPhysicsEntity->ghostObject,group, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	physicsWorld->addAction(newPhysicsEntity->character);
	
	
	physicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(newPhysicsEntity->ghostObject->getBroadphaseHandle(),physicsWorld->getDispatcher());
	
	newPhysicsEntity->character->reset ();
	
	newPhysicsEntity->character->setUseGhostSweepTest(false);
	
	physicsChildren.push_back(newPhysicsEntity);
	collisionChildren.push_back(newPhysicsEntity);
	return newPhysicsEntity;
	
}

void PhysicsScene::removeCharacterChild(PhysicsCharacter *character) {
	physicsWorld->removeAction(character->character);

	physicsWorld->removeCollisionObject(character->ghostObject);


		for(int i=0; i < physicsChildren.size(); i++) {
			if(physicsChildren[i] == character) {
				physicsChildren.erase(physicsChildren.begin()+i);
			}
		}
		for(int i=0; i < collisionChildren.size(); i++) {
			if(collisionChildren[i] == character) {
				collisionChildren.erase(collisionChildren.begin()+i);
			}
		}		
	
}


PhysicsVehicle *PhysicsScene::addVehicleChild(SceneEntity *newEntity, Number mass, Number friction, int group) {
	addEntity(newEntity);		
	
	btDefaultVehicleRaycaster *m_vehicleRayCaster = new btDefaultVehicleRaycaster(physicsWorld);
	
	PhysicsVehicle *newPhysicsEntity = new PhysicsVehicle(newEntity, mass, friction,m_vehicleRayCaster);		
	physicsWorld->addRigidBody(newPhysicsEntity->rigidBody, group, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);	
		
	newPhysicsEntity->vehicle = new btRaycastVehicle(newPhysicsEntity->tuning,newPhysicsEntity->rigidBody,m_vehicleRayCaster);	
	
	newPhysicsEntity->rigidBody->setActivationState(DISABLE_DEACTIVATION);
	
		
	int rightIndex = 0; 
		int upIndex = 1; 
		int forwardIndex = 2;
			
	physicsWorld->addVehicle(newPhysicsEntity->vehicle);
	
	newPhysicsEntity->vehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

	newPhysicsEntity->vehicle->resetSuspension();

	physicsChildren.push_back(newPhysicsEntity);
	collisionChildren.push_back(newPhysicsEntity);
		
	return newPhysicsEntity;
}

void PhysicsScene::removePhysicsChild(SceneEntity *entity) {
	PhysicsSceneEntity *ent = getPhysicsEntityBySceneEntity(entity);
	if(ent) {
		if(ent->getType() == PhysicsSceneEntity::CHARACTER_CONTROLLER) {
			removeCharacterChild((PhysicsCharacter *)ent);
		} else {
		
			if(ent->rigidBody) 
				physicsWorld->removeRigidBody(ent->rigidBody);
			physicsWorld->removeCollisionObject(ent->collisionObject);
			for(int i=0; i < physicsChildren.size(); i++) {
				if(physicsChildren[i] == ent) {
					physicsChildren.erase(physicsChildren.begin()+i);
				}
			}
			for(int i=0; i < collisionChildren.size(); i++) {
				if(collisionChildren[i] == ent) {
					collisionChildren.erase(collisionChildren.begin()+i);
				}
			}
		}
	}
	delete ent;
	CollisionScene::removeEntity(entity);
}

void PhysicsScene::removeEntity(SceneEntity *entity) {
	PhysicsSceneEntity *ent = getPhysicsEntityBySceneEntity(entity);
	if(ent) {
		removePhysicsChild(entity);
	} else {
		CollisionScene::removeEntity(entity);
	}
}

void PhysicsHingeConstraint::setLimits(Number minLimit, Number maxLimit) {
	btConstraint->setLimit(minLimit, maxLimit);
}

Number PhysicsHingeConstraint::getAngle() {
	return btConstraint->getHingeAngle();
}

void PhysicsGenericConstraint::setLinearLowerLimit(Vector3 limit) {
	btVector3 btLimit = btVector3(limit.x, limit.y, limit.z);
	btConstraint->setLinearLowerLimit(btLimit);
}

void PhysicsGenericConstraint::setLinearUpperLimit(Vector3 limit) {
	btVector3 btLimit = btVector3(limit.x, limit.y, limit.z);
	btConstraint->setLinearUpperLimit(btLimit);
}

void PhysicsGenericConstraint::setAngularLowerLimit(Vector3 limit) {
	btVector3 btLimit = btVector3(limit.x, limit.y, limit.z);
	btConstraint->setAngularLowerLimit(btLimit);
}

void PhysicsGenericConstraint::setAngularUpperLimit(Vector3 limit) {
	btVector3 btLimit = btVector3(limit.x, limit.y, limit.z);
	btConstraint->setAngularUpperLimit(btLimit);
}

PhysicsGenericConstraint *PhysicsScene::createGenericConstraint(SceneEntity *entity) {

	PhysicsSceneEntity *pEnt = getPhysicsEntityBySceneEntity(entity);
	if(!pEnt) {
		return NULL;
	}
	
	PhysicsGenericConstraint *constraint = new PhysicsGenericConstraint();
	
	btTransform frame;
	frame.setIdentity();
	
	constraint->btConstraint = new btGeneric6DofConstraint(*pEnt->rigidBody, frame, true);
	physicsWorld->addConstraint(constraint->btConstraint);
	
	return constraint;
}

PhysicsHingeConstraint * PhysicsScene::createHingeConstraint(SceneEntity *entity, Vector3 pivot, Vector3 axis, Number minLimit, Number maxLimit) {
	PhysicsSceneEntity *pEnt = getPhysicsEntityBySceneEntity(entity);
	if(!pEnt) {
		return NULL;
	}
	
	PhysicsHingeConstraint *constraint = new PhysicsHingeConstraint();	
	btVector3 btPivot = btVector3(pivot.x, pivot.y, pivot.z);
	btVector3 btAxis = btVector3(axis.x, axis.y, axis.z);
	
	btHingeConstraint *hingeConstraint = new btHingeConstraint( *pEnt->rigidBody, btPivot, btAxis );
	hingeConstraint->setLimit(minLimit, maxLimit);
	physicsWorld->addConstraint(hingeConstraint);
	
	constraint->btConstraint = hingeConstraint;
	
	return constraint;
}

PhysicsHingeConstraint *PhysicsScene::createHingeJoint(SceneEntity *entity1, SceneEntity *entity2, Vector3 pivot1, Vector3 axis1, Vector3 pivot2, Vector3 axis2, Number minLimit, Number maxLimit) {
	
	PhysicsSceneEntity *pEnt1 = getPhysicsEntityBySceneEntity(entity1);
	PhysicsSceneEntity *pEnt2 = getPhysicsEntityBySceneEntity(entity2);
		
	if(!pEnt1 || !pEnt2) {
		return NULL;
	}
	
	PhysicsHingeConstraint *constraint = new PhysicsHingeConstraint();	
	btVector3 btPivot1 = btVector3(pivot1.x, pivot1.y, pivot1.z);
	btVector3 btAxis1 = btVector3(axis1.x, axis1.y, axis1.z);

	btVector3 btPivot2 = btVector3(pivot2.x, pivot2.y, pivot2.z);
	btVector3 btAxis2 = btVector3(axis2.x, axis2.y, axis2.z);

	btHingeConstraint *hingeConstraint = new btHingeConstraint(*pEnt1->rigidBody, *pEnt2->rigidBody, btPivot1, btPivot2, btAxis1, btAxis2 );
	hingeConstraint->setLimit(minLimit, maxLimit);
	physicsWorld->addConstraint(hingeConstraint);
	
	constraint->btConstraint = hingeConstraint;
	
	return constraint;	
	
}


PhysicsSceneEntity *PhysicsScene::getPhysicsEntityBySceneEntity(SceneEntity *entity) {
	PhysicsSceneEntity *retEntity = NULL;
	for(int i=0; i < physicsChildren.size(); i++) {
		if(physicsChildren[i]->getSceneEntity() == entity) {
			retEntity = physicsChildren[i];
		}	
	}
	return retEntity;
}

void PhysicsScene::wakeUp(SceneEntity *entity) {
	PhysicsSceneEntity *pEnt = getPhysicsEntityBySceneEntity(entity);
	if(!pEnt) {
		return;
	}
	pEnt->rigidBody->setActivationState(DISABLE_DEACTIVATION);
}

PhysicsSceneEntity *PhysicsScene::trackPhysicsChild(SceneEntity *newEntity, int type, Number mass, Number friction, Number restitution, int group, bool compoundChildren) {
	PhysicsSceneEntity *newPhysicsEntity = new PhysicsSceneEntity(newEntity, type, mass, friction,restitution, compoundChildren);
	physicsWorld->addRigidBody(newPhysicsEntity->rigidBody, group,  btBroadphaseProxy::AllFilter); //btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);	
//	world->addCollisionObject(newPhysicsEntity->collisionObject, group);	
	//newPhysicsEntity->rigidBody->setActivationState(ISLAND_SLEEPING);	
	physicsChildren.push_back(newPhysicsEntity);
	collisionChildren.push_back(newPhysicsEntity);	
	return newPhysicsEntity;	
}

PhysicsSceneEntity *PhysicsScene::addPhysicsChild(SceneEntity *newEntity, int type, Number mass, Number friction, Number restitution, int group, bool compoundChildren) {
	addEntity(newEntity);	
	return trackPhysicsChild(newEntity, type, mass, friction, restitution, group, compoundChildren);	
}
