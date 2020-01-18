#include "CarGameUtil.h"

float convTo360Scale(float trig) {
	if (trig < 0 ){
		return PI + (PI + trig);
	}
	else return trig;
}

//Check whether p2 is btw p1 and p3
bool onSegment(vec p1, vec p2, vec p3) {
	float maxX = p1.x;
	if (p3.x > p1.x) maxX = p3.x;
	float minX = p1.x;
	if (p3.x < p1.x) minX = p3.x;
	float maxY = p1.y;
	if (p3.y > p1.y) maxY = p3.y;
	float minY = p1.y;
	if (p3.y < p1.y) minY = p3.y;


	if (p2.x <= maxX && p2.x >= minX && p2.y <= maxY && p2.y <= minY) return true;
	return false;
}

//Return whether 3 locations in that order are going clockwise or anticlockwise
int orientation(vec p1, vec p2, vec p3) {
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);

	//Parallel
	if (val == 0) {
		return 0;
	}
	//Clockwise
	else if (val > 0) {
		return 1;
	}
	//AntiClockwise
	else {
		return 2;
	}
}

//Returns if 2 edges intersected
bool edgeIntersect(edge e1, edge e2) {
	int o1 = orientation(e1.v1, e1.v2, e2.v1);
	int o2 = orientation(e1.v1, e1.v2, e2.v2);
	int o3 = orientation(e2.v1, e2.v2, e1.v1);
	int o4 = orientation(e2.v1, e2.v2, e1.v2);

	//General Case
	if (o1 != o2 && o3 != o4) {
		return true;
	}

	//If they are all on the same line
	if (o1 == 0 && onSegment(e1.v1, e2.v2, e1.v2)) return true;
	if (o2 == 0 && onSegment(e1.v1, e2.v2, e1.v2)) return true;
	if (o3 == 0 && onSegment(e2.v1, e1.v1, e2.v2)) return true;
	if (o4 == 0 && onSegment(e2.v1, e1.v2, e2.v2)) return true;

	return false;
}

//Calculates where on the line they intersected
intersect edgeIntersectReturnVal(edge e1, edge e2) {
	intersect result;
	result.first = true;
	result.second = -1;
	result.third = -1;

	int o1 = orientation(e1.v1, e1.v2, e2.v1);
	int o2 = orientation(e1.v1, e1.v2, e2.v2);
	int o3 = orientation(e2.v1, e2.v2, e1.v1);
	int o4 = orientation(e2.v1, e2.v2, e1.v2);

	//General Case
	if (o1 != o2 && o3 != o4) {
		//calculate and update first and second intersect val
		float m1, m2, c1, c2, x, y, hyp1, hyp2, val1, val2;
		m1 = (e1.v2.y - e1.v1.y) / (e1.v2.x - e1.v1.x);
		m2 = (e2.v2.y - e2.v1.y) / (e2.v2.x - e2.v1.x);
		c1 = e1.v1.y - m1 * e1.v1.x;
		c2 = e2.v1.y - m2 * e2.v1.x;

		x = (c2 - c1) / (m1 - m2);
		y = m1 * x + c1;
		hyp1 = sqrt((e1.v2.x - e1.v1.x) * (e1.v2.x - e1.v1.x) + (e1.v2.y - e1.v1.y) * (e1.v2.y - e1.v1.y));
		hyp2 = sqrt((e2.v2.x - e2.v1.x) * (e2.v2.x - e2.v1.x) + (e2.v2.y - e2.v1.y) * (e2.v2.y - e2.v1.y));
		val1 = sqrt((x - e1.v1.x) * (x - e1.v1.x) + (x - e1.v1.y) * (x - e1.v1.y));
		val2 = sqrt((x - e2.v1.x) * (x - e2.v1.x) + (x - e2.v1.y) * (x - e2.v1.y));

		result.second = val1 / hyp1;
		result.third = val2 / hyp2;
		return result;
	}

	//If they are all on the same line
	if (o1 == 0 && onSegment(e1.v1, e2.v2, e1.v2)) return result;
	if (o2 == 0 && onSegment(e1.v1, e2.v2, e1.v2)) return result;
	if (o3 == 0 && onSegment(e2.v1, e1.v1, e2.v2)) return result;
	if (o4 == 0 && onSegment(e2.v1, e1.v2, e2.v2)) return result;

	result.first = false;
	return result;
}

void vec::add(float len) {
	if (length() != 0) {
		float myLen = len / length();
		x += x * myLen;
		y += y * myLen;
	}
	else {
		cout << "Tried to add to the hypotenuse of a 0 2d vector" << endl;
	}
}

void vec::add(float len, const float ang) {
	x += len * cos(ang);
	y += len * sin(ang);
}
