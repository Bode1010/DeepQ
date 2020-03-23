#ifndef QLEARN_H
#define QLEARN_H
#pragma once

#include "Game.h"//To obtain the environment we will be working in
#include "NeuralNetwork.h"//To obtain Neural network
#include <fstream>
#include <vector>
#include <deque>
using namespace std;

//It can't pick invalid moves randomly, so their q values remain 0. when eploiting, if everything else is negative cuz it hasnt found the goal
//it will pick an invalid move. Not completely sure how to solve this or what effect it has on the program, but it works fine as is.
class QLearn
{
public:
	QLearn(Game &env, NeuralNetwork& net, NeuralNetwork& ACopyOfNet);
	//Performs a pass through the policy network. debuggin purposes
	vector<float> feedForward(vector<int> v);
	//Return policy network. debugging puposes
	NeuralNetwork* getPolicy() { return policy; }
	//Trains the network
	void trainNetwork(int reps);
	//Selects the best action for each possible state till termination without updating
	void Play();
	//Load a policy network if previously saved
	bool load(string);
	//Save a policy network
	void save(string);
	~QLearn();

private:
	Game *myEnv;
	//Framerate at which the network plays the game
	const int frameRate = 30;

	//Network vars:
	NeuralNetwork* policy;
	NeuralNetwork* target;
	vector<float> StateToVector(State state);//converts a state to a vector of floats so it can be used as input for the neural network
	void PushMemory(Memory m); //Adds memory to replay memory
	void UpdateMemory(); //Updates the size of the replay memory
	int repMemSize = 500000; //replay memory size, actually defined below
	std::deque<Memory> repMem;
	const int stepsBeforeUpdate = 110; //Numer of steps to be taken during gameplay before the targetNet weights are updated to the policy net weights
	//The larger my minibatch size, the smaller I need to make my neural net growth rate to achieve stable growth. I try to keep a ratio of batchsize 16 to growthrate 0.4
	const int sizeOfBatch = 16; //number of inputs to be chosen from replay memory at a time. formerly 16, research paper used 50
	int targetNetStep = 0; //steps counter till we need to update the target network

	//Adaptive memory Hyperparameters. See section 4 of The Effects of Memory Replay in Reinforcement Learning by Ruishan Lui et al
	//Slows down training and requires more reps, but with this training gives stable results. Used in "UpdateMemory()"
	//nOld controls the noisiness of your reward chart. higher values smooth out your chart and increase time taken by about half
	//a minute for every multiple of 10x
	//low k/nOld ratio (around 5) gives quickly rising and falling reward charts (When using low graph noisiness), 
	//with high k ratios (around 15) having more of a slow and steady burn

	//Number of memories checked every update sampled out of realNOlds
	int sampledNOld = 125;
	const int realNOld = 250;
	//How often the memory updates
	int k = 50;
	//How much the checked memories changed the network last time
	float sigmaOld = 0;
	//How much the checked memories changed the network this time
	float sigmaNew;
	//Total number of steps taken during training
	uint64_t t;

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