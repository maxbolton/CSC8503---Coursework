#include "playerCharacter.h"
#include "Window.h"



namespace NCL::CSC8503 {
	/*
	Update the position of the player character based on player input
	*/
	void playerCharacter::Update(float dt)
	{
		if (!activeController) {
			return;
		}

		//Update the mouse by how much
		pitch -= activeController->GetNamedAxis("YLook");
		yaw -= activeController->GetNamedAxis("XLook");

		//Bounds check the pitch, to be between straight up and straight down ;)
		pitch = std::min(pitch, 90.0f);
		pitch = std::max(pitch, -90.0f);

		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}

		float frameSpeed = speed * dt;

		Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

		position += yawRotation * Vector3(0, 0, -activeController->GetNamedAxis("Forward")) * frameSpeed;
		position += yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed;

		position.y += activeController->GetNamedAxis("UpDown") * frameSpeed;

	}

}
