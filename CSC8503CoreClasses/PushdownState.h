#pragma once
#include "Window.h"
#include <iostream>


namespace NCL {
	namespace CSC8503 {

		enum stateNames {
			StartMenu,
			InGame,
			PauseMenu
		};


		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState(Window* window) : window(window) {}
			virtual ~PushdownState() {}

			virtual stateNames GetState() const = 0;

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}

		protected:
			Window* window;
		};

		class PauseScreen : public PushdownState {
		public:
			PauseScreen(Window* window) : PushdownState(window) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (window->GetKeyboard()->KeyPressed(KeyCodes::P)) {
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}

			stateNames GetState() const override {
				return stateNames::PauseMenu;
			}

			void OnAwake() override {
				std::cout << "Press P to unpause game!\n";
			}
		};

		class GameScreen : public PushdownState {
		public:
			GameScreen(Window* window) : PushdownState(window) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				pauseReminder -= dt;
				if (pauseReminder < 0) {
					std::cout << "Press P to pause game or escape to quit!\n";
					pauseReminder += 1.0f;
				}
				if (window->GetKeyboard()->KeyPressed(KeyCodes::P)) {
					//paused = true;
					*newState = new PauseScreen(window);
					return PushdownResult::Push;
				}
				if (window->GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}

			stateNames GetState() const override {
				return stateNames::InGame;
			}

			void OnAwake() override {
				std::cout << "Preparing to mine coins!\n";
			}
		protected:
			int coinsMined = 0;
			float pauseReminder = 1;
		};

		class MainMenu : public PushdownState {
		public:
			MainMenu(Window* window) : PushdownState(window) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (window->GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
					*newState = new GameScreen(window);
					return PushdownResult::Push;
				}
				if (window->GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}

			stateNames GetState() const override {
				return stateNames::StartMenu;
			}

			void OnAwake() override {
				std::cout << "Welcome to the game!\n";
				std::cout << "Press SPACE to start, or ESC to quit!\n";
			}
		};
	}
}
