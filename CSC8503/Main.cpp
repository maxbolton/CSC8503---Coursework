#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "CourseworkSubmission.h"
#include "NetworkedGame.h"
#include "NetworkBase.h"
#include "NetworkObject.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>


float dt;

vector<Vector3> testNodes;
void TestPathfinding() {
	NavigationGrid grid("Maze2.txt");

	NavigationPath outPath;


	Vector3 startPos(10, 0, 10);
	Vector3 endPos(130, 0, 130);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		// add -200 offset to z to make it visible
		a.z -= 200;
		b.z -= 200;

		//Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}


/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
#pragma region init game
	WindowInitialisation initInfo;
	initInfo.width		= 1280;
	initInfo.height		= 720;
	initInfo.windowTitle = "CSC8503 Game technology!";

	Window*w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised()) {
		return -1;
	}	


	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);


	CourseworkSubmission* g = new CourseworkSubmission();

	PushdownMachine* machine = PushdownMachine::Get();
	machine->Update(0);


	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
#pragma endregion


	while (w->UpdateWindow() && !machine->IsStackEmpty()) {
		dt = w->GetTimer().GetTimeDeltaSeconds();
		
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}
		

		if (!machine->Update(dt)) {
			break;
		}
		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		if (machine->GetActiveState() != stateNames::PauseMenu) {
			g->UpdateGame(dt);
		}



		g->UIManager(dt);

	}


	Window::DestroyGameWindow();
}


/* UNUSED - Replaced with pushdown automata
* Keeping for future reference
*/
void InitMenuStateMachine() {
	StateMachine* MenusStateMachine = new StateMachine();


	State* MainMenu = new State([&](float dt)->void{
		std::cout << "State: Main Menu\n";
		}
	);

	State* PauseMenu = new State([&](float dt)->void {
		std::cout << "State: Pause Menu\n";
		}
	);

	State* Gameplay = new State([&](float dt)->void {
		std::cout << "State: Gameplay\n";
		}
	);

	StateTransition* startGame = new StateTransition(MainMenu, Gameplay, [&](void)->bool {
		return false; }
	);

	StateTransition* pauseGame = new StateTransition(Gameplay, PauseMenu, [&](void)->bool {
		return false; }
	);

	StateTransition* resumeGame = new StateTransition(PauseMenu, Gameplay, [&](void)->bool {
		return false; }
	);

	MenusStateMachine->AddState(MainMenu);
	MenusStateMachine->AddState(PauseMenu);
	MenusStateMachine->AddState(Gameplay);
	MenusStateMachine->AddTransition(startGame);
	MenusStateMachine->AddTransition(pauseGame);
	MenusStateMachine->AddTransition(resumeGame);

	MenusStateMachine->SetState(MainMenu);
}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;

	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for key...\n";
			behaviourTimer = rand() % 100;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			behaviourTimer -= dt;
			if (behaviourTimer <= 0.0f) {
				std::cout << "Found key!\n";
				return Success;
			}
		}
		return state; // will return Ongoing until success
		}
	);

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Going to room...\n";
			state = Ongoing;
		}
		else if (state == Ongoing) {
			distanceToTarget -= dt;
			if (distanceToTarget <= 0.0f) {
				std::cout << "Reached room!\n";
				return Success;
			}
		}
		return state; // will return Ongoing until success
		}
	);

	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Opening door...\n";
			return Success;
		}
		return state; // will return Ongoing until success
		}
	);

	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking For Treasure!\n";
			return Ongoing;
		}
		else if (state == Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "Found Treasure!\n";
				return Success;
			}
			std::cout << "No Treasure!\n";
			return Failure;
		}
		return state;
		}
	);

	BehaviourAction* lookForItems = new BehaviourAction("Look For Items", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking For Items!\n";
			return Ongoing;
		}
		else if (state == Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "Found Items!\n";
				return Success;
			}
			std::cout << "No Items!\n";
			return Failure;
		}
		return state;
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "Starting Behaviour Sequence...\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f);
		}
		if (state == Success) {
			std::cout << "Behaviour Sequence Completed Successfully!\n";
		}
		else {
			std::cout << "Behaviour Sequence Failed!\n";
		}
	}
	std::cout << "All done!\n";
}

class TestPacketReciever : public PacketReceiver {
public:
	TestPacketReciever(std::string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			std::string msg = realPacket->GetStringFromData();
			std::cout << name << " received string message: " << msg << "\n";
		}
	}

protected:
	std::string name;
};

class FullStateReceiver : public PacketReceiver {
public:
	FullStateReceiver(int& lastAcknowledgedState)
		: lastAcknowledgedState(lastAcknowledgedState) {
	}

	void ReceivePacket(int type, GamePacket* payload, int source = -1) override {
		if (type == Full_State) {  // Ensure it's a Full_State packet
			FullPacket* fullPacket = static_cast<FullPacket*>(payload);
			lastAcknowledgedState = fullPacket->fullState.stateID; // Update the integer
		}
	}

private:
	int& lastAcknowledgedState; // Reference to the integer tracking the latest state
};

void TestNetworking() {
	NetworkBase::Initialise();

	TestPacketReciever serverReceiver("Server");
	TestPacketReciever clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; ++i) {
		StringPacket serverPacket("Server says hello!" + std::to_string(i));
		server->SendGlobalPacket(serverPacket);

		StringPacket clientPacket("Client says hello!" + std::to_string(i));
		client->SendPacket(clientPacket);

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	NetworkBase::Destroy();
}
