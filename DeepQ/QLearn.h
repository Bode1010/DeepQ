#ifndef QLEARN_H
#define QLEARN_H
#pragma once

#include "Game.h"//To obtain the environment we will be working in
#include "NNet.h"//To obtain Neural network
#include <vector>
using namespace std;

//It can't pick invalid moves randomly, so their q values remain 0. when eploiting, if everything else is negative cuz it hasnt found the goal
//it will pick an invalid move. Not completely sure how to solve this or what effect it has on the program, but it works fine as is.
class QLearn
{
public:
	QLearn(Game &env, vector<Layer> layout);
	//Performs a pass through the policy network
	vector<float> feedForward(vector<int> v);
	//Checks if the network has been trained previously
	bool saveFilePresent();
	//Return policy network
	NNet getPolicy() { return policy; }
	//Trains the network
	void trainNetwork(int reps);
	//Selects the best action for each possible state till termination without updating
	void Play();
	//Load a policy network if previously saved
	void load();
	//Save a policy network
	void save();
	~QLearn();

private:
	Game *myEnv;
	//Framerate at which the network plays the game
	const int frameRate = 30;

	//Network vars:
	NNet policy;
	NNet target;
	vector<float> StateToVector(State state);//converts a state to a vector of floats so it can be used as input for the neural network
	void PushMemory(Memory m); //Adds memory to replay memory
	static const int maxRepMemSize = 500000; //replay memory size
	vector<Memory> repMem;
	int stepsBeforeUpdate = 110; //Numer of steps to be taken during gameplay before the targetNet weights are updated to the policy net weights
	int targetNetStep = 0; //steps counter till we need to update the target network
	int curRepMemSize = 0;
	int sizeOfBatch = 16; //number of inputs to be chosen from replay memory at a time. 2^x for effieciency(16 is 2^4)
	int repMemCount = 0;

	//access functions: returns the best action to take in that state
	Action accessPolicyNetwork(State state);
	Action accessTargetNetwork(State state);

	//Q Learning Vars
	float discountRate = 0.999f;
	float explorationDecayRate = 0.01;
	float explorationRate = 1.0f;
	float minExpRate = 0.01;
	float maxExpRate = 1.f;

	int maxStepsPerEpisode = 500;
};

#endif