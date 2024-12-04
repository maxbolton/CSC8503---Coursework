#pragma once
#include "Constraint.h"



namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class SpringConstraint : public Constraint {
		public:
			SpringConstraint(GameObject* a, GameObject* b, float d);
			~SpringConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;

			float targetDistance;
		};
	}
}