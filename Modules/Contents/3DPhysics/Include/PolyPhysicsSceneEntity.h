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

#pragma once
#include "PolyGlobals.h"
#include "PolyCollisionSceneEntity.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <vector>

class btKinematicCharacterController;
class btPairCachingGhostObject;
class btDefaultMotionState;

namespace Polycode {
	
	/**
	* A wrapper around SceneEntity that provides physics information.
	*/
	class _PolyExport PhysicsSceneEntity : public CollisionSceneEntity {
	public:
		PhysicsSceneEntity(SceneEntity *entity, int type, Number mass, Number friction, Number restitution);
		virtual ~PhysicsSceneEntity();
		virtual void Update();
				
			/** @name Physics scene entity
			*  Public methods
			*/
			//@{			
				
		SceneEntity *getSceneEntity();
		void setFriction(Number friction);		
		int getType() { return type; }	
		
			void setVelocity(Vector3 velocity);
			void warpTo(Vector3 position, bool resetRotation);
			//@}
			// ----------------------------------------------------------------------------------------------------------------
			
			
		
	/**
		* Box shape
		*/
		static const int SHAPE_BOX = 0;		
		/**
		* Terrain shape
		*/		
		static const int SHAPE_TERRAIN = 1;
		
		/**
		* Sphere shape
		*/		
		static const int SHAPE_SPHERE = 2;	
		
		/**
		* Mesh shape
		*/		
		static const int SHAPE_MESH = 3;			
		
		/**
		* Character controller shape
		*/				
		static const int CHARACTER_CONTROLLER = 4;
		
		/**
		* Capsule shape
		*/						
		static const int SHAPE_CAPSULE = 5;		
		
		/**
		* Plane shape
		*/								
		static const int SHAPE_PLANE = 6;
		
		/**
		* Cone shape
		*/										
		static const int SHAPE_CONE = 7;
		
		/**
		* Cylinder shape
		*/												
		static const int SHAPE_CYLINDER = 8;

		
		bool enabled;
		
		btRigidBody* rigidBody;
		
	protected:
	
		Number mass;
		
		// Kept just to be deleted
		btDefaultMotionState* myMotionState;
	};
	
	/**
	* A Physics character controller.
	*/
	class _PolyExport PhysicsCharacter : public PhysicsSceneEntity {
		public:
			PhysicsCharacter(SceneEntity *entity, Number mass, Number friction, Number stepSize);
			virtual ~PhysicsCharacter();
	
			virtual void Update();
				
			/** @name Physics character
			*  Public methods
			*/
			//@{			
	
			void setWalkDirection(Vector3 direction);
			void jump();
			
			void warpCharacter(Vector3 position);
			void setJumpSpeed(Number jumpSpeed);
			void setFallSpeed(Number fallSpeed);			
			void setMaxJumpHeight(Number maxJumpHeight);			
			bool onGround();
			//@}
			// ----------------------------------------------------------------------------------------------------------------
		
		
			btKinematicCharacterController *character;
			btPairCachingGhostObject *ghostObject;
				
		
		protected:
	};
	
	/**
	* Physics vehicle wheel info.
	*/
	class PhysicsVehicleWheelInfo {
		public:
			/**
			* Wheel index.
			*/
			unsigned int wheelIndex;
			
			/**
			* Wheel scene entity.
			*/			
			SceneEntity *wheelEntity;
	};
	
	/**
	* A physics vehicle controller.
	*/
	class _PolyExport PhysicsVehicle : public PhysicsSceneEntity {
		public:
				
			PhysicsVehicle(SceneEntity *entity, Number mass, Number friction, btDefaultVehicleRaycaster *rayCaster);

			/** @name Physics vehicle
			*  Public methods
			*/
			//@{			

			void addWheel(SceneEntity *entity, Vector3 connection, Vector3 direction, Vector3 axle, Number suspentionRestLength, Number wheelRadius, bool isFrontWheel,Number suspensionStiffness = 20.0f, Number suspensionDamping = 1.0f, Number suspensionCompression = 4.0f, Number wheelFriction = 10000.0f, Number rollInfluence = 0.05f);
			void applyEngineForce(Number force, unsigned int wheelIndex);
			void setSteeringValue(Number value, unsigned int wheelIndex);
			void setBrake(Number value, unsigned int wheelIndex);
			
			void warpVehicle(Vector3 position);

			//@}
			// ----------------------------------------------------------------------------------------------------------------


			void Update();
			virtual ~PhysicsVehicle();
						
		
		btRaycastVehicle::btVehicleTuning tuning;
		btDefaultVehicleRaycaster *rayCaster;
		btRaycastVehicle *vehicle;
		
		protected:
			std::vector<PhysicsVehicleWheelInfo> wheels;

	};
}
