#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>//matrix
#include <cstdlib>//matrix
using namespace std;

//Set up this way because items in the packet could differ,
//state could be a number, list of number, an image etc and so could action
//so I made them their own structs so when I need to change it I could just change it here
struct State {
	vector<float> result;
	void print() {
		for (int i = 0; i < result.size(); i++) {
			cout << result[i] << " ";
		}
		cout << endl;
	}
	State(int num) { 
		for (int i = 0; i < 16; i++) {
			if (i == num)
				result.push_back(1);
			else {
				result.push_back(0);
			}
		}
	}
	State() {};
};

struct Packet {
	State state;
	float reward;
	bool done;
	bool won;
};

struct Action {
	int val;
	Action(int num) { val = num; }
	Action() {};
};

struct Memory {
	Memory() {};
	Memory(State s, Action a, float r, State ns) { state = s; action = a; nextReward = r; nextState = ns; }
	State state;
	Action action;
	float nextReward;
	State nextState;
};

#endif