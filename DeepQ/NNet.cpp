#include "NNet.h"

OneOutput::OneOutput() {
}

OneOutput::OneOutput(float val1, int index1) {
	val = val1; 
	index = index1;
}

Layer::Layer(LayerType Type, int Size, Activation act, bool batchNorm) {
	type = Type; mySize = Size; activate = act; this->batchNorm = batchNorm;
	bn.first = 1;
	bn.second = 0;
	for (unsigned int i = 0; i < mySize; i++) {
		neurons.push_back(pair<float, float>(0, 0));
		mean.push_back(0);
		stdDev.push_back(1);
	}
}

void Layer::operator=(const Layer &obj) {
	this->type = obj.type; 
	this->activate = obj.activate; 
	this->mySize = obj.mySize; 
	this->neurons = obj.neurons;
	this->bn = obj.bn;
	this->batchNorm = obj.batchNorm;
	this->mean = obj.mean;
	this->stdDev = obj.stdDev;
}

Activation Layer::getActivation() { return activate; }

LayerType Layer::getType() { return type; }

float Layer::Activate(float x) {
	if (activate == SIGMOID) return SigmoidActivate(x);
	else if (activate == RELU) return ReLUActivate(x);
	else if (activate == TANH) return TanhActivate(x);
	else if (activate == NONE) return (x);
	else {
		cout << activate << " is not a viable activation, check Activate function in Layer class." << endl;
	}
}

float Layer::dActivate(float x) {
	if (activate == SIGMOID) return SigmoidDActivate(x);
	else if (activate == RELU) return ReLUDActivate(x);
	else if (activate == TANH) return TanhDActivate(x);
	else if (activate == NONE) return (1);
	else {
		cout << activate << " is not a viable activation, check Activate function in Layer class." << endl;
	}
}

void Layer::print() {
	for (int i = 0; i < neurons.size(); i++) {
		cout << neurons[i].second << " ";
	}
	cout << endl;
}

bool Layer::getBatchNorm() {
	return batchNorm;
}

NNet::NNet(vector<Layer> Layout) {
	network = Layout;
	for (unsigned int i = 0; i < Layout.size(); i++) {
		network[i].bn.first = 1;
		network[i].bn.second = 0;
		//initalize weights and biases with random values
		if (i > 0) {
			Matrix tempWeight(Layout[i].neurons.size(), Layout[i - 1].neurons.size(), true);
			Matrix tempBias(Layout[i].neurons.size(), 1, true);
			weights.push_back(tempWeight);
			weights.push_back(tempBias);
		}
	}
}

//Layers are stored unActivated unless batchNorm is applied
//takes a duplicated vector of layers(network) and feeds forward through it
void NNet::feedForwardTrain(vector<float> input, vector<Layer> &layers) {
	try {
		if (input.size() != layers[0].neurons.size()) {
			throw - 3;
		}

		//Push input into first layer of NNet
		for (unsigned int i = 0; i < input.size(); i++) {
			layers[0].neurons[i].second = input[i];
			if (layers[0].getBatchNorm()) {
				float temp = (input[i] - layers[0].mean[i]) / layers[0].stdDev[i];
				layers[0].neurons[i].second = layers[0].bn.first * temp - layers[0].bn.second;
			}
		}

		//for all subsequent layers, 
		for (unsigned int i = 1; i < layers.size(); i++) {
			/*Convert previous layer to matrix*/
			Matrix prevLayer(layers[i - 1].neurons.size(), 1, false);
			for (unsigned int j = 0; j < layers[i - 1].neurons.size(); j++) {
				prevLayer.vals[j][0] = layers[i - 1].neurons[j].second;
			}
			/*multiply weight matrix by prevuiys layer matrix, and add bias matrix*/
			Matrix layer = weights[2 * (i - 1)] * prevLayer + weights[2 * (i - 1) + 1];

			//If layer isn't batch normalized
			if (!layers[i].getBatchNorm()) {
				/*Push resulting matrix into corresponding layer of NNet*/
				for (unsigned int j = 0; j < layers[i].neurons.size(); j++) {/*If i did this right, then layer(matrix) size == layers[i] size*/
					layers[i].neurons[j].first = layer.vals[j][0];
					layers[i].neurons[j].second = layers[i].Activate(layer.vals[j][0]);
				}
			}

			/*If layer is batch normalized*/
			//Then the neurons preactivated val(first) is its activated val and its activated val(second) is its batchnorm val
			else {
				for (unsigned int j = 0; j < layers[i].neurons.size(); j++) {
					layers[i].neurons[j].first = layers[i].Activate(layer.vals[j][0]);
					float temp = (layers[i].neurons[j].first - layers[i].mean[j]) / layers[i].stdDev[j];
					layers[i].neurons[j].second = layers[i].bn.first * temp - layers[i].bn.second;
				}
			}
		}
	}
	catch (int e) {
		cout << "Input size does not match given size in feedForward function. Error no. " << e << endl;
	}
}

void NNet::feedForward(vector<float> input) {
	try {
		if (input.size() != network[0].neurons.size()) {
			throw - 3;
		}

		//Push input into first layer of NNet
		for (unsigned int i = 0; i < input.size(); i++) {
			network[0].neurons[i].second = input[i];
			if (network[0].getBatchNorm()) {
				float temp = (input[i] - network[0].mean[i]) / network[0].stdDev[i];
				network[0].neurons[i].second = network[0].bn.first * temp - network[0].bn.second;
			}
		}

		//for all subsequent layers, 
		for (unsigned int i = 1; i < network.size(); i++) {
			/*Convert previous layer into matrix*/
			Matrix prevLayer(network[i - 1].neurons.size(), 1, false);
			for (unsigned int j = 0; j < network[i - 1].neurons.size(); j++) {
				prevLayer.vals[j][0] = network[i - 1].neurons[j].second;
			}
			/*multiply weight matrix by layer matrix, and add bias matrix*/
			Matrix layer = weights[2 * (i - 1)] * prevLayer + weights[2 * (i - 1) + 1];

			//If layer isn't batch normalized
			if (!network[i].getBatchNorm()) {
				/*Push resulting matrix into corresponding layer of NNet*/
				for (unsigned int j = 0; j < network[i].neurons.size(); j++) {/*If i did this right, then layer(matrix) size == network[i] size*/
					network[i].neurons[j].first = layer.vals[j][0];
					network[i].neurons[j].second = network[i].Activate(layer.vals[j][0]);
				}
			}

			/*If layer is batch normalized*/
			//Then the neurons preactivated val(first) is its activated val and its activated val(second) is its batchnorm val
			else {
				/*Activate layer*/
				for (unsigned int j = 0; j < network[i].neurons.size(); j++) {
					network[i].neurons[j].first = network[i].Activate(layer.vals[j][0]);
					float temp = (network[i].neurons[j].first - network[i].mean[j]) / network[i].stdDev[j];
					network[i].neurons[j].second = network[i].bn.first * temp - network[i].bn.second;
				}
			}
		}
	}
	catch (int e) {
		cout << "Input size does not match given size in feedForward function. Error no. " << e << endl;
	}
}

//Works with a batch of input. Takes a single step per batch
//Note, for batchnormed layers, their gradient descent d_activates their batchnormed values.
//Insted, we should skip the deactivation for layers that are batch normed
void NNet::train(vector<vector<float>> input, vector<vector<float>> output) {
	if (input.size() != output.size()) {
		cout << "Training input batch != Training output batch" << endl;
	}
	
	/*Calculate the mean and standard deviation for each neuron in each batchnormed layer for each input*/
	vector<vector<Layer>> netVec;
	vector<vector<float>> mean;
	vector<vector<float>> stdDev;
	int curBatchNormLayerCount = 0;
	bool hasBatchNorm = false;
	for (unsigned int i = 0; i < network.size(); i++) {
		if (network[i].getBatchNorm()) {
			hasBatchNorm = true;
			break;
		}
	}
	if (hasBatchNorm) {
		/*Create space to calculate the mean and stdDev for batchNormed layers*/
		for (unsigned int i = 0; i < network.size(); i++) {
			if (network[i].getBatchNorm()) {
				mean.push_back(vector<float>());
				stdDev.push_back(vector<float>());
				for (unsigned int j = 0; j < network[i].size(); j++) {
					mean.back().push_back(0);
					stdDev.back().push_back(0);
				}
			}
		}

		//feed forward all the input
		//Use a vector of 'networks' to store all the different layers after forward propagation

		//IM ACCESSING THEIR POST NORMALIZED VALS!!!!!\
		WE MIGHT NEED TO CREATE A NEW LAYER SPECIFICALLY FOR BATCH NORM OR SMTH\
		OR SPLIT A NEURON INTO IT'S PREACTIVATED, ACTIVATED AND NORMALIZED VALS TO\
		HELP WITH THE CALCULATION\
		CANNOT CREATE A NEW FEED FORWARD CUZ IT WILL MESS UP FUTURE LAYERS

		for (unsigned int i = 0; i < input.size(); i++) {
			curBatchNormLayerCount = 0;
			netVec.push_back(network);
			feedForwardTrain(input[i], netVec.back());
			//for each input using a vector<vector<float>> mean, sum all the neurons vals in relevant layers
			/*Check to see if each layer is batchnormed*/
			for (unsigned int j = 0; j < network.size(); j++) {
				if (network[j].getBatchNorm()) {
					/*Add up the different neuron values to calc the mean*/
					for (unsigned int k = 0; k < network[j].neurons.size(); k++) {
						mean[curBatchNormLayerCount][k] += netVec.back()[j].neurons[k].first;
					}
					curBatchNormLayerCount++;
				}
			}
		}

		//divide all the sums by the size of the batch to get the mean
		//set the batchnormed layers means accordingly
		curBatchNormLayerCount = 0;
		for (unsigned int i = 0; i < network.size(); i++) {
			if (network[i].getBatchNorm()) {
				for (unsigned int j = 0; j < network[i].size(); j++) {
					mean[curBatchNormLayerCount][j] /= input.size();
					network[i].mean[j] = mean[curBatchNormLayerCount][j];
				}
				curBatchNormLayerCount++;
			}
		}

		//go through all all the neurons of the batch normalized layers, adding up their stdDev in a vector<vector<float>> stdDev
		for (unsigned int i = 0; i < input.size(); i++) {
			curBatchNormLayerCount = 0;
			for (unsigned int j = 0; j < network.size(); j++) {
				if (network[j].getBatchNorm()) {
					for (unsigned int k = 0; k < network[j].size(); k++) {
						stdDev[curBatchNormLayerCount][k] += (netVec[i][j].neurons[k].first - network[j].mean[k]) * (netVec[i][j].neurons[k].first - network[j].mean[k]);
					}
					curBatchNormLayerCount++;
				}
			}
		}

		//Divide std dev vals by size of input then sqrt them
		//set the batchnormed layers stdDevs accordingly
		curBatchNormLayerCount = 0;
		for (unsigned int i = 0; i < network.size(); i++) {
			if (network[i].getBatchNorm()) {
				for (unsigned int j = 0; j < network[i].size(); j++) {
					stdDev[curBatchNormLayerCount][j] /= input.size();
					stdDev[curBatchNormLayerCount][j] = sqrt(stdDev[curBatchNormLayerCount][j]);
					network[i].stdDev[j] = stdDev[curBatchNormLayerCount][j];
				}
				curBatchNormLayerCount++;
			}
		}

		/*for (unsigned int i = 0; i < network.size(); i++) {
			if (network[i].getBatchNorm()) {
				for (unsigned int j = 0; j < network[i].size(); j++) {
					cout << "Mean for neuron " << j << " :" << network[i].mean[j] << endl;
					cout << "stdDev for neuron " << j << " :" << network[i].stdDev[j] << endl;
				}
			}
		}
		cout << "Gamma: " << network[1].bn.first << ", Beta: " << network[1].bn.second << endl;
		cout << endl;*/
	}

	//Create threads for each input stored in a vector of threads, feed forward and back propagate and calculate changes to be made
	vector<future<pair<wArray, vector<doublef>>>> updateCalcThreads;
	for (unsigned int i = 0; i < input.size(); i++) {
		updateCalcThreads.push_back(async(launch::async, &NNet::FFandBP, this, input[i], output[i]));
	}

	//Create a space to add all the weight changes together
	vector<Matrix> change = weights;
	for (int i = 0; i < change.size(); i++) {
		change[i].clear();
	}

	/*Create a space to hold all the batchNorm variables changes together*/
	vector<doublef> tempbn;
	for (int i = 0; i < network.size(); i++) {
		tempbn.push_back(doublef());
		tempbn[i].first = 0;
		tempbn[i].second = 0;
	}

	/*Apply average of all the weight changes to the weight matrix*/
	for (int i = input.size() - 1; i >= 0; i--) {
		pair<wArray, vector<doublef>> temp;
		/*get the return pair from each thread*/
		temp = updateCalcThreads[i].get();

		/*Add all the changes to all the weights together*/
		for (int j = 0; j < change.size(); j++) {
			change[j] = change[j] + temp.first[j];
		}

		/*Add all the changes to all the batchNorm variables together*/
		for (int j = 0; j < network.size(); j++) {
			tempbn[j].first += temp.second[j].first;
			tempbn[j].second += temp.second[j].second;
		}
	}

	/*apply changes to weights and biases here. weights - growthRate* the average change to the weights*/
	for (unsigned int i = 0; i < change.size(); i++) {
		weights[i] = weights[i] - ((change[i] / output.size()) * growthRate);
	}

	/*apply changes to gammas and betas of all layers batchnorms here.*/
	for (unsigned int i = 0; i < network.size(); i++) {
		if (network[i].getBatchNorm()) {
			network[i].bn.first -= ((tempbn[i].first / output.size()) * growthRate);
			network[i].bn.second -= ((tempbn[i].second / output.size()) * growthRate);
		}
	}
}

//Trains the network with only one output neuron. Usually used on networks with more than one output neuron. Works with batches
void NNet::trainWithOneOutput(vector<vector<float>> input, vector<OneOutput> output) {
	//Create threads for each input stored in a vector of threads, feed forward and back propagate and calculate changes to be made
	vector<future<pair<wArray, vector<doublef>>>> updateCalcThreads;
	for (unsigned int i = 0; i < input.size(); i++) {
		updateCalcThreads.push_back(async(launch::async, &NNet::FFandBPWithOneOutput, this, input[i], output[i]));
	}
	//Create a space to add all the weight changes together
	vector<Matrix> change = weights;
	for (int i = 0; i < change.size(); i++) {
		change[i].clear();
	}

	/*Create a space to hold all the batchNorm variables changes together*/
	vector<doublef> tempbn;
	for (int i = 0; i < network.size(); i++) {
		tempbn.push_back(doublef());
	}

	/*Apply average of all the weight changes to the weight matrix*/
	for (int i = input.size() - 1; i >= 0; i--) {
		pair<wArray, vector<doublef>> temp;
		/*get the return pair from each thread*/
		temp = updateCalcThreads[i].get();

		/*Add all the changes to all the weights together*/
		for (int j = 0; j < change.size(); j++) {
			change[j] = change[j] + temp.first[j];
		}

		/*Add all the changes to all the batchNorm variables together*/
		for (int j = 0; j < network.size(); j++) {
			tempbn[j].first += temp.second[j].first;
			tempbn[j].second += temp.second[j].second;
		}
	}

	/*apply changes to weights and biases here. weights - growthRate* the average change to the weights*/
	for (unsigned int i = 0; i < change.size(); i++) {
		weights[i] = weights[i] - ((change[i] / output.size()) * growthRate);
	}

	/*apply changes to gammas and betas of all layers batchnorms here.*/
	for (unsigned int i = 0; i < network.size(); i++) {
		network[i].bn.first -= ((tempbn[i].first / output.size()) * growthRate);
		network[i].bn.second -= ((tempbn[i].second / output.size()) * growthRate);
	}
}

//duplicates original network so it can work independently, feeds forwards and calculates the changes that needs to be made
pair<wArray, vector<doublef>> NNet::FFandBP(vector<float> input, vector<float> output) {
	pair<wArray, vector<doublef>> result;

	/*Create space to hold the weight changes*/
	vector<Matrix> change = weights;
	for (int i = 0; i < change.size(); i++) {
		change[i].clear();
	}

	/*our layers var copies the layout of the original network in the private part of the class*/
	vector<Layer> layers = network;

	/*used to store the change values of each neuron*/
	vector<vector<float>> temp;

	/*Create space to hold the batch normalization variable changes*/
	vector<doublef> tempbn;
	for (int i = 0; i < network.size(); i++) {
		tempbn.push_back(doublef());
		temp.push_back(vector<float>());
		for (unsigned int j = 0; j < network[i].size(); j++) {
			temp[i].push_back(0);
		}
		tempbn[i].first = 0;
		tempbn[i].second = 0;
	}

	/*Run input through our duplicated network*/
	feedForwardTrain(input, layers);

	/*Calculate the cost and corresponding change from the last layer*/
	float cost = 0;
	for (int j = 0; j < layers[layers.size() - 1].neurons.size(); j++) {
		float myOut = layers[layers.size() - 1].neurons[j].second;

		/*Calculate the cost for this set of inputs. Cost = (myOutput - expectedOutput)^2. Used for visualization.*/
		cost += 0.5f * (myOut - output[j]) * (myOut - output[j]);
		/*Calculate the changes that would affect the neurons of the output layer.*/
		temp[temp.size() - 1][j] = (-output[j] + myOut) * layers[layers.size() - 1].dActivate(layers[layers.size() - 1].neurons[j].first);
	}

	/*Calculate the weights and bias changes of all layers, including the neuron changes of the hidden layers.*/
	for (int j = layers.size() - 2; j >= 0; j--) {//for every layer
		/*Calculate the bias change*/
		for (unsigned int k = 0; k < layers[j + 1].neurons.size(); k++) {
			change[2 * j + 1].vals[k][0] += 1.0f * layers[j + 1].dActivate(layers[j + 1].neurons[k].first) * temp[j + 1][k];
		}


		float d_1 = 0, d_2 = 0;
		/*Calculate the neuron and weight change*/
		for (unsigned int k = 0; k < layers[j].neurons.size(); k++) {
			/*Calculate weight changes for this neuron.*/
			for (unsigned int l = 0; l < layers[j + 1].neurons.size(); l++) {
				/*Calculate the weight change*/
				change[2 * j].vals[l][k] += 1.0f * layers[j].neurons[k].second * layers[j + 1].dActivate(layers[j + 1].neurons[l].first) * temp[j + 1][l];
				/*Calculate the neuron change*/
				temp[j][k] += 1.0f * weights[2 * j].vals[l][k] * layers[j + 1].dActivate(layers[j + 1].neurons[l].first) * temp[j + 1][l];
			}
			/*If there is batch Normalization on this layer*/
			if (layers[j].getBatchNorm()) {
				tempbn[j].first += temp[j][k] * layers[j].neurons[k].first;
				tempbn[j].second += temp[j][k];
				d_1 += 1.0f * layers[j].bn.first * temp[j][k];
				/****************************CHECK WEBSITE FOR CONFRIMATION OF THIS************************/
				d_2 += 1.0f * layers[j].bn.first * temp[j][k] * layers[j].neurons[k].first;
			}
		}


		if (layers[j].getBatchNorm()) {	
			/*Dear future me, Pray you never have to change this. To understand what these are you are going to need to recalculate
			the derivation of batch normalization, then look at your derivation and think of what these variables are
			as at the moment I cannot think of any way to describe what they do without typing out a paragraph. I could have
			used this paragraph to explain, but im not going to :). Good luck!*/
			/*Here is a hint: https://kevinzakka.github.io/2016/09/14/batch_normalization/ */

			for(int k = 0; k < layers[j].neurons.size(); k++){
				temp[j][k] = 1.0f * (layers[j].neurons.size() * temp[j][k] * layers[j].bn.first - d_1 - layers[j].neurons[k].first * d_2) / (layers[j].neurons.size() * layers[j].stdDev[k]);
			}
		}
	}
	
	result.first = change;
	result.second = tempbn;
	return result;
}

pair<wArray, vector<doublef>> NNet::FFandBPWithOneOutput(vector<float> input, OneOutput out) {
	pair<wArray, vector<doublef>> result;

	/*Create space to hold the weight changes*/
	vector<Matrix> change = weights;
	for (int i = 0; i < change.size(); i++) {
		change[i].clear();
	}

	/*our layers var copies the layout of the original network in the private part of the class*/
	vector<Layer> layers = network;

	/*used to store the change values of each neuron*/
	vector<vector<float>> temp;

	/*Create space to hold the batch normalization variable changes*/
	vector<doublef> tempbn;
	for (int i = 0; i < network.size(); i++) {
		tempbn.push_back(doublef());
		temp.push_back(vector<float>());
		for (unsigned int j = 0; j < network[i].size(); j++) {
			temp[i].push_back(0);
		}
		tempbn[i].first = 0;
		tempbn[i].second = 0;
	}

	/*Run input through our duplicated network*/
	feedForwardTrain(input, layers);

	/*Calculate the cost and corresponding change from the last layer*/
	float cost = 0;
	for (int j = 0; j < layers[layers.size() - 1].neurons.size(); j++) {
		float myOut = layers[layers.size() - 1].neurons[j].second;

		if (j == out.index) {
			/*Calculate the cost for this set of inputs. Cost = (myOutput - expectedOutput)^2. Used for visualization.*/
			cost += 0.5f * (myOut - out.val) * (myOut - out.val);
			/*Calculate the changes that would affect the neurons of the output layer.*/
			temp[temp.size() - 1][j] = (-out.val + myOut) * layers[layers.size() - 1].dActivate(layers[layers.size() - 1].neurons[j].first);
		}
		else {
			temp[temp.size() - 1][j] = 0;
		}
	}

	/*Calculate the weights and bias changes of all layers, including the neuron changes of the hidden layers.*/
	for (int j = layers.size() - 2; j >= 0; j--) {//for every layer
		/*Calculate the bias change*/
		for (unsigned int k = 0; k < layers[j + 1].neurons.size(); k++) {
			change[2 * j + 1].vals[k][0] += 1.0f * layers[j + 1].dActivate(layers[j + 1].neurons[k].first) * temp[j + 1][k];
		}


		float d_1 = 0, d_2 = 0;
		/*Calculate the neuron and weight change*/
		for (unsigned int k = 0; k < layers[j].neurons.size(); k++) {
			/*Calculate weight changes for this neuron.*/
			for (unsigned int l = 0; l < layers[j + 1].neurons.size(); l++) {
				/*Calculate the weight change*/
				change[2 * j].vals[l][k] += 1.0f * layers[j].neurons[k].second * layers[j + 1].dActivate(layers[j + 1].neurons[l].first) * temp[j + 1][l];
				/*Calculate the neuron change*/
				temp[j][k] += 1.0f * weights[2 * j].vals[l][k] * layers[j + 1].dActivate(layers[j + 1].neurons[l].first) * temp[j + 1][l];
			}
			/*If there is batch Normalization on this layer*/
			if (layers[j].getBatchNorm()) {
				tempbn[j].first += temp[j][k] * layers[j].neurons[k].first;
				tempbn[j].second += temp[j][k];
				d_1 += 1.0f * layers[j].bn.first * temp[j][k];
				/****************************CHECK WEBSITE FOR CONFRIMATION OF THIS************************/
				d_2 += 1.0f * layers[j].bn.first * temp[j][k] * layers[j].neurons[k].first;
			}
		}


		if (layers[j].getBatchNorm()) {
			/*Dear future me, Pray you never have to change this. To understand what these are you are going to need to recalculate
			the derivation of batch normalization, then look at your derivation and think of what these variables are
			as at the moment I cannot think of any way to describe what they do without typing out a paragraph. I could have
			used this paragraph to explain, but im not going to :). Good luck!*/
			/*Here is a hint: https://kevinzakka.github.io/2016/09/14/batch_normalization/ */

			for (int k = 0; k < layers[j].neurons.size(); k++) {
				temp[j][k] = 1.0f * (layers[j].neurons.size() * temp[j][k] * layers[j].bn.first - d_1 - layers[j].neurons[k].first * d_2) / (layers[j].neurons.size() * layers[j].stdDev[k]);
			}
		}
	}

	result.first = change;
	result.second = tempbn;
	return result;
}

void NNet::save() {
	char ans = 'M';
	while (ans != 'Y' && ans != 'N') {
		cout << "Save trained network ? Y / N" << endl;
		cin >> ans;
	}
	if (ans == 'Y') {
		ofstream wr(saveFile);
		//First write the number of layers
		wr << network.size() << endl;
		//Then write all the layers
		int bn = 0;
		for (unsigned int i = 0; i < network.size(); i++) {
			if (network[i].getBatchNorm()) {
				bn = 1;
			}
			wr << network[i].getType() << " " << network[i].neurons.size() << " " << network[i].getActivation() << " " << bn << endl;
		}
		//Next write the number of matrices in the weight array
		wr << weights.size() << endl;
		for (unsigned int i = 0; i < weights.size(); i++) {
			//Specify the size of the matrix
			wr << weights[i].vals.size() << " " << weights[i].vals[0].size() << endl;
			//Write the matrix to the file
			for (unsigned int j = 0; j < weights[i].vals.size(); j++) {
				for (unsigned int k = 0; k < weights[i].vals[j].size(); k++) {
					wr << weights[i].vals[j][k] << " ";
				}
			}
		}
		wr.close();
	}
}

bool NNet::saveFilePresent() {
	fstream file;
	file.open(saveFile);
	if (file.fail()) return false;
	return true;
}

void NNet::load() {
	ifstream rd(saveFile);
	vector<Layer> rdlayers;
	wArray rdweights;
	//read number of layers
	int num;
	rd >> num;
	bool batchNorm = true;
	//Push all layers into network
	for (unsigned int i = 0; i < num; i++) {
		int type, size, activation, bn;
		rd >> type;
		rd >> size;
		rd >> activation;
		rd >> bn;
		if (bn == 0) {
			batchNorm = false;
		}
		Activation tempAct = NONE;
		switch (activation) {
		case 0:
			tempAct = NONE;
			break;
		case 1:
			tempAct = RELU;
			break;
		case 2:
			tempAct = SIGMOID;
			break;
		case 3:
			tempAct = TANH;
		}
		LayerType temptype = DENSE;
		switch (type) {
		case 0:
			temptype = DENSE;
			break;
		}
		rdlayers.push_back(Layer(temptype, size, tempAct, batchNorm));
	}
	network = rdlayers;

	//read the number of weights
	rd >> num;

	for (unsigned int i = 0; i < num; i++) {
		//read the size of the matrix
		float x, y;
		rd >> x; rd >> y;
		Matrix temp(x, y, false);
		//Push info into the matrix
		for (unsigned int j = 0; j < x; j++) {
			for (unsigned int k = 0; k < y; k++) {
				rd >> temp.vals[j][k];
			}
		}
		//Add matrix to the weight array
		rdweights.push_back(temp);
	}
	weights = rdweights;
}

void NNet::operator=(const NNet &c) {
	this->network = c.network;
	this->weights = c.weights;
	this->growthRate = c.growthRate;
}

vector<float> NNet::getOutput() {
	vector<float> result;
	vector<Layer> layers = network;
	for (unsigned int i = 0; i < layers[layers.size() - 1].neurons.size(); i++) {
		result.push_back(layers[layers.size() - 1].Activate(layers[layers.size() - 1].neurons[i].second));
	}
	return result;
}

void NNet::printOutput() {
	vector<Layer> layers = network;
	for (unsigned int i = 0; i < layers[layers.size() - 1].neurons.size(); i++) {
		cout << layers[layers.size() - 1].Activate(layers[layers.size() - 1].neurons[i].second) << endl;
	}
}

int NNet::getMaxOutputIndex() {
	int result = 0;
	vector<Layer> layers = network;
	for (unsigned int i = 1; i < layers[layers.size() - 1].neurons.size(); i++) {
		if (layers[layers.size() - 1].neurons[i] > layers[layers.size() - 1].neurons[result]) result = i;
	}
	return result;
}

float NNet::getMaxOutput() {
	vector<Layer> layers = network;
	float result = layers[layers.size() - 1].neurons[0].second;
	for (unsigned int i = 1; i < layers[layers.size() - 1].neurons.size(); i++) {
		if (layers[layers.size() - 1].neurons[i].second > result) result = layers[layers.size() - 1].neurons[i].second;
	}
	return layers[layers.size() - 1].Activate(result);
}

void NNet::visualize() {
	for (int i = 0; i < weights.size(); i++) {
		weights[i].print();
	}
}

NNet::~NNet()
{
}

float Layer::SigmoidActivate(float x) {
	float expon = exp(x);
	float ans = expon / (expon + 1);
	return ans;
}

float Layer::SigmoidDActivate(float x) {
	float expon = exp(x);
	float ans = expon / (expon + 1);
	float newAns = ans * (1 - ans);
	return newAns;

}

float Layer::ReLUActivate(float x) {
	if (x > 0) return x;
	return 0;
}

float Layer::ReLUDActivate(float x) {
	if (x > 0) return 1;
	return 0;
}

float Layer::TanhActivate(float x) {
	return tanh(x);
}

float Layer::TanhDActivate(float x) {
	float temp = cosh(x) * cosh(x);
	return 1.0f / temp;
}