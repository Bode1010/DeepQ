#pragma once
#include <vector>
#include <iostream>
#include "SimHash.h"
using namespace std;

struct Neuron {
	Neuron() {};
	Neuron(unsigned batchSize);
	Neuron(unsigned batchsize, vector<float> weight);
	Neuron(vector<float> weight);

	vector<float> weight;
	vector<float> gradient;
	vector<float> activation;

	//Add true or false to the end of the active vector
	void pushActive(bool);
	//returns true or false from a position in the active vector
	bool getActive(unsigned);
	//set a position in the active vector to either true or false
	void setActive(unsigned, bool);
	//converts floats to integers
	static int floatToInt(float);
	//Assigns related values based on batch size
	void SetVars(int bSize);
	static float maxW;
private:
	int actCount = 0;
	//stores neurons activeness in bits
	vector<unsigned> active;
	//static fti FloatConverter;
};

enum ActivationFunction {
	TANH, RELU, SIGMOID, SOFTMAX, NONE
};

enum LayerType {
	DENSE, GRU
};

struct Layer {
	Layer() {};
	float activate(float x);
	float dActivate(float x);
	int size() { return mySize; }
	void setSize(int x) { mySize = x; }
	LayerType getLayerType() { return layType; }
	ActivationFunction getActivationFunction() { return actFunc; }
	//Default Hash Table vars are 0 bits, 1 tables and  no neuron limit
	Layer(LayerType l, int layerSize, ActivationFunction func);
	Layer(LayerType l, int layerSize, ActivationFunction func, int neuLim);
	Layer(LayerType l, int layerSize, ActivationFunction func, int Bits, int Tables);
	Layer(LayerType l, int layerSize, ActivationFunction func, int Bits, int Tables, int neuLim);
	
	//public vars
	int neuronLimit = INT_MAX;
	int bits = 0, tables = 1;
	SimHash HashTable;
	vector<Neuron> neuron;
	vector<float> inputAt(int pipe);
	vector<unsigned> intInputAt(int pipe);

private:
	static float TanhActivate(float x);
	static float SigmoidActivate(float x);
	static float ReluActivate(float x);
	static float SoftmaxActivate(float x) { return x; }
	static float NoneActivate(float x) { return x; };
	static float TanhDActivate(float x);
	static float SigmoidDActivate(float x);
	static float ReluDActivate(float x);
	static float SoftmaxDActivate(float x) { return 1; };
	static float NoneDActivate(float x) { return 1; }

	//Layer vars
	ActivationFunction actFunc;
	LayerType layType;
	int mySize;
};

