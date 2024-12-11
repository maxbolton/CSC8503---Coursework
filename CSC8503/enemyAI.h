#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"
#include "State.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "playerCharacter.h"


namespace NCL::CSC8503 {

	class enemyAI : public GameObject
	{
	public:
		enemyAI() : GameObject() {

			progress = 1.0f;

			enemyStateMachine = new StateMachine();

			State* roaming = new State([&](float dt)->void {
				std::cout << "Enemy: Roaming\n"; 
				enemyAI::roaming(dt);
				}
			);

			State* chasingPlayer = new State([&](float dt)->void {
				std::cout << "Enemy: Chasing Player\n";
				enemyAI::chasingPlayer(dt);
				}
			);

			State* holdingPlayer = new State([&](float dt)->void {
				std::cout << "Enemy: Player Grabbed Menu\n";
				}
			);


			StateTransition* startChase = new StateTransition(roaming, chasingPlayer, [&](void)->bool {
				// Define condition to start chasing the player
				Vector3 enemyPos = this->GetTransform().GetPosition();
				Vector3 playerPos = player->GetTransform().GetPosition();

				float distance = Vector::Length(enemyPos - playerPos);

				return distance < 25.0f; // Example condition: start chasing if player is within 50 units
				}
			);

			StateTransition* giveUp = new StateTransition(chasingPlayer, roaming, [&](void)->bool {
				// Define condition to give up chasing the player
				Vector3 enemyPos = this->GetTransform().GetPosition();
				Vector3 playerPos = player->GetTransform().GetPosition();

				float distance = Vector::Length(enemyPos - playerPos);

				return distance > 25.0f; // Example condition: give up chasing if player is more than 50 units away
				}
			);

			StateTransition* startGrab = new StateTransition(chasingPlayer, holdingPlayer, [&](void)->bool {
				return false; }
			);

			StateTransition* stopGrab = new StateTransition(holdingPlayer, roaming, [&](void)->bool {
				return false; }
			);

			enemyStateMachine->AddState(roaming);
			enemyStateMachine->AddState(chasingPlayer);
			enemyStateMachine->AddState(holdingPlayer);

			enemyStateMachine->AddTransition(startChase);
			enemyStateMachine->AddTransition(giveUp);
			enemyStateMachine->AddTransition(startGrab);
			enemyStateMachine->AddTransition(stopGrab);

			enemyStateMachine->SetState(roaming);
		
		};

		~enemyAI();

		void Update(float dt);

		
		NavigationPath* GetPath() {
			return outPath;
		}

		void SetPath(NavigationPath* path) {
			outPath = path;
		}

		NavigationGrid* GetNavGrid() {
			return navGrid;
		}

		void SetNavGrid(NavigationGrid* grid) {
			navGrid = grid;
		}


		void SetPlayer(playerCharacter* p) {
			player = p;
		}

	private:
		

		bool findPathToObj(GameObject* obj);
		bool findPathToPos(Vector3 pos);
		bool findRandomPath();
		void traversePath(float dt);

		NavigationGrid* navGrid;
		NavigationPath* outPath;
		float progress;


		void printPath(Vector4 col);

		playerCharacter* player;

		StateMachine* enemyStateMachine;

		//state machine methods
		void roaming(float dt);
		void chasingPlayer(float dt);
		void holdingPlayer(float dt);



	};








}


