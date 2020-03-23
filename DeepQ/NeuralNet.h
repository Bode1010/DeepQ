#pragma once
#include "NeuralNetUtil.h"
#include "NeuralNetwork.h"
#include <fstream>

class NeuralNet : public NeuralNetwork
{
	//number of hash table updates
	int t = 0;
	//Number of iterations, used to determine the frequency of hashtable updates
	int iter = 0;
	int baseT = 50;
	float lambda = 1.1;
	int nextUpdate = baseT;

	vector<Layer> net;
	float growthRate = 0.0038;
	//cost function derivative variable
	float (*cost)(float, float);
	//Cost function derivative
	float CostDerivative(float, float);
	//Print output of the last layer
	void printOutput(int pipe);
	//returns output of the last layer in a specific pipe
	vector<float> getOutput(int pipe);
	//Forward pass through the network
	void feedForward(vector<float> input, int pipe);
	//Gradient descent through a dense layer
	void DenseBackPass(int layerIndex, int pipeIndex);
	//Foward pass through a dense layer
	void DenseForwardPass(int layerIndex, int pipeIndex);
	//Gradient Descent
	void BackPropagate(const vector<float>& output, int pipe);
	//Element wise multiplication of two vectors
	float multVec(const vector<float>&, const vector<float>&);
	//Updates all the hashtables
	void UpdateHashTables();
	//Updates number of iterations and updates hash tables
	void HashUpdateTracker();

	//Debug functions
	void DebugWeights();
	void DebugWeights(int layer);

	//Load functions
	void LoadPrevNetVersion(ifstream);
	void LoadCurrNetVersion(ifstream);

public:
	NeuralNet() {};
	void save(string);
	bool load(string);
	void printOutput();
	float getMaxOutput();
	int getMaxOutputIndex();
	vector<float> getOutput();
	void SetCostFuncDerivative(float (*func)(float, float), float, float);
	void operator=(const NeuralNet& obj);
	void feedForward(const vector<float>& input);
	NeuralNet(vector<Layer>& layout);
	void train(const vector<vector<float>>& input, const vector<vector<float>>& output);
	void trainWithOneOutput(const vector<vector<float>>& inputs, const vector<OneOutput>& outputs);
};

