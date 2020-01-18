#ifndef NNet_H
#define NNet_H
#pragma once

#include "Matrix.h"//for matrix math
#include <math.h>//for activation functions
#include <thread>//For multithreading
#include <future>//for myltithreading
#include <string>
#include <utility>
#include <fstream>//for saving and loading
using namespace std;
typedef vector<Matrix> wArray;
typedef pair<float, float> doublef;

/**************************TODO************************************/
//graph of error over time normalized to fit the screen so we can see how good are network is doing. Maybe a viz subclass using SFML?
//Implement batchNorm
//make a way to switch btw dense and convolutional, apply accordingly during backprop

enum Activation {
	NONE, RELU, SIGMOID, TANH
};

enum LayerType {
	DENSE
};

struct OneOutput {
	OneOutput();
	OneOutput(float val1, int index1);
	float val;//value of the output
	int index;//which output it is
};

class Layer
{
public:
	void print();
	/*The first float is it's preactivated value, the second is it's activated*/
	vector<pair<float, float>> neurons;
	LayerType getType();
	float Activate(float x);
	float dActivate(float x);
	Activation getActivation();
	bool getBatchNorm();
	unsigned int size() { return mySize; }
	void operator=(const Layer &obj);
	Layer(LayerType Type, int Size, Activation, bool batchNorm);

	/*BatchNorm Variables*/
	doublef bn;
	/*bn.First is gamma in f(x) = normalizedX * gamma + beta. bn.second is beta */
	vector<float> mean;
	vector<float> stdDev;

private:
	unsigned int mySize;
	bool batchNorm;
	LayerType type;
	Activation activate;

	/*BatchNorm vars*/

	/*Activations*/
	float SigmoidActivate(float);
	float SigmoidDActivate(float);
	float ReLUActivate(float);
	float ReLUDActivate(float);
	float TanhActivate(float);
	float TanhDActivate(float);
};

class NNet
{
public:
	NNet() {};
	NNet(vector<Layer> layout);
	//trains with one output neuron instead of all of them.
	void trainWithOneOutput(vector<vector<float>> inputs, vector<OneOutput> outputs);
	//Updates the weights in the network
	void train(vector<vector<float>> inputs, vector<vector<float>> outputs);
	//Used for forward passes through the network
	void feedForward(vector<float> input);
	//used to assign nets to other nets
	void operator=(const NNet &obj);
	//returns a vector of the output. Used after a forward pass (ie feedforward function)
	vector<float> getOutput();
	//returns the max output index after a forward pass
	int getMaxOutputIndex();
	//returns the max output after a forward pass
	float getMaxOutput();
	//Prints output in a straight line
	void printOutput();
	//Pretty self Explanatory tbh
	bool saveFilePresent();
	void save();
	void load();
	//Prints every weight. Used for debugging
	void visualize();
	~NNet();

private:
	//Default save file
	const string saveFile = "NNetSave.txt";
	//Holds the non activated values of the network
	vector<Layer> network;
	//returns the updates to be made after backpropping some inputs and outputs. returns changes to the weight array and the batch norm if applicable
	pair<wArray, vector<doublef>> FFandBPWithOneOutput(vector<float> input, OneOutput output);
	pair<wArray, vector<doublef>> FFandBP(vector<float> input, vector<float> output);
	//Duplicates the network layout so I can multithread without interfering with other threads
	void feedForwardTrain(vector<float> input, vector<Layer> &layers);
	//Holds weights and biases. Type of layer affects composition of weight matrix
	wArray weights;

	//How fast weights and biases update
	float growthRate = 0.005;
};

#endif
