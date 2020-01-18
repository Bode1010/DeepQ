#pragma once
#ifndef GAME_H
#define GAME_H
#include "Util.h"

//It's an abstract class, but I defined the functions because you can't actually create variables of abstract class, like I need to 
//do in the private section of my Qtable
class Game
{
public:
	Game() {};
	virtual void reset() =0;
	virtual void Display() = 0;
	virtual void endDisplay() = 0;
	virtual int getActionSize() = 0;
	virtual Packet step(Action) = 0;
	virtual State getStartState() =0;
	virtual Action getRandAction(State state) =0;
	~Game() {};
};

#endif