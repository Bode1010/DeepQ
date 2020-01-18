#include "CarGame.h"

CarGame::CarGame(sf::RenderWindow *wind)
{
	wd = wind;

	player.pos = vec(startX, startY);
	player.angle = PI / 2;
	player.length = pLength;
	player.width = pWidth;
	player.vel = vec(0, 0);

	player.colliders = { edge(), edge() };
	//1 in the x axis is up, 1 in the y axis is left
	rays = { vec(1, 0), vec(1, 1), vec(0, 1), vec(0, -1), vec(1, -1), vec(1, 0.5), vec(1, -0.5)};

	for (unsigned int i = 0; i < rays.size(); i++) {
		rayHits.push_back(vec());
	}

	/*Procure a track, by building or importing*/
	if (hasSavedFile()) {
		char ans = 'M';
		while (ans != 'Y' && ans != 'N') {
			system("cls");
			cout << "Track Save File Detected, load? Y/N" << endl;
			cin >> ans;
			if (ans == 'Y') load();
			else if (ans == 'N') { 
				BuildTrack();
				ans = 'H';
				while (ans != 'Y' && ans != 'N') {
					system("cls");
					cout << "Do you want to save this track? It will write over any preiously saved tracks. Y/N" << endl;
					cin >> ans;
					if (ans == 'Y') save();
				}
			}
		}
	}
	else {
		BuildTrack();
		char ans = 'H';
		while (ans != 'Y' && ans != 'N') {
			system("cls");
			cout << "Do you want to save this track? It will write over any preiously saved tracks. Y/N" << endl;
			cin >> ans;
			cin.get();
			if (ans == 'Y') save();
		}
	}

	/*Adjusting the player size, angle and position based on the track*/
	float newX = (track[0].v1.x + track[track.size() / 2].v1.x) / 2;
	float newY = (track[0].v1.y + track[track.size() / 2].v1.y) / 2;
	float dist = sqrt((track[0].v1.y - track[track.size() / 2].v1.y) * (track[0].v1.y - track[track.size() / 2].v1.y) + (track[0].v1.x - track[track.size() / 2].v1.x) * (track[0].v1.x - track[track.size() / 2].v1.x));
	pLength *= 0.06 * dist;
	pWidth *= 0.06 * dist;
	player.width = pWidth;
	player.length = pLength;
	player.pos = vec(newX, newY);
	startX = newX;
	startY = newY;
	maxSpeed *= 0.06 * dist;
	maxSteerAng *= 0.06 * dist;
	maxReverseSpeed *= 0.06 * dist;
	WheelBase = (4.0f / 6.0f) * pLength;
	startingAngle = atan2(pWidth / 2, pLength / 2);
	distToVert = sqrt((pWidth / 2) * (pWidth / 2) + (pLength / 2) * (pLength / 2));
	player.angle = atan2((track[0].v1.y - track[track.size() / 2].v1.y), -(track[0].v1.x - track[track.size() / 2].v1.x)) + PI/2;


	/*Calculate reward Gates*/
	for (unsigned int i = 0; i < track.size() / 2; i++) {
		for (int j = 0; j < gatesPerTrackPoint; j++) {
			//If the average length of this portion of track is less than 5 times the cars length,\
			 half the amount of reward gates by skippig a reward gate
			int gPTP = gatesPerTrackPoint;
			float l1X = (track[(i + 1) % (track.size() / 2)].v1.x - track[i].v1.x);
			float l1Y = (track[(i + 1) % (track.size() / 2)].v1.y - track[i].v1.y);
			float l2X = (track[(track.size() / 2) + (i + 1) % (track.size() / 2)].v1.x - (track[(track.size() / 2) + i].v1.x));
			float l2Y = (track[(track.size() / 2) + (i + 1) % (track.size() / 2)].v1.y - (track[(track.size() / 2) + i].v1.y));
			float length1 = sqrt(l1X * l1X + l1Y * l1Y);
			float length2 = sqrt(l1X * l2X + l2Y * l2Y);
			if ((length1 + length2) / 2 < 5 * pLength) {
				j++;
			}
			vec p1, p2;
			p1.x = track[i].v1.x + j * (track[(i + 1) % (track.size() / 2)].v1.x - track[i].v1.x) / gPTP;
			p1.y = track[i].v1.y + j * (track[(i + 1) % (track.size() / 2)].v1.y - track[i].v1.y) / gPTP;
			p2.x = track[(track.size() / 2) + i].v1.x + j * (track[(track.size() / 2) + (i + 1) % (track.size() / 2)].v1.x - (track[(track.size() / 2) + i].v1.x)) / gPTP;
			p2.y = track[(track.size() / 2) + i].v1.y + j * (track[(track.size() / 2) + (i + 1) % (track.size() / 2)].v1.y - (track[(track.size() / 2) + i].v1.y)) / gPTP;
			rewardGate.push_back(edge(p1, p2));
		}
	}

	/*Start Threads*/
	Coll = thread(&CarGame::hasCollided, this);
	RT = thread(&CarGame::rayTrace, this);


	/*Instatiate display variables for drawing to screen in the display function*/
	xDim = wd->getSize().x;
	yDim = wd->getSize().y;

	if (!playerTexture.loadFromFile("CarTextureFlipped.png")) {
		cout << "Car texture could not load" << endl;
	}
	playerTexture.setSmooth(true);

	playWid = (player.width / 100) * xDim;
	playLen = (player.length / 100) * yDim;

	Player = sf::RectangleShape(sf::Vector2f(playLen, playWid));
	Player.setOrigin(sf::Vector2f(playLen / 2, playWid / 2));
	Player.setTexture(&playerTexture);
	Player.setScale(1.f, -1.f);

	/*Assign Track positions*/
	Track = sf::VertexArray(sf::Lines, 2 * track.size());
	for (unsigned int i = 0; i < 2 * track.size(); i++) {
		if (i % 2 == 0) {
			float tempX = track[i / 2].v1.x / 100 * xDim;
			float tempY = track[i / 2].v1.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		else {
			float tempX = track[i / 2].v2.x / 100 * xDim;
			float tempY = track[i / 2].v2.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		Track[i].color = trackColor;
	}
	
	/*Assign reward gate positions*/
	RGates = sf::VertexArray(sf::Lines, 2 * rewardGate.size());
	for (unsigned int i = 0; i < 2 * rewardGate.size(); i++) {
		if (i % 2 == 0) {
			float tempX = rewardGate[i / 2].v1.x / 100 * xDim;
			float tempY = rewardGate[i / 2].v1.y / 100 * yDim;
			RGates[i].position = sf::Vector2f(tempX, tempY);
		}
		else {
			float tempX = rewardGate[i / 2].v2.x / 100 * xDim;
			float tempY = rewardGate[i / 2].v2.y / 100 * yDim;
			RGates[i].position = sf::Vector2f(tempX, tempY);
		}
		RGates[i].color = trackColor;
	}

	/*Instantiate RayTrace rays*/
	Rays = sf::VertexArray(sf::Lines, 2 * rays.size());

	/*Instantiate Rayhit indicators*/
	for (unsigned int i = 0; i < rayHits.size(); i++) {
		rayHitIndic.push_back(sf::CircleShape(3));
		rayHitIndic[i].setOrigin(1.5, 1.5);
	}

	activeRewardGate = sf::VertexArray(sf::Lines, 2);
}

Action CarGame::getRandAction(State state) {
	return Action(rand() % 6);
}

/*State consists of: NORMALIZE INPUT
//Eight car sensors(The rays that project from the car
//Car Velocity size
//Car Velocity Orientation
//Car Orientation
//Car Length
//Car Width
*/
State CarGame::getStartState() {
	reset();

	//Pick a random reward gate
	int chosenRewardGate = rand() % rewardGate.size();
	activeRewardIndex = (chosenRewardGate + 1) % rewardGate.size();

	//start the player at that position and orietation
	float newX = (rewardGate[chosenRewardGate].v1.x + rewardGate[chosenRewardGate].v2.x) / 2;
	float newY = (rewardGate[chosenRewardGate].v1.y + rewardGate[chosenRewardGate].v2.y) / 2;
	//float dist = sqrt((rewardGate[chosenRewardGate].v1.y - rewardGate[chosenRewardGate].v2.y) * (rewardGate[chosenRewardGate].v1.y - rewardGate[chosenRewardGate].v2.y) + (rewardGate[chosenRewardGate].v1.x - rewardGate[chosenRewardGate].v2.x) * (rewardGate[chosenRewardGate].v1.x - rewardGate[chosenRewardGate].v2.x));

	player.pos = vec(newX, newY); 
	player.angle = atan2((rewardGate[chosenRewardGate].v1.y - rewardGate[chosenRewardGate].v2.y), -(rewardGate[chosenRewardGate].v1.x - rewardGate[chosenRewardGate].v2.x)) + PI / 2;
	player.vel = vec(0, 0);
	if (isnan(player.angle)) {
		cout << "player angle is nan, check get start state" << endl;
	}

	State result;
	unsigned int i = 0;
	float dist = 0;
	for (i = 0; i < rayHits.size(); i++) {
		//Calculate the length of each ray hit
		float X1 = rayHits[i].x - player.pos.x;
		float Y1 = rayHits[i].y - player.pos.y;
		X1 = X1 * X1;
		Y1 = Y1 * Y1;
		dist = sqrt(X1 + Y1);
		if(isnan(dist)) {
			cout << "dist variable is nan, check getstartstate" << endl;
			cout << "rayhit[i]: " << rayHits[i].x << ", " << rayHits[i].y << endl;
			cout << "player.pos: " << player.pos.x << ", " << player.pos.y << endl;
			cout << "Pre sqrt: " << X1 + Y1 << endl;
			cout << "sqrt: " << sqrt(X1 + Y1) << endl;
			cout << "dist: " << dist << endl;
		}
		result.result.push_back(dist);
	}
	return result;
}

int CarGame::getActionSize() {
	return 6;
}

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
Packet CarGame::step(Action act) {
	Packet result;
	result.reward = 0;
	done = false;
	//Because distance covered is calculated per time step, distance is speed * 1 which is speed
	//When I turn, I deccel the car (or accel it) first. then I move the back wheel forward(or back in the direction it was going

	/*Perform Action*/
	if (act.val == 0) {
		//Add acceleration to current speed
		if (player.vel.length() < maxSpeed) {
			player.vel.add(accel, player.angle);
		}
		//If we press up and we are at max speed position updates regardless
		player.pos.x += player.vel.x;
		player.pos.y -= player.vel.y;
	}
	/*else if (act.val == 1) {
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		//Decelerate
		if (player.vel.length() > 0.05) {
			player.vel.add(-deccel * velLen);
			velLen = player.vel.length();

			//Turn:
			// Get location of front and back wheels
			float tempLength = WheelBase;
			vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
			vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

			//Get their locations after turning them
			float velAng = player.vel.getAngle();
			backWheelLoc.x += velLen * cos(velAng);
			backWheelLoc.y -= velLen * sin(velAng);
			frontWheelLoc.x += velLen * cos(velAng + maxSteerAng);
			frontWheelLoc.y -= velLen * sin(velAng + maxSteerAng);

			//update car location
			player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
			player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
			player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));


			//Update car velocity Direction
			player.vel.x = velLen * cos(velAng + (player.angle - temp));
			player.vel.y = velLen * sin(velAng + (player.angle - temp));
		}
	}
	*/
	else if (act.val == 1) {
		if (player.vel.length() < maxReverseSpeed || (player.vel.getAngle() >= player.angle - 0.3 && player.vel.getAngle() <= player.angle + 0.3)) {
			player.vel.add(-accel, player.angle);
		}
		player.pos.x += player.vel.x;
		player.pos.y -= player.vel.y;

	}
	/*else if (act.val == 3) {
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		//Decelerate
		if (velLen > 0.05) {
			player.vel.add(-deccel * velLen);
			velLen = player.vel.length();

			//Turn:
			// Get location of front and back wheels
			float tempLength = WheelBase;
			vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
			vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

			//Get their locations after turning them
			float velAng = player.vel.getAngle();
			backWheelLoc.x += velLen * cos(velAng);
			backWheelLoc.y -= velLen * sin(velAng);
			frontWheelLoc.x += velLen * cos(velAng - maxSteerAng);
			frontWheelLoc.y -= velLen * sin(velAng - maxSteerAng);

			//update car location
			player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
			player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
			player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));

			//Update car velocity
			player.vel.x = velLen * cos(velAng + (player.angle - temp));
			player.vel.y = velLen * sin(velAng + (player.angle - temp));
		}

	}
	*/
	else if (act.val == 2) {
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		//Add acceleration to current speed
		if (player.vel.length() < maxSpeed) {
			player.vel.add(accel, temp);
		}
		velLen = player.vel.length();

		//Turn:
		// Get location of front and back wheels
		float tempLength = WheelBase;
		vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
		vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

		//Get their locations after turning them
		float velAng = player.vel.getAngle();
		backWheelLoc.x += velLen * cos(velAng);
		backWheelLoc.y -= velLen * sin(velAng);
		frontWheelLoc.x += velLen* cos(velAng + maxSteerAng);
		frontWheelLoc.y -= velLen * sin(velAng + maxSteerAng);

		//update car location
		player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
		player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
		player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));
		
		//Update car velocity
		player.vel.x = velLen * cos(velAng + (player.angle - temp));
		player.vel.y = velLen * sin(velAng + (player.angle - temp));
	}
	else if (act.val == 3) {
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		//Add acceleration to current speed
		if (player.vel.length() < maxSpeed) {
			player.vel.add(accel, temp);
		}
		velLen = player.vel.length();

		//Turn:
		// Get location of front and back wheels
		float tempLength = WheelBase;
		vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
		vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

		//Get their locations after turning them
		float velAng = player.vel.getAngle();
		backWheelLoc.x += velLen * cos(velAng);
		backWheelLoc.y -= velLen * sin(velAng);
		frontWheelLoc.x += velLen * cos(velAng - maxSteerAng);
		frontWheelLoc.y -= velLen * sin(velAng - maxSteerAng);

		//update car location
		player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
		player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
		player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));

		//Update car velocity
		player.vel.x = velLen * cos(velAng + (player.angle - temp));
		player.vel.y = velLen * sin(velAng + (player.angle - temp));
	}
	else if (act.val == 4) {
		//Move back
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		if (player.vel.length() < maxReverseSpeed || (player.vel.getAngle() >= player.angle - 0.2 && player.vel.getAngle() <= player.angle + 0.2)) {
			player.vel.add(-accel, temp);
			velLen = player.vel.length();
		}

		//Turn:
		// Get location of front and back wheels
		float tempLength = WheelBase;
		vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
		vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

		//Get their locations after turning them
		float velAng = player.vel.getAngle();
		backWheelLoc.x += velLen * cos(velAng);
		backWheelLoc.y -= velLen * sin(velAng);
		frontWheelLoc.x += velLen * cos(velAng + maxSteerAng);
		frontWheelLoc.y -= velLen * sin(velAng + maxSteerAng);

		//update car location
		player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
		player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
		player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));

		//Update car velocity Direction
		player.vel.x = velLen * cos(velAng + (player.angle - temp));
		player.vel.y = velLen * sin(velAng + (player.angle - temp));
	}
	else if (act.val == 5) {
		//Move Back
		float temp = player.angle;
		float cosPA = cos(temp);
		float sinPA = sin(temp);
		float velLen = player.vel.length();
		if (player.vel.length() < maxReverseSpeed || (player.vel.getAngle() >= player.angle - 0.2 && player.vel.getAngle() <= player.angle + 0.2)) {
			player.vel.add(-accel, temp);
			velLen = player.vel.length();
		}

		//Turn:
		// Get location of front and back wheels
		float tempLength = WheelBase;
		vec backWheelLoc(player.pos.x - ((tempLength / 2) * cosPA), player.pos.y + ((tempLength / 2) * sinPA));
		vec frontWheelLoc(player.pos.x + ((tempLength / 2) * cosPA), player.pos.y - ((tempLength / 2) * sinPA));

		//Get their locations after turning them
		float velAng = player.vel.getAngle();
		backWheelLoc.x += velLen * cos(velAng);
		backWheelLoc.y -= velLen * sin(velAng);
		frontWheelLoc.x += velLen * cos(velAng - maxSteerAng);
		frontWheelLoc.y -= velLen * sin(velAng - maxSteerAng);

		//update car location
		player.pos.x = (backWheelLoc.x + frontWheelLoc.x) / 2;
		player.pos.y = (backWheelLoc.y + frontWheelLoc.y) / 2;
		player.angle = PI - atan2((backWheelLoc.y - frontWheelLoc.y), (backWheelLoc.x - frontWheelLoc.x));

		//Update car velocity
		player.vel.x = velLen * cos(velAng + (player.angle - temp));
		player.vel.y = velLen * sin(velAng + (player.angle - temp));
	}
	else if (act.val == 6) {
		float velLen = player.vel.length();
		//Decelerate
		if (velLen > 0.05) {
			player.vel.add(-deccel);
			velLen = player.vel.length();
			player.pos.x += player.vel.x;
			player.pos.y -= player.vel.y;
		}
	}
	else {
		cout << "Action not valid, check step function." << endl;
	}

	/*Update Player colliders*/
	float cosAngPSA = cos(player.angle + startingAngle);
	float sinAngPSA = sin(player.angle + startingAngle);
	float cosAngMSA = cos(player.angle - startingAngle);
	float sinAngMSA = sin(player.angle - startingAngle);
	vec topLeft;
	topLeft.x = player.pos.x + distToVert * cosAngPSA;
	topLeft.y = player.pos.y - distToVert * sinAngPSA;

	vec topRight;
	topRight.x = player.pos.x + distToVert * cosAngMSA;
	topRight.y = player.pos.y - distToVert * sinAngMSA;

	vec bottomLeft;
	bottomLeft.x = player.pos.x - distToVert * cosAngMSA;
	bottomLeft.y = player.pos.y + distToVert * sinAngMSA;

	vec bottomRight;
	bottomRight.x = player.pos.x - distToVert * cosAngPSA;
	bottomRight.y = player.pos.y + distToVert * sinAngPSA;

	player.colliders[0] = edge(bottomRight, topLeft);
	player.colliders[1] = edge(bottomLeft, topRight);

	/*Procure state from our info*/
	unsigned int i = 0;
	for (i = 0; i < rayHits.size(); i++) {
		float X1 = (rayHits[i].x - player.pos.x);
		X1 *= X1;
		float Y1 = (rayHits[i].y - player.pos.y);
		Y1 *= Y1;
		float dist = sqrt(X1 + Y1);
		result.state.result.push_back(dist);
	}

	for (i = 0; i < rayHits.size(); i++) {
		result.state.result[i] /= maxPossDist;
	}
	result.won = false;
	result.done = collided;
	/*Reward Agent*/
	if (rewardCollide) {
		//win condition
		if (activeRewardIndex == finishLineIndex) {
			lapsCompleted++;
			if (lapsCompleted >= lapsToWin) {
				result.done = true;
				result.won = true;
				lapsCompleted = 0;
			}
		}
		//reward for hitting gates
		result.reward += rewardGateReward;
		activeRewardIndex = (activeRewardIndex + 1) % rewardGate.size();
	}
	if (collided) {
		//punishment for crashing
		result.reward += hitWallReward;
	}

	return result;
}

void CarGame::reset() {
	float newX = (track[0].v1.x + track[track.size() / 2].v1.x) / 2;
	float newY = (track[0].v1.y + track[track.size() / 2].v1.y) / 2;
	//float dist = sqrt((track[0].v1.y - track[track.size() / 2].v1.y) * (track[0].v1.y - track[track.size() / 2].v1.y) + (track[0].v1.x - track[track.size() / 2].v1.x) * (track[0].v1.x - track[track.size() / 2].v1.x));

	player.pos = vec(newX, newY); 
	player.angle = atan2((track[0].v1.y - track[track.size() / 2].v1.y), -(track[0].v1.x - track[track.size() / 2].v1.x)) + PI / 2;
	player.vel = vec(0, 0);
	activeRewardIndex = 0;

	lapsCompleted = 0;

	/*Update colliders*/
	float cosAngPSA = cos(player.angle + startingAngle);
	float sinAngPSA = sin(player.angle + startingAngle);
	float cosAngMSA = cos(player.angle - startingAngle);
	float sinAngMSA = sin(player.angle - startingAngle);
	vec topLeft;
	topLeft.x = player.pos.x + distToVert * cosAngPSA;
	topLeft.y = player.pos.y - distToVert * sinAngPSA;

	vec topRight;
	topRight.x = player.pos.x + distToVert * cosAngMSA;
	topRight.y = player.pos.y - distToVert * sinAngMSA;

	vec bottomLeft;
	bottomLeft.x = player.pos.x - distToVert * cosAngMSA;
	bottomLeft.y = player.pos.y + distToVert * sinAngMSA;

	vec bottomRight;
	bottomRight.x = player.pos.x - distToVert * cosAngPSA;
	bottomRight.y = player.pos.y + distToVert * sinAngPSA;

	player.colliders[0] = edge(bottomRight, topLeft);
	player.colliders[1] = edge(bottomLeft, topRight);
}

dispPacket CarGame::getCurFrame() {
	return dispPacket(player, track);
}

void CarGame::DrawGameUser() {
	int xDim = wd->getSize().x;
	int yDim = wd->getSize().y;
	/*Player Toggle Keys*/
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;

	//Initialize window
	wd->setKeyRepeatEnabled(false);


	/******************Player Setup********************************/
	/*Get Player texture*/
	sf::Texture playerTexture;
	if (!playerTexture.loadFromFile("CarTextureFlipped.png")) {
		cout << "Car texture could not load" << endl;
	}
	playerTexture.setSmooth(true);

	/*Get current player and track*/
	Agent player = getCurFrame().player;
	vector<edge> track = getCurFrame().track;

	/*Get Player width and length*/
	float playWid = (player.width / 100) * xDim;
	float playLen = (player.length / 100) * yDim;

	/*Instantiate player rectangle*/
	sf::RectangleShape Player(sf::Vector2f(playLen, playWid));
	Player.setOrigin(sf::Vector2f(playLen / 2, playWid / 2));
	Player.setTexture(&playerTexture);
	Player.setScale(1.f, -1.f);
	/*************************************************************/

	/*Assign Track positions*/
	sf::VertexArray Track(sf::Lines, 2 * track.size());
	for (unsigned int i = 0; i < 2 * track.size(); i++) {
		if (i % 2 == 0) {
			float tempX = track[i / 2].v1.x / 100 * xDim;
			float tempY = track[i / 2].v1.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		else {
			float tempX = track[i / 2].v2.x / 100 * xDim;
			float tempY = track[i / 2].v2.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		Track[i].color = trackColor;
	}

	/*Instantiate RayTrace rays*/
	sf::VertexArray Rays(sf::Lines, 2 * rays.size());

	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1 / frameRate) * 1000.0f;

	/*Game Loop*/
	while (wd->isOpen() && !done) {
		/*Limit fps*/
		startTime = chrono::system_clock::now();
		chrono::duration<double, milli> work_time = startTime - lastFrameTime;
		if (work_time.count() < timeInMilli) {
			std::chrono::duration<double, std::milli> delta_ms(timeInMilli - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		lastFrameTime = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = lastFrameTime - startTime;

		/*Handle User input.*/
		sf::Event evnt;
		while (wd->pollEvent(evnt)) {
			switch (evnt.type) {
			case sf::Event::KeyPressed:
				if (evnt.key.code == sf::Keyboard::W) {
					w = true;
				}
				else if (evnt.key.code == sf::Keyboard::S) {
					s = true;
				}
				else if (evnt.key.code == sf::Keyboard::A) {
					a = true;
				}
				else if (evnt.key.code == sf::Keyboard::D) {
					d = true;
				}
				break;
			case sf::Event::KeyReleased:
				if (evnt.key.code == sf::Keyboard::W) {
					w = false;
				}
				else if (evnt.key.code == sf::Keyboard::S) {
					s = false;
				}
				else if (evnt.key.code == sf::Keyboard::A) {
					a = false;
				}
				else if (evnt.key.code == sf::Keyboard::D) {
					d = false;
				}
				break;
			case sf::Event::Closed:
				wd->close();
				done = true;
				break;
			default:
				break;
			}
		}
		 
		Action act;
		/*Converts key toggles into actionable input*/
		if (w) {
			if (a) {
				act.val = 4;
			}
			else if (d) {
				act.val = 5;
			}
			else {
				act.val = 0;
			}
		}
		else if (s) {
			if (a) {
				act.val = 6;
			}
			else if (d) {
				act.val = 7;
			}
			else {
				act.val = 2;
			}
		}
		else if (a) {
			act.val = 1;
		}
		else if (d) {
			act.val = 3;
		}
		else {
			act.val = 8;
		}
		
		Packet newState = step(act);

		/*Update Player*/
		player = getCurFrame().player;

		/*Extract player position*/
		Agent player = getCurFrame().player;
		float playPosX = (player.pos.x / 100) * xDim;
		float playPosY = (player.pos.y / 100) * yDim;

		sf::VertexArray Colliders(sf::Lines, 4);
		Colliders[0].position = sf::Vector2f(player.colliders[0].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[1].position = sf::Vector2f(player.colliders[0].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);
		Colliders[2].position = sf::Vector2f(player.colliders[1].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[3].position = sf::Vector2f(player.colliders[1].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);

		/*Turn Track Red if collided*/
		if (collided) {
			if (Track[0].color == trackColor) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = sf::Color::Red;
				}
			}
		}
		else {
			if (Track[0].color == sf::Color::Red) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = trackColor;
				}
			}

		}

		/*Update Player Rays Representations*/
		for (unsigned int i = 0; i < 2 * rays.size(); i++) {
			if (i % 2 == 0) {
				Rays[i].position = sf::Vector2f(playPosX, playPosY);
			}
			else {
				float RayPosX = ((rayHits[i / 2].x) / 100) * xDim;
				float RayPosY = ((rayHits[i / 2].y) / 100) * yDim;
				Rays[i].position = sf::Vector2f(RayPosX, RayPosY);
			}
		}

		//Update active reward Gate
		activeRewardGate[0].position.x = rewardGate[(activeRewardIndex) % rewardGate.size()].v1.x / 100 * xDim;
		activeRewardGate[0].position.y = rewardGate[(activeRewardIndex) % rewardGate.size()].v1.y / 100 * yDim;
		activeRewardGate[1].position.x = rewardGate[(activeRewardIndex) % rewardGate.size()].v2.x / 100 * xDim;
		activeRewardGate[1].position.y = rewardGate[(activeRewardIndex) % rewardGate.size()].v2.y / 100 * yDim;

		/*Create player object representation*/
		Player.setPosition(sf::Vector2f(playPosX, playPosY));
		Player.setRotation(-(player.angle / PI) * 180);

		/*Draw To Screen*/
		wd->clear();
		wd->draw(Track);
		wd->draw(Player);
		wd->draw(Rays);
		wd->draw(activeRewardGate);
		//wd->draw(Colliders);
		wd->display();
	}
	wd->close();
}

void CarGame::DrawGameAI(NNet &myNet) {
	int xDim = wd->getSize().x;
	int yDim = wd->getSize().y;
	reset();

	/******************Player Setup********************************/
	/*Get Player texture*/
	sf::Texture playerTexture;
	if (!playerTexture.loadFromFile("CarTextureFlipped.png")) {
		cout << "Car texture could not load" << endl;
	}
	playerTexture.setSmooth(true);

	/*Get current player and track*/
	Agent player = getCurFrame().player;
	vector<edge> track = getCurFrame().track;

	/*Get Player width and length*/
	float playWid = (player.width / 100) * xDim;
	float playLen = (player.length / 100) * yDim;

	/*Instantiate player rectangle*/
	sf::RectangleShape Player(sf::Vector2f(playLen, playWid));
	Player.setOrigin(sf::Vector2f(playLen / 2, playWid / 2));
	Player.setTexture(&playerTexture);
	Player.setScale(1.f, -1.f);
	/*************************************************************/

	/*Assign Track positions*/
	sf::VertexArray Track(sf::Lines, 2 * track.size());
	for (unsigned int i = 0; i < 2 * track.size(); i++) {
		if (i % 2 == 0) {
			float tempX = track[i / 2].v1.x / 100 * xDim;
			float tempY = track[i / 2].v1.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		else {
			float tempX = track[i / 2].v2.x / 100 * xDim;
			float tempY = track[i / 2].v2.y / 100 * yDim;
			Track[i].position = sf::Vector2f(tempX, tempY);
		}
		Track[i].color = trackColor;
	}
	/*Assign reward gate positions*/
	sf::VertexArray RGates(sf::Lines, 2 * rewardGate.size());
	for (unsigned int i = 0; i < 2 * rewardGate.size(); i++) {
		if (i % 2 == 0) {
			float tempX = rewardGate[i / 2].v1.x / 100 * xDim;
			float tempY = rewardGate[i / 2].v1.y / 100 * yDim;
			RGates[i].position = sf::Vector2f(tempX, tempY);
		}
		else {
			float tempX = rewardGate[i / 2].v2.x / 100 * xDim;
			float tempY = rewardGate[i / 2].v2.y / 100 * yDim;
			RGates[i].position = sf::Vector2f(tempX, tempY);
		}
		RGates[i].color = trackColor;
	}

	/*Instantiate RayTrace rays*/
	sf::VertexArray Rays(sf::Lines, 2 * rays.size());

	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1 / frameRate) * 1000.0f;

	/*Game Loop*/
	State curState = getStartState();
	while (wd->isOpen() && !done) {
		/*Limit fps*/
		startTime = chrono::system_clock::now();
		chrono::duration<double, milli> work_time = startTime - lastFrameTime;
		if (work_time.count() < timeInMilli) {
			std::chrono::duration<double, std::milli> delta_ms(timeInMilli - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		lastFrameTime = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = lastFrameTime - startTime;

		/*Handle User input.*/
		sf::Event evnt;
		while (wd->pollEvent(evnt)) {
			switch (evnt.type) {
			case sf::Event::Closed:
				done = true;
				wd->close();
				break;
			default:
				break;
			}
		}
		
		/*Update Player*/
		player = getCurFrame().player;

		/*Extract player position*/
		Agent player = getCurFrame().player;
		float playPosX = (player.pos.x / 100) * xDim;
		float playPosY = (player.pos.y / 100) * yDim;

		sf::VertexArray Colliders(sf::Lines, 4);
		Colliders[0].position = sf::Vector2f(player.colliders[0].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[1].position = sf::Vector2f(player.colliders[0].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);
		Colliders[2].position = sf::Vector2f(player.colliders[1].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[3].position = sf::Vector2f(player.colliders[1].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);

		/*Turn Track Red if collided*/
		if (collided) {
			curState.print();
			if (Track[0].color == trackColor) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = sf::Color::Red;
				}
			}
		}
		else {
			if (Track[0].color == sf::Color::Red) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = trackColor;
				}
			}

		}

		/*Update Player Rays Representations*/
		for (unsigned int i = 0; i < 2 * rays.size(); i++) {
			if (i % 2 == 0) {
				Rays[i].position = sf::Vector2f(playPosX, playPosY);
			}
			else {
				float RayPosX = ((rayHits[i / 2].x) / 100) * xDim;
				float RayPosY = ((rayHits[i / 2].y) / 100) * yDim;
				Rays[i].position = sf::Vector2f(RayPosX, RayPosY);
			}
		}

		/*Create player object representation*/
		Player.setPosition(sf::Vector2f(playPosX, playPosY));
		Player.setRotation(-(player.angle / PI) * 180);

		/*Draw To Screen*/
		wd->clear();
		wd->draw(Track);
		//wd->draw(RGates);
		wd->draw(Player);
		//wd->draw(Rays);
		//wd->draw(Colliders);
		wd->display();

		Action act;
		myNet.feedForward(curState.result);
		act = Action(myNet.getMaxOutputIndex());
		switch (act.val) {
		case 0:
			cout << "It moved up" << endl;
			break;
		case 1:
			cout << "It moved left" << endl;
			break;
		case 2:
			cout << "It moved right" << endl;
			break;
		case 3:
			cout << "It moved right" << endl;
			break;
		case 4:
			cout << "It moved up-left" << endl;
			break;
		case 5:
			cout << "It moved up-right" << endl;
			break;
		case 6:
			cout << "It moved down-left" << endl;
			break;
		case 7:
			cout << "It moved down-right" << endl;
			break;
		case 8:
			cout << "It moved nowhere" << endl;
			break;
		default:
			cout << "It didnt pick a valid action" << endl;
			break;
		}

		Packet temp = step(act);
		curState = temp.state;


		if (temp.done) {
			break;
		}
	}
	done = true;
}

void CarGame::Display() {
	/*Game Loop*/
	if (wd->isOpen() && !done) {
		/*Handle User input.*/
		sf::Event evnt;
		while (wd->pollEvent(evnt)) {
			switch (evnt.type) {
			case sf::Event::Closed:
				done = true;
				wd->close();
				break;
			default:
				break;
			}
		}
		float playPosX = (player.pos.x / 100) * xDim;
		float playPosY = (player.pos.y / 100) * yDim;

		//Draw Player Colliders
		/*sf::VertexArray Colliders(sf::Lines, 4);
		Colliders[0].position = sf::Vector2f(player.colliders[0].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[1].position = sf::Vector2f(player.colliders[0].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);
		Colliders[2].position = sf::Vector2f(player.colliders[1].v1.x / 100 * xDim, player.colliders[0].v1.y / 100 * yDim);
		Colliders[3].position = sf::Vector2f(player.colliders[1].v2.x / 100 * xDim, player.colliders[0].v2.y / 100 * yDim);*/

		//Turn Track Red if collided
		/*if (collided) {
			if (Track[0].color == trackColor) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = sf::Color::Red;
				}
			}
		}
		else {
			if (Track[0].color == sf::Color::Red) {
				for (unsigned int i = 0; i < 2 * track.size(); i++) {
					Track[i].color = trackColor;
				}
			}
		}*/

		//Update Player Rays Representations
		for (unsigned int i = 0; i < 2 * rays.size(); i++) {
			if (i % 2 == 0) {
				Rays[i].position = sf::Vector2f(playPosX, playPosY);
			}
			else {
				float RayPosX = ((rayHits[i / 2].x) / 100) * xDim;
				float RayPosY = ((rayHits[i / 2].y) / 100) * yDim;
				Rays[i].position = sf::Vector2f(RayPosX, RayPosY);
			}
		}

		//Update active reward Gate
		activeRewardGate[0].position.x = rewardGate[(activeRewardIndex) % rewardGate.size()].v1.x/100 * xDim;
		activeRewardGate[0].position.y = rewardGate[(activeRewardIndex) % rewardGate.size()].v1.y/100 * yDim;
		activeRewardGate[1].position.x = rewardGate[(activeRewardIndex) % rewardGate.size()].v2.x/100 * xDim;
		activeRewardGate[1].position.y = rewardGate[(activeRewardIndex) % rewardGate.size()].v2.y/100 * yDim;

		//Update rayhit representation locations
		for (unsigned int i = 0; i < rayHits.size(); i++) {
			rayHitIndic[i].setPosition(sf::Vector2f(rayHits[i].x/100 *xDim, rayHits[i].y/100 * yDim));
		}

		/*Create player object representation*/
		Player.setPosition(sf::Vector2f(playPosX, playPosY));
		Player.setRotation(-(player.angle / PI) * 180);

		/*Draw To Screen*/
		wd->clear();
		wd->draw(Track);
		//wd->draw(RGates);
		for (unsigned int i = 0; i < rayHits.size(); i++) {
			wd->draw(rayHitIndic[i]);
		}
		wd->draw(Player);
		//wd->draw(activeRewardGate);
		//wd->draw(Rays);
		//wd->draw(Colliders);
		wd->display();
	}
}

void CarGame::hasCollided() {
	while (!done) {
		bool temp = false;
		bool temp2 = false;
		for (unsigned int i = 0; i < player.colliders.size(); i++) {
			for (unsigned int j = 0; j < track.size(); j++) {
				if (edgeIntersect(player.colliders[i], track[j])) {
					temp = true;
				}
			}
			if (edgeIntersect(player.colliders[i], rewardGate[activeRewardIndex])) {
				temp2 = true;
			}
		}
		collided = temp;
		rewardCollide = temp2;
	}
}

void CarGame::rayTrace() {
	//If ray doesnt hit anything, rayHit = stepsize * maxReps
	float maxReps = 1000;
	while (!done) {
		int numRaysCollided = 0;
		int k = 0;

		vector<vec> tempRays = rays;
		vector<vec> cosSinVals;
		vector<float> hasHit;
		// need to make sure rayhit correspond to how rays were defined
		for (int i = 0; i < rays.size(); i++) {
			float newX = cos(player.angle + tempRays[i].getAngle());
			float newY = sin(player.angle + tempRays[i].getAngle());
			cosSinVals.push_back(vec(newX, newY));
			hasHit.push_back(false);
		}

		//While not all rays have hit something and current rep is less than max rep
		while (k < maxReps && numRaysCollided != rays.size()) {//Increase radius of search by stepsize
			k++;
			for (int i = 0; i < tempRays.size(); i++) {
				if (!hasHit[i])
				{//increment them by step size
					tempRays[i].add(stepSize);
					edge temp1(player.pos, player.pos + vec(tempRays[i].length() * cosSinVals[i].x, -tempRays[i].length() * cosSinVals[i].y));
					//check for collisions for every edge
					for (int j = 0; j < track.size(); j++) {
						auto temp = edgeIntersect(temp1, track[j]);
						if (temp) {
							numRaysCollided++;
							rayHits[i] = temp1.v2;
							if (isnan(rayHits[i].x) || isnan(rayHits[i].y)) {
								cout << "Rayhit was nan, checkraytrace func" << endl;
							}
							hasHit[i] = true;
							break;
						}
					}
				}
			}
		}
	}
}

void CarGame::BuildTrack() {
	/*Function variables*/
	int xDim = wd->getSize().x;
	int yDim = wd->getSize().y;
	bool finished = false;

	/*Load font*/
	sf::Font font;
	if (!font.loadFromFile("Roboto-Black.ttf")) {
		cout << "Could not load font" << endl;
	}

	/*Instantiate text*/
	sf::Text instruct1;
	instruct1.setFont(font);
	instruct1.setString("Q - Rotate anticlockwise");
	instruct1.setCharacterSize(10);
	sf::Text instruct2 = instruct1;
	instruct2.setString("E - Rotate clockwise");
	instruct2.setPosition(sf::Vector2f(0, 12));
	sf::Text instruct3 = instruct1;
	instruct3.setString("MouseWheel - Scale track size");
	instruct3.setPosition(sf::Vector2f(0, 24));
	sf::Text instruct5 = instruct1;
	instruct5.setString("X - Delete last Track point");
	instruct5.setPosition(sf::Vector2f(0, 36));
	sf::Text instruct6 = instruct1;
	instruct6.setString("Right click = select");
	instruct6.setPosition(sf::Vector2f(0, 48));
	sf::Text instruct4 = instruct1;
	instruct4.setString("Click to place, Enter to go to game");
	instruct4.setPosition(sf::Vector2f(0, 60));

	/*Instantiate track*/
	vector<trackPoint> tp;
	trackPoint *selected;

	/*Instantiate trackpoint*/
	int selectedIndex = 0;
	tp.push_back(trackPoint(vec(50, 50), PI / 2, 5));
	selected = &tp[0];
	float X = selected->pos.x / 100 * xDim;
	float Y = selected->pos.y / 100 * yDim;
	float rad = selected->radius / 100 * (cos(selected->angle) * xDim + sin(selected->angle) * yDim);
	//Depending on what angle I am at, my radius could be its full length in the x axis, the full length in y, or somewhere in btw

	/*Convert trackpoint to on screen representation*/
	vector<sf::CircleShape> tpRep;
	tpRep.push_back(sf::CircleShape(trackPointSize));
	tpRep[0].setOrigin(trackPointSize, trackPointSize);
	tpRep[0].setFillColor(sf::Color::Magenta);

	/*Instantiate on screen representation of track and trackpoints*/
	sf::VertexArray trackEdge(sf::Lines, 2 * tp.size());
	sf::VertexArray roadSquares(sf::Quads, 4 * tp.size());


	/*Instantiate fps Limiters*/
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1 / frameRate) * 1000.0f;

	/*Track building Loop*/
	while (wd->isOpen() && !finished) {
		/*Limit fps*/
		startTime = chrono::system_clock::now();
		chrono::duration<double, milli> work_time = startTime - lastFrameTime;
		if (work_time.count() < timeInMilli) {
			std::chrono::duration<double, std::milli> delta_ms(timeInMilli - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		lastFrameTime = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = lastFrameTime - startTime;

		/*Handle User input*/
		sf::Event evnt;
		while (wd->pollEvent(evnt)) {
			switch (evnt.type) {
			case sf::Event::Closed:
				wd->close();
				break;
			case sf::Event::KeyPressed:
				if (evnt.key.code == sf::Keyboard::Return) {
					tpRep[selectedIndex].setOutlineThickness(0);
					if (selectedIndex == tp.size() - 1) {
						//Delete last tp
						tp.pop_back();
						//delete last tp rep
						tpRep.pop_back();
						//Update trackEdge size
						trackEdge.resize(2 * tp.size());
						selected = &tp[0];
						selectedIndex = 0;
					}
					finished = true;
				}
				else if (evnt.key.code == sf::Keyboard::Q) {
					selected->angle += d_angle;
				}
				else if (evnt.key.code == sf::Keyboard::E) {
					selected->angle -= d_angle;
				}
				else if (evnt.key.code == sf::Keyboard::X) {
					if (tp.size() != 0) {
						//Delete last tp
						tp.pop_back();
						selected = &tp.back();
						selectedIndex = tp.size() - 1;

						//create a new tp rep
						tpRep.pop_back();

						//Update trackEdge size
						trackEdge.resize(2 * tp.size());

						//Something like this
						//It places the beg of the road squares where the mouse is
						int chosen = selectedIndex;
						if (selectedIndex < 1) {
							chosen = 1;
						}
						X = tp[chosen - 1].pos.x / 100 * xDim;
						Y = tp[chosen - 1].pos.y / 100 * yDim;
						rad = (tp[chosen - 1].radius / 100) * (abs(cos(tp[chosen - 1].angle)) * xDim + abs(sin(tp[chosen - 1].angle)) * yDim);
						vec v1(X + rad * cos(tp[chosen - 1].angle), Y - rad * sin(tp[chosen - 1].angle));
						vec v2(X - rad * cos(tp[chosen - 1].angle), Y + rad * sin(tp[chosen - 1].angle));
						roadSquares[4 * (chosen)].position = sf::Vector2f(v1.x, v1.y);
						roadSquares[4 * (chosen) + 1].position = sf::Vector2f(v2.x, v2.y);
						roadSquares.resize(4 * tp.size());
					}
				}
				break;
			case sf::Event::MouseWheelMoved:
				selected->radius -= evnt.mouseWheel.delta;
				break;
			case sf::Event::MouseButtonPressed:
				if (evnt.mouseButton.button == sf::Mouse::Left) {
					if (selectedIndex == tp.size() - 1) {
						//Create a new tp
						tp.push_back(trackPoint(selected->pos, selected->angle, selected->radius));
						//set current selected border to 0
						tpRep[selectedIndex].setOutlineThickness(0);

						//Update trackEdge size
						X = tp[selectedIndex].pos.x / 100 * xDim;
						Y = tp[selectedIndex].pos.y / 100 * yDim;
						rad = (tp[selectedIndex].radius / 100) * (abs(cos(tp[selectedIndex].angle)) * xDim + abs(sin(tp[selectedIndex].angle)) * yDim);
						vec v1(X + rad * cos(tp[selectedIndex].angle), Y - rad * sin(tp[selectedIndex].angle));
						vec v2(X - rad * cos(tp[selectedIndex].angle), Y + rad * sin(tp[selectedIndex].angle));
						roadSquares.resize(4 * tp.size());
						
						//Update selected tp
						selectedIndex = tp.size() - 1;
						selected = &tp[selectedIndex];
						roadSquares[4 * selectedIndex].position = sf::Vector2f(v1.x, v1.y);
						roadSquares[4 * selectedIndex + 1].position = sf::Vector2f(v2.x, v2.y);
						roadSquares[4 * selectedIndex].color = sf::Color(100, 100, 100, 225);
						roadSquares[4 * selectedIndex + 1].color = sf::Color(100, 100, 100, 225);

						//create a new tp rep
						tpRep.push_back(sf::CircleShape(trackPointSize));
						tpRep[selectedIndex].setOrigin(trackPointSize, trackPointSize);
						tpRep[selectedIndex].setFillColor(sf::Color::Magenta);


						trackEdge.resize(2 * tp.size());

					}
				}
				else if (evnt.mouseButton.button == sf::Mouse::Right) {
					int mouseX = sf::Mouse::getPosition(*wd).x;
					int mouseY = sf::Mouse::getPosition(*wd).y;
					for (int i = 0; i < tp.size(); i++) {
						float onScreenX = tp[i].pos.x / 100 * xDim;
						float onScreenY = tp[i].pos.y / 100 * yDim;
						if (sqrt((onScreenX - mouseX) * (onScreenX - mouseX) + (onScreenY - mouseY) * (onScreenY - mouseY)) <= trackPointSize) {
							if (i == tp.size() - 1) {
								//Create a new tp
								tp.push_back(trackPoint(tp[i].pos, tp[i].angle, tp[i].radius));
								tpRep[selectedIndex].setOutlineThickness(0);

								//create a new tp rep
								tpRep.push_back(sf::CircleShape(trackPointSize));
								tpRep[tpRep.size() - 1].setOrigin(trackPointSize, trackPointSize);
								tpRep[tpRep.size() - 1].setFillColor(sf::Color::Magenta);

								//Update trackEdge size
								trackEdge.resize(2 * tp.size());
								roadSquares.resize(4 * tp.size());
								//Dont choose last selected, use last in the list
								X = tp[i].pos.x / 100 * xDim;
								Y = tp[i].pos.y / 100 * yDim;
								rad = (tp[i].radius / 100) * (abs(cos(tp[i].angle)) * xDim + abs(sin(tp[i].angle)) * yDim);
								vec v1(X + rad * cos(tp[i].angle), Y - rad * sin(tp[i].angle));
								vec v2(X - rad * cos(tp[i].angle), Y + rad * sin(tp[i].angle));
								selected = &tp.back();
								selectedIndex = tp.size() - 1;
								roadSquares[4 * selectedIndex].position = sf::Vector2f(v1.x, v1.y);
								roadSquares[4 * selectedIndex + 1].position = sf::Vector2f(v2.x, v2.y);
								roadSquares[4 * selectedIndex].color = sf::Color(100, 100, 100, 225);
								roadSquares[4 * selectedIndex + 1].color = sf::Color(100, 100, 100, 225);
								break;
							}
							else {
								//If the last tp is still attached to the mouse
								//if previous selected was the last in the pack
								tpRep[selectedIndex].setOutlineThickness(0);
								if (selectedIndex == tp.size() - 1) {
									//Delete last tp
									tp.pop_back();
									//delete last tp rep
									tpRep.pop_back();
									//Update trackEdge size.
									trackEdge.resize(2 * tp.size());
									roadSquares.resize(4 * tp.size());

									int chosen = selectedIndex;
									if (selectedIndex < 1) {
										chosen = 1;
									}
									X = tp[chosen - 1].pos.x / 100 * xDim;
									Y = tp[chosen - 1].pos.y / 100 * yDim;
									rad = (tp[chosen - 1].radius / 100) * (abs(cos(tp[chosen - 1].angle)) * xDim + abs(sin(tp[chosen - 1].angle)) * yDim);
									vec v1(X + rad * cos(tp[chosen - 1].angle), Y - rad * sin(tp[chosen - 1].angle));
									vec v2(X - rad * cos(tp[chosen - 1].angle), Y + rad * sin(tp[chosen - 1].angle));
									roadSquares[4 * (chosen - 1)].position = sf::Vector2f(v1.x, v1.y);
									roadSquares[4 * (chosen - 1) + 1].position = sf::Vector2f(v2.x, v2.y);
								}

								selected = &tp[i];
								selectedIndex = i;
								break;
							}
						}

					}
				}
				break;
			default:
				break;
			}
		}

		/*Update selected icon*/
		tpRep[selectedIndex].setOutlineThickness(selectedThicc);
		tpRep[selectedIndex].setOutlineColor(sf::Color::Black);
		
		/*Update selected trackpoint position*/
		if (*selected == tp.back()) {
			tpRep[selectedIndex].setPosition(sf::Mouse::getPosition(*wd).x, sf::Mouse::getPosition(*wd).y);

			selected->pos.x = 1.f * sf::Mouse::getPosition(*wd).x / xDim * 100;
			selected->pos.y = 1.f * sf::Mouse::getPosition(*wd).y / yDim * 100;
		}

		//Length or radius changes depending on whether it is on the x or y axis
		X = selected->pos.x / 100 * xDim;
		Y = selected->pos.y / 100 * yDim;
		rad = (selected->radius / 100) * (abs(cos(selected->angle)) * xDim + abs(sin(selected->angle)) * yDim);

		/*Update and convert selected tp lines to onscreen representations*/
		vec v1(X + rad * cos(selected->angle), Y - rad * sin(selected->angle));
		vec v2(X - rad * cos(selected->angle), Y + rad * sin(selected->angle));

		trackEdge[2 * selectedIndex].position = sf::Vector2f(v1.x, v1.y);
		trackEdge[2 * selectedIndex + 1].position = sf::Vector2f(v2.x, v2.y);

		/*Update road squares*/
		if (tp.size() == 1) {
			roadSquares[4 * selectedIndex].position = sf::Vector2f(v1.x, v1.y);
			roadSquares[4 * selectedIndex + 1].position = sf::Vector2f(v2.x, v2.y);
			roadSquares[4 * selectedIndex + 2].position = sf::Vector2f(v2.x, v2.y);
			roadSquares[4 * selectedIndex + 3].position = sf::Vector2f(v1.x, v1.y);
			roadSquares[4 * selectedIndex].color = sf::Color(100, 100, 100, 225);
			roadSquares[4 * selectedIndex + 1].color = sf::Color(100, 100, 100, 225);
			roadSquares[4 * selectedIndex + 2].color = sf::Color(100, 100, 100, 225);
			roadSquares[4 * selectedIndex + 3].color = sf::Color(100, 100, 100, 225);
		}
		else {
			roadSquares[4 * selectedIndex + 2].position = sf::Vector2f(v2.x, v2.y);
			roadSquares[4 * selectedIndex + 3].position = sf::Vector2f(v1.x, v1.y);
			int chosen = selectedIndex;
			if (selectedIndex < 1) {
				chosen = 1;
			}
			roadSquares[4 * (chosen - 1)].position = sf::Vector2f(v1.x, v1.y);
			roadSquares[4 * (chosen - 1) + 1].position = sf::Vector2f(v2.x, v2.y);

			roadSquares[4 * selectedIndex + 2].color = sf::Color(100, 100, 100, 225);
			roadSquares[4 * selectedIndex + 3].color = sf::Color(100, 100, 100, 225);
		}


		/*Draw to screen*/
		wd->clear();
		wd->draw(roadSquares);
		for (unsigned int i = 0; i < tpRep.size(); i++) {
			wd->draw(tpRep[i]);
		}
		wd->draw(trackEdge);
		wd->draw(instruct1);
		wd->draw(instruct2);
		wd->draw(instruct3);
		wd->draw(instruct4);
		wd->draw(instruct5);
		wd->draw(instruct6);
		wd->display();
	}

	/*Convert track points to an actionable version*/
	//Add all the top verts from the tps to the track
	for (int i = 0; i < tp.size(); i++) {
		float thisX = tp[i].pos.x + tp[i].radius * cos(tp[i].angle);
		float thisY = tp[i].pos.y - tp[i].radius * sin(tp[i].angle);
		float nextX = tp[(i + 1) % tp.size()].pos.x + tp[(i + 1) % tp.size()].radius * cos(tp[(i + 1) % tp.size()].angle);
		float nextY = tp[(i + 1) % tp.size()].pos.y - tp[(i + 1) % tp.size()].radius * sin(tp[(i + 1) % tp.size()].angle);

		track.push_back(edge(vec(thisX, thisY), vec(nextX, nextY)));
	}

	//Add all the bottom verts from the tp to the track
	for (int i = 0; i < tp.size(); i++) {
		float thisX = tp[i].pos.x - tp[i].radius * cos(tp[i].angle);
		float thisY = tp[i].pos.y + tp[i].radius * sin(tp[i].angle);
		float nextX = tp[(i + 1) % tp.size()].pos.x - tp[(i + 1) % tp.size()].radius * cos(tp[(i + 1) % tp.size()].angle);
		float nextY = tp[(i + 1) % tp.size()].pos.y + tp[(i + 1) % tp.size()].radius * sin(tp[(i + 1) % tp.size()].angle);

		track.push_back(edge(vec(thisX, thisY), vec(nextX, nextY)));
	}
}

void  CarGame::save() {
	ofstream wr(saveFile);
	//Write the number of edges
	wr << track.size() << endl;
	//write down all edges
	for (unsigned int i = 0; i < track.size(); i++) {
		wr << track[i].v1.x << " " << track[i].v1.y << " " << track[i].v2.x << " " << track[i].v2.y << endl;
	}
}

void CarGame::load() {
	ifstream rd(saveFile);
	//read the number of edges
	int size;
	rd >> size;
	for (int i = 0; i < size; i++) {
		float a, b;
		rd >> a >> b;
		vec tmp1(a, b);
		rd >> a >> b;
		vec tmp2(a, b);
		track.push_back(edge(tmp1, tmp2));
	}
}

bool CarGame::hasSavedFile() {
	fstream fl;
	fl.open(saveFile);
	if (fl.fail()) return false;
	return true;
}

CarGame::~CarGame()
{
	Coll.join();
	RT.join();
}

void CarGame::endDisplay() {
	done = true;
	wd->close();
}
