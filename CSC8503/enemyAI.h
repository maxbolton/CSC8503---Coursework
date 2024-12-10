#pragma once
#include "GameObject.h"

namespace NCL::CSC8503 {

	class enemyAI : public GameObject
	{
	public:
		enemyAI() : GameObject() {};

		~enemyAI();

		void Update(float dt);

	private:
		
	};

}


