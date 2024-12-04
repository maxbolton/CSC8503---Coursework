#include "playerCharacter.h"
#include "Window.h"
#include "PhysicsObject.h"



namespace NCL::CSC8503 {
	/*
	Update the position of the player character based on player input
	*/
	void playerCharacter::Update(float dt)
	{
		if (!activeController) {
			return;
		}

		float frameSpeed = speed * dt;
		Vector3 currentPos = this->GetPhysicsObject()->GetTransform()->GetPosition();
		Transform* t = this->GetPhysicsObject()->GetTransform();

		yaw -= activeController->GetNamedAxis("XLook");
		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}
		Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

		Vector3 forwardMovement(0, 0, 0);
		Vector3 sideMovement(0, 0, 0);

		if (this->IsGrounded()) {
			std::cout << "player is grounded!" << std::endl;
			forwardMovement = yawRotation * Vector3(0, 0, activeController->GetNamedAxis("Forward")) * frameSpeed; // Z-axis
			sideMovement = yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed; // X-axis


			// apply impulse force in 'up' direction if the player presses the jump button
			this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, activeController->GetNamedAxis("UpDown") * 3, 0));

		}

		Vector3 totalMovement = forwardMovement + sideMovement;

		t->SetPosition(currentPos + totalMovement);
		t->SetOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));

	}

}
