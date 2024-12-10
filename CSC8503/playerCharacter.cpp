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

		switch (state) {
		case defaultState:
			joyControls(dt);
			break;
		case dead:
			break;
		case invincible:
			break;
		case roombaMode:
			roombaControls(dt);
			break;
		}
        
    }

    void playerCharacter::joyControls(float dt) {
        float frameSpeed = (speed * 0.5) * dt;
        Vector3 currentPos = this->GetPhysicsObject()->GetTransform()->GetPosition();
        Transform* t = this->GetPhysicsObject()->GetTransform();

        float forwardInput = activeController->GetNamedAxis("Forward");
        float sideInput = activeController->GetNamedAxis("Sidestep");

        if (forwardInput != 0 || sideInput != 0) {
            yaw = atan2(sideInput, -forwardInput) * 180.0f / 3.14159265358979323846f;
            if (yaw < 0) {
                yaw += 360.0f;
            }
        }


        Quaternion currentOrientation = t->GetOrientation();
        Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, yaw, 0);

        // Ensure the shortest path is taken
        if (Quaternion::Dot(currentOrientation, targetOrientation) < 0.0f) {
            targetOrientation = -targetOrientation;
        }

        t->SetOrientation(Quaternion::Slerp(currentOrientation, targetOrientation, 0.25f));


        Vector3 forwardMovement(0, 0, 0);
        Vector3 sideMovement(0, 0, 0);

        if (this->IsGrounded()) {
            //std::cout << "player is grounded!" << std::endl;
            forwardMovement = Vector3(0, 0, -forwardInput) * frameSpeed; // Z-axis
            sideMovement = Vector3(sideInput, 0, 0) * frameSpeed; // X-axis

            // apply impulse force in 'up' direction if the player presses the jump button
            this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, activeController->GetNamedAxis("UpDown") * 3, 0));
        }

        Vector3 totalMovement = forwardMovement + sideMovement;

        t->SetPosition(currentPos + totalMovement);
    }

	void playerCharacter::tankControls(float dt)
	{
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
			//std::cout << "player is grounded!" << std::endl;
			forwardMovement = yawRotation * Vector3(0, 0, activeController->GetNamedAxis("Forward")) * frameSpeed; // Z-axis
			sideMovement = yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed; // X-axis


			// apply impulse force in 'up' direction if the player presses the jump button
			this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, activeController->GetNamedAxis("UpDown") * 3, 0));

		}

		Vector3 totalMovement = forwardMovement + sideMovement;

		t->SetPosition(currentPos + totalMovement);
		t->SetOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));

	}

    void playerCharacter::roombaControls(float dt) {

		joyControls(dt);



    }
}
