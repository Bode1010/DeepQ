#include "QLearn.h"
#include "LizardGame.h"
#include "CarGame.h"

//NOTE: MAKING THE NUMBER OF STEPS BEFORE TARGET NET UPDATE TOO SMALL OR TOO LARGE (RELATIVE TO THE GAME WERE PLAYING) CREATES \
INSTABILITY IN THE NETWORK!!!

//PROBLEM: WHEN WE ARE SUCCEEDING IN GOING AROUND THE TRACK, THE REPLAY MEMORY BECOMES MORE AND MORE POPULATED WITH SUCCESS STATES\
AND BECAUSE OF THIS THE CAR IS TAUGHT TO BELIEVE IT WILL ALWAYS SUCCEED NO MATTER WHICH ACTION IT TAKES AND WILL END UP FAILING.\
ALSO, TANH IS RETURNING NAN

//ONE OF MY RAYHITS IS NAN

//In raytrace function before i start searching for rayhits lock down the player posiiton

//SOLUTION FOR THIS GAME: ONCE THE CAR HITS THE MAX STEPS PER GAME WITHOUT DYING, END TRAINING
int main() {
	//Should the rate at which the targetnet updates be related to the exploration decay rate?
	srand(time(0));
	//Instantiate network template
	vector<Layer> layout;
	layout.push_back(Layer(DENSE, 7, NONE, false));
	layout.push_back(Layer(DENSE, 21, TANH, false));
	layout.push_back(Layer(DENSE, 6, NONE, false));
	
	//Instantiate Game and DQN
	sf::RenderWindow wd(sf::VideoMode(512, 512), "Car Game");
	CarGame env(&wd);
	//LizardGame env;
	QLearn DeepQ(env, layout);

	//Train
	char ans = 'M';
	int trainingReps = 1000;
	if (DeepQ.saveFilePresent()) {
		while (ans != 'Y' && ans != 'N') {
			system("cls");
			cout << "Load trained network? Y/N" << endl;
			cin >> ans;
			if (ans == 'Y') {
				DeepQ.load(); 
			}
			else if (ans == 'N') {
				DeepQ.trainNetwork(trainingReps);
				DeepQ.save();
			}
		}
	}
	else {
		DeepQ.trainNetwork(trainingReps);
		DeepQ.save();
	}
	cout << "Finished training. Hit enter to continue" << endl;
	cin.get();
	cin.get();

	//Play game with trained DQN
	DeepQ.Play();

	cout << "Program Terminated" << endl;
	cin.get();
	return 0;
}