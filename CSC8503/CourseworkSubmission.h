#include "../NCLCoreClasses/KeyboardMouseController.h"
#include "playerCharacter.h"
#include "enemyAI.h"
#include "NavigationGrid.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"


namespace NCL {
	namespace CSC8503 {
		class CourseworkSubmission {
		public:
			CourseworkSubmission();
			~CourseworkSubmission();

			virtual void UpdateGame(float dt);


			void UIManager(float dt);

		protected:

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			void BridgeConstraintTest();

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitMaze(Vector3 origin);
			void BuildMazeFromGrid(Vector3 origin);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void MovePlayerTo(const Vector3& destination);
			
				
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			playerCharacter* AddPlayerToWorld(const Vector3& position);
			enemyAI* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;
			KeyboardMouseController controller;
			playerCharacter* player;
			NavigationGrid* navGrid;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* catMesh = nullptr;
			Mesh* kittenMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;

			Texture* beigeTex = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;



			Ray* lastRay = nullptr;

			Ray GetLastRay() const {
				return *lastRay;
			}

			void SetLastRay(Ray& r) {
				lastRay = &r;
			}


		};
	}
}

