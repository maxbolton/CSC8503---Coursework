#include "enemyAI.h"
#include "NavigationMesh.h"
#include "NavigationGrid.h"
#include "Debug.h"

namespace NCL::CSC8503 {

	void enemyAI::Update(float dt) {
		//std::cout << "Enemy AI Update\n";
	}

	enemyAI::~enemyAI() {
	}

	bool enemyAI::findPathToObj(GameObject* obj, NavigationGrid* navGrid) {
		// find path to object within navMesh
		
		Vector3 objPos = navGrid->GetLocalPos(obj->GetTransform().GetPosition());
		Vector3 enemyPos = navGrid->GetLocalPos(this->GetTransform().GetPosition());
		

		NavigationPath out;


		bool found = navGrid->FindPath(enemyPos, objPos, out);


		//this->SetPath(out);

		vector<Vector3> testNodes;

		Vector3 pos;
		while (out.PopWaypoint(pos)) {
			testNodes.push_back(pos);
		}
		

		for (int i = 1; i < testNodes.size(); ++i) {
			Vector3 a = testNodes[i - 1];
			Vector3 b = testNodes[i];

			// add -200 offset to z to make it visible
			a.z -= 200;
			b.z -= 200;

			Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		}
		

		return true;

	}

}