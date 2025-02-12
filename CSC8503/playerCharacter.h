#pragma once
#include "GameObject.h"
#include "Controller.h"


namespace NCL::CSC8503{

	enum playerState {
		defaultState,
		dead,
		invincible,
		roombaMode,
	};

	class playerCharacter : public GameObject
	{

	public:

		playerCharacter() : GameObject() {
			yaw = 0.0f;
			grounded = false;
			kittens = new vector<GameObject*>();
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

		void SetState(playerState s) {
			state = s;
		}

		playerState GetState() {
			return state;
		}

		void joyControls(float dt);

		void tankControls(float dt);

		void roombaControls(float dt);

		void addKitten(GameObject* k) {
			kittens->push_back(k);
		}

		vector<GameObject*>* getKittens() {
			return kittens;
		}

	
	private:

		const Controller* activeController = nullptr;
		float yaw;
		float speed = 100.0f;
		bool grounded;
		playerState state = defaultState;

		vector<GameObject*>* kittens;
	};



}

