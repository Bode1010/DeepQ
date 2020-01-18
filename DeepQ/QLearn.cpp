#include "QLearn.h"

QLearn::QLearn(Game &game, vector<Layer> layout) {
	myEnv = &game;
	policy = NNet(layout);
	target = policy;

	//Create space for the repMem
	for (unsigned int i = 0; i < maxRepMemSize; i++) {
		repMem.push_back(Memory());
	}
}

void QLearn::trainNetwork(int numReps) {
	int completeCounter = 0;
	int ART = 100; //Average reward timer: how many times to print the average reward
	int maxCompleteGames = 2;
	float totRew = 0;
	cout << "Count to " << ART << ": " << endl;
	for (int m = 1; m <= numReps; m++) {
		float totalR = 0;
		myEnv->reset();
		State state = myEnv->getStartState();
		Action act;
		unsigned int i = 0;
		for (i = 0; i < maxStepsPerEpisode; i++) {
			/********************Explore or Exploit***********************/
			//Pick a number
			float epsilon = (rand() % 1000) / 1000.0f;

			//If the number is greater than our exploration threshold, exploit our knowlodge when making a decision
			if (epsilon > explorationRate) {
				//exploit
				act = accessPolicyNetwork(state);
			}
			//If the number is less than our exploration threshold, explore the environment by picking a random action
			else {
				//explore
				act = myEnv->getRandAction(state);
			}

			//myEnv->Display();

			//Perform action in the environment and store the next state and reward
			Packet newPack = myEnv->step(act);
			State newState = newPack.state;
			float reward = newPack.reward;
			bool isDone = newPack.done;
			bool won = newPack.won;
			totalR += reward;

			//Add this whole interaction into the replay memory
			PushMemory(Memory(state, act, reward, newState));

			//sample random memories and add the to the batch to be trained on
			vector<vector<float>> inputBatch;
			vector<OneOutput> outputBatch;
			int chosen;
			if (curRepMemSize < sizeOfBatch) chosen = curRepMemSize;
			else chosen = sizeOfBatch;
			for (unsigned int j = 0; j < chosen; j++) {
				//Using curRepMemSize to track the size of the repmem array so that I am always picking memories that exist in the array
				int selected = rand() % curRepMemSize;
				vector<float> input = StateToVector(repMem[selected].state);
				//Add input into inputBatch
				inputBatch.push_back(input);
				//accessTargetNetwork(nextState), use it to calculte the bellmont eq.
				target.feedForward(StateToVector(repMem[selected].nextState));
				float bestNextQVal = target.getMaxOutput();
				//Calc bellmont eq for action in question, push into targetOUtput
				float update = repMem[selected].nextReward + discountRate * bestNextQVal;
				OneOutput newOutput(update, repMem[selected].action.val);
				//add target output to batch
				outputBatch.push_back(newOutput);
			}

			//Update policy Network
			policy.trainWithOneOutput(inputBatch, outputBatch);
			//policy.train(inputBatch, targetBatch);

			//Check to see if it's time to update the targetNetwork yet
			if (targetNetStep >= stepsBeforeUpdate) {
				target = policy;
				targetNetStep = 0;
			}
			targetNetStep++;

			//update  the state of the game
			state = newState;

			if (won) { completeCounter++; break; }
			if (isDone) break;
		}


		//Update the exploration threshold to decay as time goes by
		if (explorationRate >= minExpRate && explorationRate <= maxExpRate)
			explorationRate = (1 - explorationDecayRate) * explorationRate;

		totRew += totalR;
		if (numReps >= ART) {
			if (m % (numReps /ART) == 0) {
				cout << "Average Reward = " << totRew / (numReps / ART) << endl;
				cout << m / (1.0f *numReps / ART) << " ";
				cout << explorationRate << endl;
				policy.printOutput();
				totRew = 0;
			}
		}

		if (completeCounter >= maxCompleteGames) {
			cout << "We won in " << completeCounter << " games. Training complete. Check train function for any grievances." << endl;
			break;
		}
	}

}

void QLearn::PushMemory(Memory m) {
	if (repMemCount >= maxRepMemSize) {
		repMemCount = 0;
	}
	repMem[repMemCount] = m;
	repMemCount++;
	if (curRepMemSize < maxRepMemSize) curRepMemSize++;
}

Action QLearn::accessPolicyNetwork(State state) {
	policy.feedForward(StateToVector(state));
	//Returns the index of te action with the highest Q value
	return Action(policy.getMaxOutputIndex());
}

Action QLearn::accessTargetNetwork(State state) {
	target.feedForward(StateToVector(state));
	return Action(target.getMaxOutputIndex());
}

vector<float> QLearn::feedForward(vector<int> v) {
	vector<float> result;
	for (int i = 0; i < v.size(); i++) {
		result.push_back(v[i]);
	}
	policy.feedForward(result);
	return policy.getOutput();
}

vector<float> QLearn::StateToVector(State state) {
	vector<float> result;
	for (int i = 0; i < state.result.size(); i++) {
		result.push_back(state.result[i]);
	}
	return result;
}

void QLearn::Play() {
	myEnv->reset();
	State state = myEnv->getStartState();

	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1.0f / frameRate) * 1000.0f;

	for (unsigned int i = 0; i < maxStepsPerEpisode; i++) {
		/*Limit fps*/
		startTime = chrono::system_clock::now();
		chrono::duration<double, milli> work_time = startTime - lastFrameTime;
		if (work_time.count() < timeInMilli) {
			std::chrono::duration<double, std::milli> delta_ms(timeInMilli - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		lastFrameTime = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = lastFrameTime - startTime;

		Action act = accessPolicyNetwork(state);
		Packet newPack = myEnv->step(act);

		//Display to screen
		myEnv->Display();

		//Update the state
		state = newPack.state;

	}
	myEnv->endDisplay();
}

bool QLearn::saveFilePresent() { 
	return policy.saveFilePresent(); 
}

void QLearn::load() {
	policy.load();
}

void QLearn::save() {
	policy.save();
}

QLearn::~QLearn()
{
}
