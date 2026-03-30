#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <omp.h>

using namespace std;
using namespace chrono;

//Функция для умножения матриц с параллелизацией через OpenMP
void multiply_matrices(const vector<vector<int>>& A, 
                      const vector<vector<int>>& B, 
                      vector<vector<int>>& C) {
    int n = A.size();
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

//Генерация случайной матрицы
vector<vector<int>> generate_random_matrix(int n) {
    vector<vector<int>> matrix(n, vector<int>(n));
    time_t t = time(NULL);
    int base = t % 1000;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = ((base + i * 7 + j * 13) % 9) + 1;
            matrix[i][j] = val;
        }
    }
    return matrix;
}

//Сохранение матрицы в файл
void save_matrix(const string& filename, const vector<vector<int>>& matrix) {
    ofstream file(filename);
    int n = matrix.size();
    file << n << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << matrix[i][j] << " ";
        }
        file << endl;
    }
    file.close();
}

//Верификация
bool verify_with_python(const vector<vector<int>>& A, 
                       const vector<vector<int>>& B, 
                       const vector<vector<int>>& C) {
    ofstream temp("temp_verify.txt");
    int n = A.size();
    temp << n << endl;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) temp << A[i][j] << " ";
        temp << endl;
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) temp << B[i][j] << " ";
        temp << endl;
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) temp << C[i][j] << " ";
        temp << endl;
    }
    temp.close();
    
    int result = system("python verify.py");
    return result == 0;
}

//Функция для проведения эксперимента с заданным размером
void run_experiment(int n, ofstream& results) {
    cout << "\n=== Размер матрицы: " << n << "x" << n << " ===" << endl;
    
    auto A = generate_random_matrix(n);
    auto B = generate_random_matrix(n);
    vector<vector<int>> C(n, vector<int>(n));
    
    multiply_matrices(A, B, C);
    
    int repeats = 3;
    double total_time = 0;
    
    for (int r = 0; r < repeats; r++) {
        auto start = high_resolution_clock::now();
        multiply_matrices(A, B, C);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        total_time += duration.count() / 1000000.0;
    }
    
    double avg_time = total_time / repeats;
    long long operations = 2LL * n * n * n;
    double mflops = (operations / 1000000.0) / avg_time;
    
    cout << fixed << setprecision(4);
    cout << "Время выполнения: " << avg_time << " секунд" << endl;
    cout << "Количество операций: " << operations << endl;
    cout << "Производительность: " << mflops << " MFLOPS" << endl;
    
    results << n << "," << avg_time << "," << mflops << "," << omp_get_max_threads() << endl;
}

int main() {
    #ifdef _OPENMP
        cout << "OpenMP активен. Максимум потоков: " << omp_get_max_threads() << endl;
    #else
        cout << "OpenMP не активен" << endl;
    #endif
    
    // Демо для 5x5
    cout << "\n=== ДЕМО (5x5) ===" << endl;
    int demo_n = 5;
    auto A = generate_random_matrix(demo_n);
    auto B = generate_random_matrix(demo_n);
    vector<vector<int>> C(demo_n, vector<int>(demo_n));
    
    auto start = high_resolution_clock::now();
    multiply_matrices(A, B, C);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    double seconds = duration.count() / 1000000.0;
    
    save_matrix("matrix1.txt", A);
    save_matrix("matrix2.txt", B);
    save_matrix("result.txt", C);
    
    cout << "Время: " << seconds << " секунд" << endl;
    cout << "Верификация: " << (verify_with_python(A, B, C) ? "успешно" : "ошибка") << endl;
    
    //ЭКСПЕРИМЕНТЫ
    cout << "\n\n========== ЗАПУСК ЭКСПЕРИМЕНТОВ ==========" << endl;
    
    //Открываем файл для результатов
    ofstream results("experiment_results.csv");
    results << "N,time_sec,MFLOPS,threads\n";
    
    //Размеры матриц для исследования
    vector<int> sizes = {200, 400, 800, 1200, 1600, 2000};
    
    for (int n : sizes) {
        run_experiment(n, results);
    }
    
    results.close();
    
    cout << "\n\n========== РЕЗУЛЬТАТЫ СОХРАНЕНЫ ==========" << endl;
    cout << "Файл: experiment_results.csv" << endl;
    
    return 0;
}