#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace std::chrono;

void generateMatrix(vector<vector<double>>& matrix, int n);
bool readMatrix(const string& filename, vector<vector<double>>& matrix);
bool checkSizes(const vector<vector<double>>& A, const vector<vector<double>>& B);
double multiplyMatricesSequential(const vector<vector<double>>& A,
                                   const vector<vector<double>>& B,
                                   vector<vector<double>>& C);
void printMatrix(const vector<vector<double>>& matrix, int maxRows = 10);
void saveResult(const string& filename, const vector<vector<double>>& matrix, 
                double time, int N, const vector<vector<double>>& A, 
                const vector<vector<double>>& B);
void saveExperimentResults(const vector<int>& sizes, 
                          const vector<double>& times,
                          const vector<double>& mflops,
                          const string& config);
bool verifyResult(const vector<vector<double>>& C, 
                  const vector<vector<double>>& A,
                  const vector<vector<double>>& B);

#endif
