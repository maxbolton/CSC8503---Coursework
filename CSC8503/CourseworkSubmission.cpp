#include "CourseworkSubmission.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "SpringConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "playerCharacter.h"
#include "enemyAI.h"

#include "PushdownMachine.h"
#include "PushdownState.h"
#include "NavigationGrid.h"
#include <NavigationGrid.cpp>



using namespace NCL;
using namespace CSC8503;


bool freecam = false;


Vector3 gridOffset = Vector3(0, -15, -200);

CourseworkSubmission::CourseworkSubmission() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;


	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();

	Window* w = Window::GetWindow();

	useGravity = true;

}

CourseworkSubmission::~CourseworkSubmission() {
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
	
	delete player;
	delete navGrid;
	delete enemy;
	delete kittens[0];
	delete kittens[1];
	delete kittens[2];


}

#pragma region Initialisers
/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void CourseworkSubmission::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	catMesh = renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh = renderer->LoadMesh("Kitten.msh");

	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	beigeTex = renderer->LoadTexture("GoatBeige.png");


	//navGrid = new NavigationGrid("Maze2.txt", gridOffset);
	navGrid = new NavigationGrid("Maze4.txt", gridOffset);

	InitWorld();
	InitCamera();

	menuMachine = PushdownMachine::Create(new MainMenu(Window::GetWindow(), player));
}

void CourseworkSubmission::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-35.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 50, 150));



	GameObject* camera = &world->GetMainCamera();

	// give camera physical volume for smooth camera movement, collision detection etc...
	SphereVolume* cameraSphere = new SphereVolume(1.0f);

	world->GetMainCamera().SetPhysicsObject(new PhysicsObject(&camera->GetTransform(), cameraSphere));
	world->GetMainCamera().GetPhysicsObject()->SetInverseMass(0.5f);


	SpringConstraint* constraint = new SpringConstraint((GameObject*)player, ((Camera*)&world->GetMainCamera()), 1.0f);

	camera->SetCollisionLayer(CollisionLayer::Camera);
	world->AddConstraint(constraint);
	world->AddGameObject(camera);

	lockedObject = nullptr;

	//world->GetMainCamera().SetController(controller);

}

void CourseworkSubmission::InitWorld() {
	world->ClearAndErase();
	physics->Clear();


	//BridgeConstraintTest();
	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	//InitGameExamples();
	InitDefaultFloor();

	player = AddPlayerToWorld(Vector3(30, -11, -135));
	player->SetCollisionLayer(CollisionLayer::Player);
	player->GetPhysicsObject()->SetInverseMass(0.5f);
	player->SetController(controller);
	physics->SetPlayer(player);


	BuildMazeFromGrid(gridOffset);
	InitBallPit(Vector3(-100 , -10, -100));
	//InitMixedGridAtOrigin(10, 10, 10, 10, Vector3(-145, -5, -145));
	
	enemy = AddEnemyToWorld(Vector3(50, -11, -130));
	enemy->SetPlayer(player);
	enemy->SetNavGrid(navGrid);
	enemy->GetPhysicsObject()->SetInverseMass(0.1f);


	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		Vector3 pos = randomMazePos();
		kittens[i] = AddKittenToWorld(navGrid->GetWorldPos(pos.x, pos.z));
	}

	useGravity = true;
	physics->UseGravity(useGravity); 

}

void CourseworkSubmission::resetGame() {
	// place player back at start
	player->GetTransform().SetPosition(Vector3(30, -11, -135));

	//empty kitten array
	for (int i = 0; i < 3; i++) {
		kittens[i] = nullptr;
	}

	// delete kittens
	for (int i = 0; i < 3; i++) {
		world->RemoveGameObject(kittens[i]);
	}

	// place new kittens
	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		Vector3 pos = randomMazePos();
		kittens[i] = AddKittenToWorld(navGrid->GetWorldPos(pos.x, pos.z));
	}

	// reset enemy
	enemy->GetTransform().SetPosition(Vector3(50, -11, -130));

	std::cout << "Game reset!" << std::endl;

}

#pragma endregion

void CourseworkSubmission::UpdateGame(float dt) {

	//draw lastRay
	if (selectionObject != nullptr)
		Debug::DrawLine(lastRay->GetPosition(), selectionObject->GetTransform().GetPosition(), Debug::MAGENTA);

	if (freecam) {
		world->GetMainCamera().SetController(controller);
		world->GetMainCamera().UpdateCamera(dt);
	}
	else {
		player->Update(dt);
		world->GetMainCamera().followPlayer(dt);
	}
	
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = Matrix::Inverse(temp);

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

	UpdateKeys();

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::M)) {
		Vector3 destination(50, 0, 50); // Example destination
		//MovePlayerTo(destination);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}
	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	if (menuMachine->GetActiveState() == stateNames::GameOver && Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {

		player->SetState(playerState::defaultState);
		resetGame();
	}

	if (inSelectionMode)
		SelectObject();

	MoveSelectedObject();


	if (testStateObject)
		testStateObject->Update(dt);

	world->UpdateWorld(dt);
	physics->Update(dt);

	enemy->Update(dt);


	//Debug::UpdateRenderables(dt);

	//This year we can draw debug textures as well!
	//Debug::DrawTex(*basicTex, Vector2(10, 10), Vector2(5, 5), Debug::MAGENTA);
}

void CourseworkSubmission::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		player->SetState(inSelectionMode ? playerState::roombaMode : playerState::defaultState);


		// Show/hide the cursor and lock it to the window
		Window::GetWindow()->ShowOSPointer(inSelectionMode);
		// Lock/Unlock the mouse to the window
		Window::GetWindow()->LockMouseToWindow(!inSelectionMode);

	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::L)) {
		freecam = !freecam;
		InitCamera();
	}


	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void CourseworkSubmission::UIManager(float dt) {

	renderer->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);

	switch (menuMachine->GetActiveState()) {
	case stateNames::InGame:

		/*
		if (useGravity) { Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED); }
		else { Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED); }
		*/

		if (inSelectionMode) { Debug::Print("Press Q to change to camera mode!", Vector2(5, 85)); }
		else { Debug::Print("Press Q to change to select mode!", Vector2(5, 85)); }

		/*
		Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
		*/

		// print score in top right corner
		Debug::Print(("Kittens Rescued: " + std::to_string(player->getKittens()->size()) + "/3"), Vector2(50, 5), Debug::RED);

		break;
	case stateNames::PauseMenu:
		Debug::Print("Press P to unpause game!", Vector2(5, 85));
		break;
	case stateNames::StartMenu:
		Debug::Print("MAIN MENU!", Vector2(5, 85));
		break;
	case stateNames::GameOver:
		Debug::Print("GAME OVER!", Vector2(5, 85), Debug::RED);
		Debug::Print("Final Score: " + std::to_string(player->getKittens()->size()) + "/3", Vector2(5, 80), Debug::YELLOW);
		Debug::Print("Press P to restart!", Vector2(5, 75), Debug::YELLOW);
		break;
	}

}

void CourseworkSubmission::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void CourseworkSubmission::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}


/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* CourseworkSubmission::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	floor->SetCollisionLayer(CollisionLayer::Terrain);

	Vector3 floorSize = Vector3(300, 2, 300);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* CourseworkSubmission::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* CourseworkSubmission::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

playerCharacter* CourseworkSubmission::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	playerCharacter* character = new playerCharacter();
	SphereVolume* volume = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize*5, meshSize*5, meshSize*5))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, beigeTex, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

enemyAI* CourseworkSubmission::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 5.0f;
	float inverseMass = 0.5f;

	enemyAI* character = new enemyAI();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, beigeTex, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->SetCollisionLayer(CollisionLayer::AI);

	world->AddGameObject(character);

	return character;
}

GameObject* CourseworkSubmission::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(1.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* CourseworkSubmission::AddKittenToWorld(const Vector3& position) {

	GameObject* kitten = new GameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	kitten->SetBoundingVolume((CollisionVolume*)volume);


	kitten->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(Vector3(position.x, -11, position.z));

	kitten->SetRenderObject(new RenderObject(&kitten->GetTransform(), kittenMesh, beigeTex, basicShader));
	kitten->SetPhysicsObject(new PhysicsObject(&kitten->GetTransform(), kitten->GetBoundingVolume()));

	kitten->GetPhysicsObject()->SetInverseMass(1.0f);
	kitten->GetPhysicsObject()->InitSphereInertia();

	kitten->SetCollisionLayer(CollisionLayer::Objects);

	world->AddGameObject(kitten);

	return kitten;

}

void CourseworkSubmission::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void CourseworkSubmission::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void CourseworkSubmission::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void CourseworkSubmission::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}


void CourseworkSubmission::InitMixedGridAtOrigin(int numRows, int numCols, float rowSpacing, float colSpacing, Vector3 origin) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = origin + Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}


void CourseworkSubmission::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}


void CourseworkSubmission::BuildMazeFromGrid(Vector3 origin) {
	int nodeSize = navGrid->GetNodeSize();
	int x = origin.x;
	int y = origin.y;
	int z = origin.z;

	float wallHeight = 7.0f;
	float wallThickness = 0.5f;

	for (int y = 0; y < navGrid->GetGridHeight(); ++y) {
		for (int x = 0; x < navGrid->GetGridWidth(); ++x) {
			const GridNode* node = navGrid->GetNode(x, y);
			if (node && node->type == WALL_NODE) {
				Vector3 position = origin + Vector3(x * nodeSize, 0, y * nodeSize);
				AddCubeToWorld(position, Vector3(nodeSize / 2.0f, wallHeight, nodeSize / 2.0f), 0.0f);
			}
		}
	}
}

Vector3 CourseworkSubmission::randomMazePos() {
	int nodeSize = navGrid->GetNodeSize();
	int x = rand() % navGrid->GetGridWidth();
	int y = rand() % navGrid->GetGridHeight();


	const GridNode* node = navGrid->GetNode(x, y);
	if (node && node->type == '.') {
		return Vector3(x * nodeSize, 0, y * nodeSize);
	}
	else {
		return randomMazePos();
	}
}

void CourseworkSubmission::InitBallPit(Vector3 origin) {

	float wallLength = 50.0f;
	float wallHeight = 7.0f;


	//north wall
	AddCubeToWorld(origin + Vector3(-5, 0, wallLength), Vector3(wallLength-5, wallHeight, 1), 0.0f);
	//south wall
	AddCubeToWorld(origin + Vector3(0, 0, -wallLength), Vector3(wallLength, wallHeight, 1), 0.0f);
	//east wall
	AddCubeToWorld(origin + Vector3(-wallLength, 0, 0), Vector3(1, wallHeight, wallLength), 0.0f);
	//west wall
	AddCubeToWorld(origin + Vector3(wallLength, 0, 0), Vector3(1, wallHeight, wallLength), 0.0f);


}
/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool CourseworkSubmission::SelectObject() {

	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
		if (selectionObject) {	//set colour to deselected;
			selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			selectionObject = nullptr;
		}

		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			selectionObject = (GameObject*)closestCollision.node;

			selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

			this->SetLastRay(ray);

			return true;
		}
		else {
			return false;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}
	}
	
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void CourseworkSubmission::MoveSelectedObject() {
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}


	}
}

void CourseworkSubmission::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; // how heavy the middle pieces are
	float numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(100, 100, 100);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		block->SetName("Coursework Block");

		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);

		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

StateGameObject* CourseworkSubmission::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(1.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

