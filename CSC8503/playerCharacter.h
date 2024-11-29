#pragma once
#include "GameObject.h"
#include "Controller.h"


namespace NCL::CSC8503{

	class playerCharacter : public GameObject
	{
	public:

		playerCharacter() : GameObject() {};
 
		~playerCharacter();

		void Update(float dt);

		SetController(const Controller& c) {
			activeController = &c;
			return *this;
		}

	
	private:

		const Controller* activeController = nullptr;
	};



}

