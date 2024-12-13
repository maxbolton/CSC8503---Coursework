#pragma once
#include "Window.h"
#include "../CSC8503/playerCharacter.h"
#include <iostream>


namespace NCL {
	namespace CSC8503 {

		enum stateNames {
			StartMenu,
			InGame,
			GameOver,
			PauseMenu
		};

		class PushdownMachine;
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState(Window* window, playerCharacter* player) : window(window), player(player) {}
			virtual ~PushdownState() {}

			virtual stateNames GetState() const = 0;

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}

		protected:
			Window* window;
			playerCharacter* player;
		};

		class PauseScreen : public PushdownState {
		public:
			PauseScreen(Window* window, playerCharacter* player) : PushdownState(window, player) {}
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

		class EndGameScreen : public PushdownState {
		public:
			EndGameScreen(Window* window, playerCharacter* player) : PushdownState(window, player) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (window->GetKeyboard()->KeyPressed(KeyCodes::P)) {
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}
			stateNames GetState() const override {
				return stateNames::GameOver;
			}
			void OnAwake() override {
				std::cout << "Game Over!\n";
				std::cout << "Press P to restart!\n";
			}
		};

		class GameScreen : public PushdownState {
		public:
			GameScreen(Window* window, playerCharacter* player) : PushdownState(window, player) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (window->GetKeyboard()->KeyPressed(KeyCodes::P)) {
					*newState = new PauseScreen(window, player);
					return PushdownResult::Push;
				}
				if (player->GetState() == playerState::dead) {
					std::cout << "Player has died!\n";
					*newState = new EndGameScreen(window, player);
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
				std::cout << "Press P to pause or ESC to return to Main Menu!\n";
			}
		};

		class MainMenu : public PushdownState {
		public:
			MainMenu(Window* window, playerCharacter* player) : PushdownState(window, player) {}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (window->GetKeyboard()->KeyPressed(KeyCodes::P)) {
					*newState = new GameScreen(window, player);
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
				std::cout << "Press P to start, or ESC to quit!\n";
			}
		};
	}
}