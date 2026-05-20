#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include "functions.h"

using namespace std;

__global__ void multiplyMatricesKernel(const double* A, const double* B, double* C, int n) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (row < n && col < n) {
        double sum = 0;
        for (int k = 0; k < n; k++) {
            sum += A[row * n + k] * B[k * n + col];
        }
        C[row * n + col] = sum;
    }
}

double runCUDAMultiply(const double* A, const double* B, double* C, int n, int blockSizeX, int blockSizeY) {
    double *d_A, *d_B, *d_C;
    size_t size = n * n * sizeof(double);
    
    cudaMalloc(&d_A, size);
    cudaMalloc(&d_B, size);
    cudaMalloc(&d_C, size);
    
    cudaMemcpy(d_A, A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B, size, cudaMemcpyHostToDevice);
    
    dim3 threadsPerBlock(blockSizeX, blockSizeY);
    dim3 numBlocks((n + blockSizeX - 1) / blockSizeX, (n + blockSizeY - 1) / blockSizeY);
    
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    cudaEventRecord(start);
    
    multiplyMatricesKernel<<<numBlocks, threadsPerBlock>>>(d_A, d_B, d_C, n);
    
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    cudaMemcpy(C, d_C, size, cudaMemcpyDeviceToHost);
    
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    
    return milliseconds / 1000.0;
}

int main() {
    vector<int> sizes = {200, 400, 800, 1200, 1600, 2000};
    
    vector<pair<int,int>> configs = {{16,16}, {32,32}, {8,8}, {16,8}, {32,16}};
    vector<string> configNames = {"16x16", "32x32", "8x8", "16x8", "32x16"};
    
    for (size_t c = 0; c < configs.size(); c++) {
        cout << "========================================\n";
        cout << "КОНФИГУРАЦИЯ: " << configNames[c] << "\n";
        cout << "========================================\n\n";
        
        vector<double> experimentTimes;
        vector<double> experimentMflops;
        
        for (int N : sizes) {
            cout << "Размер матрицы: " << N << "x" << N << endl;
            
            vector<vector<double>> A(N, vector<double>(N));
            vector<vector<double>> B(N, vector<double>(N));
            vector<vector<double>> C(N, vector<double>(N));
            
            generateMatrix(A, N);
            generateMatrix(B, N);
            
            vector<double> A_flat(N * N);
            vector<double> B_flat(N * N);
            vector<double> C_flat(N * N);
            
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    A_flat[i * N + j] = A[i][j];
                    B_flat[i * N + j] = B[i][j];
                }
            }
            
            cout << "Выполняется умножение (CUDA)... " << flush;
            
            double time = runCUDAMultiply(A_flat.data(), B_flat.data(), C_flat.data(), N, configs[c].first, configs[c].second);
            
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    C[i][j] = C_flat[i * N + j];
                }
            }
            
            double mflops = (2.0 * N * N * N / time / 1e6);
            
            experimentTimes.push_back(time);
            experimentMflops.push_back(mflops);
            
            cout << "Готово\n";
            cout << "Время: " << fixed << setprecision(4) << time << " сек\n";
            cout << "Производительность: " << fixed << setprecision(2) << mflops << " MFLOPS\n\n";
            
            if (N == 200 && c == 0) {
                saveResult("result_200_cuda.txt", C, time, N, A, B);
                cout << "Результат для размера 200x200 сохранен в result_200_cuda.txt\n\n";
            }
            
            if (!verifyResult(C, A, B)) {
                cout << "ОШИБКА: Верификация не пройдена!\n";
                return 1;
            }
        }
        
        saveExperimentResults(sizes, experimentTimes, experimentMflops, configNames[c]);
        cout << "Результаты сохранены в experiment_results_cuda_" << configNames[c] << ".txt\n\n";
    }
    
    cout << "========================================\n";
    cout << "ВСЕ ЭКСПЕРИМЕНТЫ ЗАВЕРШЕНЫ УСПЕШНО\n";
    cout << "========================================\n";
    
    return 0;
}
