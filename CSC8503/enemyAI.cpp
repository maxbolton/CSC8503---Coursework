#include "enemyAI.h"
#include "NavigationMesh.h"
#include "NavigationGrid.h"
#include "Debug.h"
#include "PhysicsObject.h"

namespace NCL::CSC8503 {

	void enemyAI::Update(float dt) {
		//std::cout << "Enemy AI Update\n";
		enemyStateMachine->Update(dt);

		
	}

	enemyAI::~enemyAI() {
		delete enemyStateMachine;
	}

	bool enemyAI::findPathToObj(GameObject* obj) {
		// find path to object within navMesh
		
		Vector3 objPos = this->GetNavGrid()->GetLocalPos(obj->GetTransform().GetPosition());
		Vector3 enemyPos = this->GetNavGrid()->GetLocalPos(this->GetTransform().GetPosition());

		NavigationPath* out = new NavigationPath();


		bool found = navGrid->FindPath(enemyPos, objPos, *out);

		if (found) {
			// if new path is same as current path, don't update
			// check if start and end points are the same
			if (this->GetPath() != nullptr) {
				if (this->GetPath()->GetPoints().front().x != out->GetPoints().front().x ||
					this->GetPath()->GetPoints().front().z != out->GetPoints().front().z) {
					this->SetPath(out);
					return true;
				}
				return true;
			}
			this->SetPath(out);
			return true;
		}
		else {
			return false;
		}
	}

	bool enemyAI::findPathToPos(Vector3 pos) {
		// find path to position within navMesh
		Vector3 enemyPos = this->GetNavGrid()->GetLocalPos(this->GetTransform().GetPosition());
		NavigationPath* out = new NavigationPath();

		bool found = navGrid->FindPath(enemyPos, pos, *out);

		if (found) {
			this->SetPath(out);
			return true;
		}
		else {
			return false;
		}
	}

	bool enemyAI::findRandomPath() {

		Vector3 newPos = Vector3(0, 0, 0);

		Vector3 origin = *navGrid->GetOrigin();
		float minWidth = origin.x;
		float minHeight = origin.z;

		float maxWidth = navGrid->GetGridWidth() * navGrid->GetNodeSize();
		float maxHeight = navGrid->GetGridHeight() * navGrid->GetNodeSize();

		do {
			// ensure rand() is seeded
			srand(time(NULL));
			//new random position within maze bounds
			newPos = Vector3(rand() % (int)(maxWidth - minWidth) + minWidth, 0, rand() % (int)(maxHeight - minHeight) + minHeight);

		} while (!findPathToPos(newPos));

		return true;

	}

	void enemyAI::printPath(Vector4 col) {

		vector<Vector3> testNodes;
		Vector3 pos;
		NavigationPath out = *this->GetPath();

		while (out.PopWaypoint(pos)) {
			testNodes.push_back(pos);
		}

		for (int i = 1; i < testNodes.size(); ++i) {
			Vector3 a = testNodes[i - 1];
			Vector3 b = testNodes[i];

			Debug::DrawLine(navGrid->GetWorldPos(a.x, a.z), navGrid->GetWorldPos(b.x,b.z), col);
		}
	}

	void enemyAI::traversePath(float dt) {
		//move enemy along path
		if (this->GetPath()->GetPoints().size() > 0) {
			Vector3 enemyPos = this->GetTransform().GetPosition(); // enemy global position

			Vector3 nextPos = this->GetPath()->GetPoints().back();// next point in path (local)
			Vector3 nextGlobalPos = navGrid->GetWorldPos(nextPos.x, nextPos.z); // next point in path (global)
			nextGlobalPos.y = enemyPos.y;


			Debug::DrawLine(enemyPos, nextGlobalPos, Debug::YELLOW);

			Vector3 direction = nextGlobalPos - enemyPos;
			Vector::Normalise(direction);
			//move enemy
			this->GetPhysicsObject()->AddForce(direction * 5.0f);

			//if enemy is close to next point, remove it from path
			if (Vector::Length(nextGlobalPos - enemyPos) < 5.0f) {
				this->GetPath()->GetPoints().pop_back();
			}
		}
	}

#pragma region State Functions
    void enemyAI::roaming(float dt) {
		

        //pick random point on navMesh to move to
		if (progress >= 1.0f || this->GetPath() == nullptr) {
			if (findRandomPath())
				progress = 0.0f;
		}

		//printPath(Debug::GREEN);
        

        // Measure progress towards the end point
        Vector3 enemyPos = this->GetTransform().GetPosition();
        Vector3 endPos = this->GetPath()->GetPoints().back();
        float totalDistance = Vector::Length(endPos - enemyPos);
        float currentDistance = Vector::Length(endPos - this->GetTransform().GetPosition());
        progress = 1.0f - (currentDistance / totalDistance);

		// Move towards the end point
		//traversePath(dt);

    }

	void enemyAI::chasingPlayer(float dt) {
		//chase player
		if (findPathToObj(player)) {
			printPath(Debug::RED);
		}

		traversePath(dt);
	}

	void enemyAI::holdingPlayer(float dt) {
		//hold player
	}

#pragma endregion

}