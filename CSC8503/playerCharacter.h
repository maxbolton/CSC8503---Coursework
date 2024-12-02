#pragma once
#include "GameObject.h"
#include "Controller.h"


namespace NCL::CSC8503{

	class playerCharacter : public GameObject
	{
	public:

		playerCharacter() : GameObject() {
			yaw = 0.0f;
			grounded = false;
		};
 
		~playerCharacter() {};

		void Update(float dt);

		playerCharacter& SetController(const Controller& c) {
			activeController = &c;
			return *this;
		}

		bool IsGrounded() {
			return grounded;
		}

		void SetGrounded(bool g) {
			grounded = g;
		}

	
	private:

		const Controller* activeController = nullptr;
		float yaw;
		float speed = 100.0f;
		bool grounded;
	};



}

