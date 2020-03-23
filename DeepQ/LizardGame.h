#pragma once
#ifndef LIZARDGAME_H
#define LIZARDGAME_H
#include "Game.h"

//4 by 4 grid that has holes and a goal
//State and actions are represented by indexes in the .val part of their struct
class LizardGame : public Game
{
public:
	LizardGame();
	void reset();
	void Display();
	int getStateSize();
	void endDisplay();
	int getActionSize();
	Packet step(Action);
	State getStartState();
	Action getRandAction(State);
	~LizardGame();

private:
	int playerLoc;
	bool done = false;
	static const int width = 4;
	static const int length = 4;
	const static unsigned int numHoles = 3;
	static const int boardSize = length * width;
	char board[boardSize];
	int holes[numHoles];

	const int goalReward = 10;
	const int holeReward = -10;
	const int stepReward = -1;
};

#endif