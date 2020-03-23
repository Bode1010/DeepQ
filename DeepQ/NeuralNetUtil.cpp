#include "NeuralNetUtil.h"

Neuron::Neuron(unsigned batchSize) {
	SetVars(batchSize);
}

Neuron::Neuron(unsigned batchSize, vector<float> w) {
	SetVars(batchSize);
	weight = w;
}

Neuron::Neuron(vector<float> w) {
	weight = w;
}

float Neuron::maxW = 3;

int Neuron::floatToInt(float f) {
	//Whatever the bigNum is when its multiplied by maxweight it should be under max rand(around 32767)
	float bigNum = 1e3;

	if (f >= maxW) {
		maxW = f;
	 }
	else if (f <= -maxW) {
		maxW = -f;
	}

	f *= bigNum;
	unsigned a = f + maxW*bigNum;
	return a;
}

void Neuron::SetVars(int batchSize) {
	active = vector<unsigned>(batchSize / (sizeof(int) * 8) + 1);
	gradient = vector<float>(batchSize);
	activation = vector<float>(batchSize);
}

void Neuron::pushActive(bool num) {
	if (actCount == 0) { active.push_back(num); }
	else { active.back() = pow(2, actCount) * (int)num + active.back(); }
	actCount = (actCount + 1) % (sizeof(int) * 8);
}

bool Neuron::getActive(unsigned loc) {
	if (loc >= active.size() * sizeof(int) * 8) { cout << "bit out of bounds, getactive(x)." << endl; return false; }
	unsigned a = loc % (sizeof(int) * 8);
	loc = loc / (sizeof(int) * 8);
	a = pow(2, a);
	return (active[loc] & a);
}

void Neuron::setActive(unsigned loc, bool num) {
	if (loc >= active.size() * sizeof(int) * 8) { cout << "bit out of bounds, getactive(x)." << endl; return; }
	unsigned a = loc % (sizeof(int) * 8);
	loc = loc / (sizeof(int) * 8);
	a = pow(2, a);

	if ((active[loc] & a) == num) return;
	else {
		if (num) active[loc] += a;
		else active[loc] -= a;
	}
}

Layer::Layer(LayerType l, int layerSize, ActivationFunction func) {
	layType = l;
	actFunc = func;
	mySize = layerSize + 1;//plus bias
	HashTable = SimHash(bits, tables);
}

Layer::Layer(LayerType l, int layerSize, ActivationFunction func, int neuLim) {
	layType = l;
	actFunc = func;
	mySize = layerSize + 1;
	HashTable = SimHash(bits, tables);
	neuronLimit = neuLim;
}

Layer::Layer(LayerType l, int layerSize, ActivationFunction func, int Bits, int Tables) {
	layType = l;
	actFunc = func;
	mySize = layerSize + 1;
	bits = Bits;
	tables = Tables;
	HashTable = SimHash(bits, tables);
}

Layer::Layer(LayerType l, int layerSize, ActivationFunction func, int Bits, int Tables, int neuLim) {
	layType = l;
	actFunc = func;
	mySize = layerSize + 1;
	bits = Bits;
	tables = Tables;
	HashTable = SimHash(bits, tables);
	neuronLimit = neuLim;
}

vector<float> Layer::inputAt(int x) {
	vector<float> result;
	for (int i = 0; i < mySize; i++) {
		if (neuron[i].getActive(x)) result.push_back(neuron[i].activation[x]);
		else result.push_back(0);
	}
	return result;
}

vector<unsigned> Layer::intInputAt(int x) {
	vector<unsigned> result;
	//Dont add the bias
	for (int i = 0; i < mySize - 1; i++) {
		if (neuron[i].getActive(x)) {
			result.push_back(Neuron::floatToInt(neuron[i].activation[x]));
		}
		else { result.push_back(0); }
	}
	return result;
}

float Layer::activate(float x) {
	switch (actFunc) {
	case TANH:
		return Layer::TanhActivate(x);
		break;
	case RELU:
		return Layer::ReluActivate(x);
		break;
	case SIGMOID:
		return Layer::SigmoidActivate(x);
		break;
	case SOFTMAX:
		return Layer::SoftmaxActivate(x);
		break;
	case NONE:
		return Layer::NoneActivate(x);
		break;
	}
}

float Layer::dActivate(float x) {
	switch (actFunc) {
	case TANH:
		return Layer::TanhDActivate(x);
		break;
	case RELU:
		return Layer::ReluDActivate(x);
		break;
	case SIGMOID:
		return Layer::SigmoidDActivate(x);
		break;
	case SOFTMAX:
		return Layer::SoftmaxDActivate(x);
		break;
	case NONE:
		return Layer::NoneDActivate(x);
		break;
	}
}

float Layer::SigmoidActivate(float x) {
	float expon = exp(x);
	float ans = expon / (expon + 1);
	return ans;
}

float Layer::SigmoidDActivate(float x) {
	return x * (1-x);
}

float Layer::ReluActivate(float x) {
	if (x > 0) return x;
	return 0;
}

float Layer::ReluDActivate(float x) {
	if (x > 0) return 1;
	return 0;
}

float Layer::TanhActivate(float x) {
	return tanh(x);
}

float Layer::TanhDActivate(float x) {
	return 1 - x * x;
}
