#pragma once
#include <SFML/Graphics.hpp>//To draw the game
#include <chrono>//To limit Fps
#include <thread>//For sleep function to limit fps
#include <future>//For multiThreading capabilities
#include <fstream>//For file saving capabilities
#include <math.h>
#include "CarGameUtil.h"
#include "Game.h"
#include "NNet.h"

/*Action Index:
//0 = w/up
//1 = a/left
//2 = s/down
//3 = d/right
//4 = wa/up-left
//5 = wd/up-right
//6 = sa/down-left
//7 = sd/down-right
//8 = do Nothing */
//STEP ONLY UPDATES IF IT TAKES LESS THAN HALF A SECOND TO UPDATE
class CarGame : public Game
{
public:
	~CarGame();
	void reset();
	void Display();
	void endDisplay();
	void DrawGameUser();
	int getActionSize();
	Packet step(Action);
	State getStartState();
	void DrawGameAI(NNet & obj);
	CarGame(sf::RenderWindow *wind);
	Action getRandAction(State state);

private:
	/*QLearning vals*/
	float rewardGateReward = 2;
	float hitWallReward = -5;

	/*BackGround functions*/
	void hasCollided();
	void rayTrace();

	/*Utility functions*/
	void save();
	void load();
	void BuildTrack();
	bool hasSavedFile();
	dispPacket getCurFrame();

	/*Raytrace Variables*/
	vector<vec> rays;
	float stepSize = 0.5;
	vector<vec> rayHits;

	/*Game Variables*/
	Agent player;
	bool done = false;
	vector<edge> track;
	vector<edge> rewardGate;
	bool rewardCollide = false;
	int activeRewardIndex = 0;
	int gatesPerTrackPoint = 8;
	sf::RenderWindow *wd;
	bool collided = false;
	string saveFile = "CarGameSave.txt";
	sf::Color trackColor = sf::Color::White;
	int lapsCompleted = 0;
	int finishLineIndex = 0;
	int lapsToWin = 2;

	//WARNING: Car moves slower if framerate is reduced as speed is calculated by frames passed
	float frameRate = 30;

	/*Track Representation variables Variables*/
	const int trackPointSize = 5;
	const int selectedThicc = 2;
	const float d_angle = 0.1;

	/*Player Variables*/
	float pWidth = 4;
	float pLength = 8;
	float maxSpeed = 2;
	float startX = 50;
	float maxPossDist = 50;
	float startY = 50;
	const float accel = 0.08;
	const float deccel = 0.07;
	float maxSteerAng = PI/6;
	float maxReverseSpeed = maxSpeed/2.0;
	float WheelBase = (5.f / 6.0f) * pLength;
	float startingAngle = atan2(pWidth / 2, pLength / 2);
	float distToVert = sqrt((pWidth / 2) * (pWidth / 2) + (pLength / 2) * (pLength / 2));

	/*Threads*/
	thread Coll, RT;

	/*Display vars*/
	int xDim, yDim;
	sf::Texture playerTexture;
	float playWid, playLen;
	sf::RectangleShape Player;
	sf::VertexArray Track, RGates, Rays, activeRewardGate;
	vector<sf::CircleShape> rayHitIndic;
};

