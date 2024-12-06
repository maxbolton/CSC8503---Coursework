#include "SpringConstraint.h"

#include "GameObject.h"
#include "Camera.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace Maths;
using namespace CSC8503;

SpringConstraint::SpringConstraint(GameObject* player, Camera* cam, float d) {
	this->player = player;
	this->camera = cam;
	targetDistance = d;
}

SpringConstraint::~SpringConstraint() {
}

void SpringConstraint::UpdateConstraint(float dt) {
	Vector3 relativePos = player->GetTransform().GetPosition() - camera->GetTransform().GetPosition();
	relativePos.y = 0.0f;

	float currentDistance = Vector::Length(relativePos);

	float offset = targetDistance - currentDistance;

	if (abs(offset) > 0.0f) {
		Vector3 offsetDir = Vector::Normalise(relativePos);

		PhysicsObject* playerPhys = player->GetPhysicsObject();
		PhysicsObject* camPhys = camera->GetPhysicsObject();

		Vector3 relativeVelocity = playerPhys->GetLinearVelocity() - camPhys->GetLinearVelocity();
		relativeVelocity.y = 0.0f;

		float constraintMass = playerPhys->GetInverseMass() + camPhys->GetInverseMass();

		if (constraintMass > 0.0f) {
			// How much of their relative force is affecting the constraint
			float velocityDot = Vector::Dot(relativeVelocity, offsetDir);

			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 playerImpulse = offsetDir * lambda;
			Vector3 camImpulse = -offsetDir * lambda;

			//physA->ApplyLinearImpulse(aImpulse); // multiplied by mass
			camPhys->ApplyLinearImpulse(camImpulse); // multiplied by mass
		}
	}

}