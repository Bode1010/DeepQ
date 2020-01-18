#include "LizardGame.h"

LizardGame::LizardGame()
{
	for (unsigned int i = 0; i < numHoles; i++) {
		holes[i] = rand() % (boardSize-2) + 1;
	}
	reset();
}

void LizardGame::reset() {
	playerLoc = 0;
	done = false;
	for (unsigned int i = 1; i < boardSize; i++) {
		board[i] = 'F';
	}
	for (unsigned int i = 0; i < numHoles; i++) {
		board[holes[i]] = 'H';
	}
	board[playerLoc] = 'S'; board[boardSize - 1] = 'G';
}

void LizardGame::Display() {
	system("cls");
	cout << "Board:" << endl;
	for (unsigned int i = 0; i < length; i++) {
		for (unsigned int j = 0; j < width; j++) {
			if (playerLoc == width * i + j) {
				cout << "A ";
			}
			else {
				cout << board[width * i + j] << " ";
			}
		}
		cout << endl;
	}
}

void LizardGame::endDisplay() {};

int LizardGame::getActionSize() {
	return 4;
}

State LizardGame::getStartState() {
	return State(0);
}

Packet LizardGame::step(Action act) {
	Packet result;
	result.done = false;
	board[playerLoc] = 'F';
	//Perform action that was input. 0, 1, 2, 3 is w, a, s, d respectively
	if(act.val == 0 && playerLoc >= width) playerLoc -= width;
	else if(act.val == 1 && (playerLoc) % width != 0) playerLoc -= 1;
	else if(act.val == 2 && (playerLoc) < width * (length - 1)) playerLoc += width;
	else if (act.val == 3 && (playerLoc) % width != width - 1) playerLoc += 1;

	result.state = State(playerLoc);
	board[playerLoc] = 'A';
	result.reward = stepReward;
	result.won = false;
	//When to terminate the game. If we hit the goal or a hole
	if (playerLoc == length * width - 1) {
		result.done = true;
		result.reward = goalReward;
		result.won = true;
	}
	for (unsigned int i = 0; i < numHoles; i++) {
		if (playerLoc == holes[i]) {
			result.done = true;
			result.reward = holeReward;
			break;
		}
	}
	done = result.done;
	
	return result;
}

Action LizardGame::getRandAction(State state) {
	//Would use action struct but that's a lot of work so I am using the action index to represent it directly
	vector<int> actions;
	actions.push_back(0);
	actions.push_back(1);
	actions.push_back(2);
	actions.push_back(3);

	bool axedW = false;
	if (playerLoc < width) {
		vector<int> temp;
		temp.push_back(1);
		temp.push_back(2);
		temp.push_back(3);
		actions = temp;
		axedW = true;
	}
	else if (playerLoc > width*(length -1)) {
		vector<int> temp;
		temp.push_back(0);
		temp.push_back(1);
		temp.push_back(3);
		actions = temp;
	}
	if (playerLoc % width == 0) {
		//3 situations, if we axed action 0, if we axed aciton 2, or if we didnt axe any actions
		if (actions.size() == 4) {
			vector<int> temp;
			temp.push_back(0);
			temp.push_back(2);
			temp.push_back(3);
			actions = temp;
		}
		else {
			if (axedW) {
				vector<int> temp;
				temp.push_back(2);
				temp.push_back(3);
				actions = temp;
			}
			else {

				vector<int> temp;
				temp.push_back(0);
				temp.push_back(3);
				actions = temp;
			}
		}
	}
	else if (playerLoc % width == 3) {
		//3 situations, if we axed action 0, if we axed aciton 2, or if we didnt axe any actions
		if (actions.size() == 4) {
			vector<int> temp;
			temp.push_back(0);
			temp.push_back(1);
			temp.push_back(2);
			actions = temp;
		}
		else {
			if (axedW) {
				vector<int> temp;
				temp.push_back(1);
				temp.push_back(2);
				actions = temp;
			}
			else {

				vector<int> temp;
				temp.push_back(0);
				temp.push_back(2);
				actions = temp;
			}
		}
	}
	//return a random action index from action and convert it into an action
	int chosen = rand() % actions.size();
	return Action(actions[chosen]);
}

LizardGame::~LizardGame()
{
}
