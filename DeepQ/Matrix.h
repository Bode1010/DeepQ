#ifndef MATRIX_H
#define MATRIX_H

#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
using namespace std;

class Matrix
{
public:
	Matrix(int, int, bool);//rows, columns, random(true or false)
	Matrix operator+(const Matrix &);
	Matrix operator*(const Matrix &);
	Matrix operator-(const Matrix &);
	Matrix operator/(float);
	Matrix operator*(float);
	void operator=(const Matrix &);
	void clear();
	void print();
	float AddCol(int col);
	vector<vector<float>> vals;
	~Matrix();
};

#endif