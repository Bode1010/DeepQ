#include "Matrix.h"

Matrix::Matrix(int rows, int columns, bool random)
{
	if (random) {
		for (unsigned int i = 0; i < rows; i++) {
			vals.push_back(vector<float>());
			for (unsigned int j = 0; j < columns; j++) {
				vals.back().push_back(((rand()%1000)/1000.0) - 0.5);
			}
		}
	}
	else {
		for (unsigned int i = 0; i < rows; i++) {
			vals.push_back(vector<float>());
			for (unsigned int j = 0; j < columns; j++) {
				vals.back().push_back(0);
			}
		}
	}
}

Matrix Matrix::operator+(const Matrix &c) {
	Matrix result(vals.size(), vals[0].size(), false);
	try {
		if (vals.size() != c.vals.size() || vals[0].size() != c.vals[0].size()) {
			throw - 1;
		}
		else {
			for (unsigned int i = 0; i < vals.size(); i++) {
				for (unsigned int j = 0; j < vals[0].size(); j++){
					result.vals[i][j] = vals[i][j] + c.vals[i][j];
				}
			}
		}
	}
	catch (int e) {
		cout << "The matrices to be added were not the same size " << e << endl;
	}

	return result;
}

Matrix Matrix::operator*(const Matrix &c) {
	Matrix result(vals.size(), c.vals[0].size(), false);
	try {
		if (vals[0].size() != c.vals.size()) {
			throw - 2;
		}
		else {
			for (unsigned int k = 0; k < c.vals[0].size(); k++) {
				for (unsigned int i = 0; i < vals.size(); i++) {
					int partnerrows = 0;
					float sum = 0;
					for (unsigned int j = 0; j < vals[0].size(); j++) {
						sum += 1.0 * vals[i][j] * c.vals[partnerrows][k];
						partnerrows++;
					}
					result.vals[i][k] = sum;
				}
			}
		}
	}
	catch (int e) {
		cout << "The matrix sizes do not match up for multiplication " << e << endl;
	}
	return result;
}

Matrix Matrix::operator-(const Matrix &c) {
	Matrix result(vals.size(), vals[0].size(), false);
	try {
		if (vals.size() != c.vals.size() || vals[0].size() != c.vals[0].size()) {
			throw - 1;
		}
		else {
			for (unsigned int i = 0; i < vals.size(); i++) {
				for (unsigned int j = 0; j < vals[0].size(); j++) {
					result.vals[i][j] = vals[i][j] - c.vals[i][j];
				}
			}
		}
	}
	catch (int e) {
		cout << "The matrices to be subtracted were not the same size " << e << endl;
	}

	return result;
}

Matrix Matrix::operator/(float c) {
	Matrix result(vals.size(), vals[0].size(), false);
	for (int i = 0; i < vals.size(); i++) {
		for (int j = 0; j < vals[0].size(); j++) {
			result.vals[i][j] = 1.0 * vals[i][j] / c * 1.0;
		}
	}
	return result;
}

Matrix Matrix::operator*(float c) {
	Matrix result(vals.size(), vals[0].size(), false);
	for (int i = 0; i < vals.size(); i++) {
		for (int j = 0; j < vals[0].size(); j++) {
			result.vals[i][j] = 1.0 * vals[i][j] * c * 1.0;
		}
	}
	return result;
}

void Matrix::operator=(const Matrix &c) {
	this->vals = c.vals;
}

void Matrix::clear() {
	for (int i = 0; i < vals.size(); i++) {
		for (int j = 0; j < vals[0].size(); j++) {
			vals[i][j] = 0;
		}
	}
}

float Matrix::AddCol(int col) {
	float sum = 0;
	for (int i = 0; i < vals.size(); i++) {
		sum += vals[i][col];
	}
	return sum;
}

void Matrix::print() {
	for (unsigned int i = 0; i < vals.size(); i++) {
		for (unsigned int j = 0; j < vals[0].size(); j++) {
			cout << vals[i][j] << " ";
		}
		cout << endl;
	}
}
Matrix::~Matrix()
{
}
