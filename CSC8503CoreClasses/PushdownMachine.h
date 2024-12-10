#pragma once

#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:

			static PushdownMachine* Create(PushdownState* initialState);

			bool Update(float dt);

			bool IsStackEmpty() const {
				return stateStack.empty();
			};

			int GetActiveState() const {
				if (activeState)
					return activeState->GetState();
				return -1;
			};

		private:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			static PushdownMachine* instance;

			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

