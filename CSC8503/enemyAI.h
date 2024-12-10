#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"

namespace NCL::CSC8503 {

	class enemyAI : public GameObject
	{
	public:
		enemyAI() : GameObject() {};

		~enemyAI();

		void Update(float dt);

		NavigationPath* GetPath() {
			return outPath;
		}

		void SetPath(NavigationPath* path) {
			outPath = path;
		}

		vector<Vector3>* GetPathNodes() {
			return pathNodes;
		}

		void SetPathNodes(vector<Vector3>* nodes) {
			pathNodes = nodes;
		}


		bool findPathToObj(GameObject* obj, NavigationGrid* navGrid);

	private:
		
		NavigationPath* outPath;

		vector<Vector3>* pathNodes;

	};

}


