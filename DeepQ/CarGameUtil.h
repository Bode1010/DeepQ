#ifndef CarGameUtil_h
#define CarGameUtil_h
#include <math.h>
#include <vector>
#include <iostream>
using namespace std;

static const float PI = 3.141592654;

float convTo360Scale(float);

struct vec {
	vec() {};
	vec(float x1, float y1) { x = x1; y = y1; }
	//Adds given length to the hypotenuse and updates the x and y values
	void add(float len);
	//Adds given length in a specified direction and updates the x and y values
	void add(float len, const float ang);
	float getAngle() { return atan2(y , x); }
	float length() { return sqrt(x * x + y * y); }
	vec operator+(const vec & obj) { vec result; result.x = x + obj.x; result.y = y + obj.y; return result; }
	vec operator*(float num) { vec result(x * num, y * num); return result; }
	bool operator==(const vec &obj) { if (x == obj.x && y == obj.y) return true; return false; }

	/*Variables*/
	float x;
	float y;
};

struct edge {
	vec v1;
	vec v2;
	edge() {};
	edge(vec p1, vec p2) { v1 = p1; v2 = p2; }
};

struct intersect {
	//whether or not it intersected
	bool first;
	//where it intersected the first edge on a scale of 0 - 1
	float second;
	//where it intersected the first edge on a scale of 0 - 1
	float third;
	intersect() {};
	intersect(bool v1, float v2, float v3) { first = v1; second = v2; third = v3; }
};

struct Agent {
	Agent() {};
	Agent(vec pos1, float l1, float w1) { pos = pos1; length = l1; width = w1; }
	vec pos;
	vec vel;
	float width;
	float angle;
	float length;
	vector<edge> colliders;
};

struct trackPoint {
	vec pos;
	float angle;
	float radius;
	trackPoint() {};
	trackPoint(vec p, float a, float r) { pos = p; angle = a; radius = r; }
	bool operator==(const trackPoint &obj) { if (pos == obj.pos && angle == obj.angle && radius == obj.radius) return true; return false; }
};

//Display packet. Used to send info from the game to the renderer
struct dispPacket {
	Agent player;
	vector<edge> track;
	dispPacket() {};
	dispPacket(Agent a, vector<edge> t) { player = a; track = t; }
};

bool onSegment(vec p1, vec p2, vec p3);

int orientation(vec p1, vec p2, vec p3);

bool edgeIntersect(edge e1, edge e2);

intersect edgeIntersectReturnVal(edge e1, edge e2);

#endif