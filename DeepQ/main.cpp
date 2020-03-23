#include "QLearn.h"
#include "LizardGame.h"
#include "CarGame.h"
#include "NeuralNet.h"

//NOTE: MAKING THE NUMBER OF STEPS BEFORE TARGET NET UPDATE TOO SMALL OR TOO LARGE (RELATIVE TO THE GAME WERE PLAYING) CREATES \
INSTABILITY IN THE NETWORK!!!

//implement dynamic replay memory and compare performance

//Time taken 58*30 reps, about 58 mins 3780 secs I think, replay mem size: 500k

//Complete counter disabled. Let's see just how good We can get
int main() {
	//Should the rate at which the targetnet updates be related to the exploration decay rate?
	srand(time(0));

	//LizardGame env;
	sf::RenderWindow wd(sf::VideoMode(512, 512), "Car Game");
	CarGame env(&wd);
	
	//Instantiate network template
	vector<Layer> layout;
	layout.push_back(Layer(DENSE, env.getStateSize(), NONE));
	layout.push_back(Layer(DENSE, 21, TANH, 2, 2));
	layout.push_back(Layer(DENSE, env.getActionSize(), NONE));

	//Instantiate DQN
	NeuralNet temp(layout);
	NeuralNet temp2(temp);
	QLearn DeepQ(env, temp, temp2);

	int duration;

	//Train
	char ans = 'M', an = 'M';
	int trainingReps = 4000;
	while (ans != 'Y' && ans != 'N') {
		system("cls");
		cout << "Load trained network? Y/N" << endl;
		cin >> ans;
		if (ans == 'Y') {
			if (!DeepQ.load("CarGameSaveHash.hnn")) {
				cout << "No found file, Hit enter to train" << endl;
				cin.get();
				ans = 'N';
			}
			else {
				while (an != 'Y' && an != 'N') {
					system("cls");
					cout << "Train loaded Network? Y/N" << endl;
					cin >> an;
				}
			}
		}
		if (ans == 'N' || an == 'Y') {
			auto start = time(0);
			DeepQ.trainNetwork(trainingReps);
			auto end = time(0);
			cout << "Time: " << end - start << endl;
			DeepQ.save("CarGameSaveHash.hnn");
		}
	}
	cout << "Finished training. Hit enter to continue" << endl;
	cin.get();
	cin.get();
	wd.requestFocus();

	//Play game with trained DQN
	DeepQ.Play();
	
	cout << "Program Ended" << endl;
	cin.get();
	return 0;
}