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

		int maxWidth = navGrid->GetGridWidth();
		int maxHeight = navGrid->GetGridHeight();

		// ensure rand() is seeded
		srand(time(NULL));
		//new random position within maze bounds
		Vector3 newPos = Vector3(rand() % maxWidth, 0, rand() % maxHeight);

		//make global by timesing by node size
		newPos.x *= navGrid->GetNodeSize();
		newPos.z *= navGrid->GetNodeSize();

		return findPathToPos(newPos);

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

		PhysicsObject* phys = this->GetPhysicsObject();
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

			phys->AddForce(direction * 10.0f);


			//rotate enemy to face next point
			Quaternion currentOrientation = this->GetTransform().GetOrientation();
			Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, atan2(-direction.x, -direction.z) * 180.0f / 3.14159265358979323846f, 0);

			// Ensure the shortest path is taken
			if (Quaternion::Dot(currentOrientation, targetOrientation) < 0.0f) {
				targetOrientation = -targetOrientation;
			}
			this->GetTransform().SetOrientation(Quaternion::Slerp(currentOrientation, targetOrientation, 0.25f));


			//if enemy is close to next point, remove it from path or is closer to the next point
			Vector3 secondNextPos = this->GetPath()->GetPoints().at(this->GetPath()->GetPoints().size() - 2);
			Vector3 secondNextGlobalPos = navGrid->GetWorldPos(secondNextPos.x, secondNextPos.z);

			float distance = Vector::Length(nextGlobalPos - enemyPos);

			// if enemy is closer to the second next point than the next point, remove the next point
			if (Vector::Length(secondNextGlobalPos - enemyPos) < distance || distance < 3.0f) {
				this->GetPath()->GetPoints().pop_back();
			}

		}
	}

	bool enemyAI::hasPlayerMoved() {

		Vector3 localPlayerPos = this->GetNavGrid()->GetLocalPos(player->GetTransform().GetPosition());
		Vector3 target = this->GetPath()->GetPoints().front();

		// has player moved from within 5 units of the first point in the path?
        if (abs(localPlayerPos.x - target.x) > 5 || abs(localPlayerPos.z - target.z) > 5) {
        return true;
        }
		return false;
	}

#pragma region State Functions
    void enemyAI::roaming(float dt) {
		
		//is path empty?
		if (this->GetPath() == nullptr || this->GetPath()->GetPoints().size() <= 2) {
			std::cout << "random path found?: " << findRandomPath() << std::endl ;
		}
		else {
			traversePath(dt);
			printPath(Debug::GREEN);
		}

    }

	void enemyAI::chasingPlayer(float dt) {
		//chase player


		// is path empty or has player moved?
		if (this->GetPath() == nullptr || this->GetPath()->GetPoints().size() <= 2 || hasPlayerMoved()) {
			std::cout << "player path found?: " << findPathToObj(player) << std::endl;
		}
		else {
			traversePath(dt);
			printPath(Debug::RED);
		}

	}

#pragma endregion

}