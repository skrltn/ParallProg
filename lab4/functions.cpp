#include "functions.h"
#include <string>

void generateMatrix(vector<vector<double>>& matrix, int n) {
    matrix.resize(n, vector<double>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (i + 1) * (j + 1);
        }
    }
}

bool readMatrix(const string& filename, vector<vector<double>>& matrix) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    int n;
    file >> n;
    matrix.resize(n, vector<double>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> matrix[i][j];
        }
    }
    file.close();
    return true;
}

bool checkSizes(const vector<vector<double>>& A, const vector<vector<double>>& B) {
    if (A.empty() || B.empty()) return false;
    return (A.size() == B.size() && A[0].size() == B[0].size());
}

double multiplyMatricesSequential(const vector<vector<double>>& A,
                                   const vector<vector<double>>& B,
                                   vector<vector<double>>& C) {
    int n = A.size();
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
        }
    }
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration_us = duration_cast<microseconds>(end - start).count();
    return duration_us / 1000000.0;
}

void printMatrix(const vector<vector<double>>& matrix, int maxRows) {
    int n = matrix.size();
    int rowsToPrint = min(n, maxRows);
    for (int i = 0; i < rowsToPrint; i++) {
        for (int j = 0; j < n; j++) {
            cout << matrix[i][j] << "\t";
        }
        cout << "\n";
    }
    if (n > maxRows) {
        cout << "...\n";
    }
}

void saveResult(const string& filename, const vector<vector<double>>& matrix, 
                double time, int N, const vector<vector<double>>& A, 
                const vector<vector<double>>& B) {
    ofstream file(filename);
    
    file << "РЕЗУЛЬТАТЫ ЭКСПЕРИМЕНТА\n";
    file << "Размер матрицы: " << N << "x" << N << "\n";
    file << "Технология: CUDA\n";
    file << "Время выполнения: " << fixed << setprecision(6) << time << " секунд\n";
    file << "Количество операций: " << 2.0 * N * N * N << "\n";
    
    double mflops = (time > 0) ? (2.0 * N * N * N / time / 1e6) : 0;
    file << "Производительность: " << fixed << setprecision(2) << mflops << " MFLOPS\n\n";
    
    file << "Исходные данные:\n";
    file << "Матрица A:\n";
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++)
            file << setprecision(0) << A[i][j] << " ";
        file << "\n";
    }
    
    file << "\nМатрица B:\n";
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++)
            file << setprecision(0) << B[i][j] << " ";
        file << "\n";
    }
    
    file << "\nРезультат C:\n";
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++)
            file << setprecision(0) << matrix[i][j] << " ";
        file << "\n";
    }
    
    file << "\nВерификация: успешно\n";
    file.close();
}

void saveExperimentResults(const vector<int>& sizes, 
                          const vector<double>& times,
                          const vector<double>& mflops,
                          const string& config) {
    string filename = "experiment_results_cuda_" + config + ".txt";
    ofstream file(filename);
    
    file << "ЭКСПЕРИМЕНТАЛЬНЫЕ ДАННЫЕ\n";
    file << "Конфигурация блоков: " << config << "\n";
    file << "========================================\n";
    file << "Размер\tВремя(сек)\tMFLOPS\n";
    file << "========================================\n";
    
    for (size_t i = 0; i < sizes.size(); i++) {
        file << sizes[i] << "\t" 
             << fixed << setprecision(4) << times[i] << "\t\t"
             << fixed << setprecision(2) << mflops[i] << "\n";
    }
    
    file << "\n========================================\n";
    file.close();
}

bool verifyResult(const vector<vector<double>>& C, 
                  const vector<vector<double>>& A,
                  const vector<vector<double>>& B) {
    int n = A.size();
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double expected = 0;
            for (int k = 0; k < n; k++) {
                expected += A[i][k] * B[k][j];
            }
            if (abs(C[i][j] - expected) > 1e-6) {
                return false;
            }
        }
    }
    return true;
}
