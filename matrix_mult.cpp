#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <cstdlib>

using namespace std;
using namespace chrono;

//Умножение матриц с OpenMP
void multiply_matrices(const vector<vector<int>>& A,
                      const vector<vector<int>>& B,
                      vector<vector<int>>& C) {
    int n = A.size();
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

//Генерация матрицы
vector<vector<int>> generate_matrix(int n, int seed = 42) {
    vector<vector<int>> m(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m[i][j] = (seed + i * 7 + j * 13) % 9 + 1;
        }
    }
    return m;
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

//Верификация с Python
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

//Эксперимент для одного размера
double run_experiment(int n, int repeats = 3) {
    auto A = generate_matrix(n);
    auto B = generate_matrix(n);
    vector<vector<int>> C(n, vector<int>(n));
    
    multiply_matrices(A, B, C);
    
    double total_time = 0;
    for (int r = 0; r < repeats; r++) {
        auto start = high_resolution_clock::now();
        multiply_matrices(A, B, C);
        auto end = high_resolution_clock::now();
        total_time += duration_cast<microseconds>(end - start).count() / 1e6;
    }
    
    return total_time / repeats;
}

int main(int argc, char* argv[]) {
    //Получаем количество потоков из аргумента командной строки
    int num_threads = 4;  //по умолчанию
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    
    #ifdef _OPENMP
        omp_set_num_threads(num_threads);
        cout << "OpenMP активен" << endl;
        cout << "Запрошено потоков: " << num_threads << endl;
        cout << "Доступно процессоров: " << omp_get_num_procs() << endl;
        cout << "Используется потоков: " << omp_get_max_threads() << endl;
    #else
        cout << "OpenMP НЕ активен!" << endl;
    #endif
    
    vector<int> sizes = {200, 400, 800, 1200, 1600, 2000};
    
    string filename = "results_" + to_string(num_threads) + ".csv";
    ofstream results_file(filename);
    results_file << "N,time_sec,MFLOPS,threads\n";
    
    cout << "\n========== ЭКСПЕРИМЕНТ (" << num_threads << " потоков) ==========\n" << endl;
    
    for (int n : sizes) {
        cout << "Размер " << n << "x" << n << "... " << flush;
        
        double time_sec = run_experiment(n);
        long long operations = 2LL * n * n * n;
        double mflops = (operations / 1e6) / time_sec;
        
        cout << fixed << setprecision(4);
        cout << "время = " << time_sec << " с, ";
        cout << "MFLOPS = " << mflops << endl;
        
        results_file << n << "," << time_sec << "," << mflops << "," << num_threads << "\n";
    }
    
    results_file.close();
    
    cout << "\nРезультаты сохранены в файл: " << filename << endl;
    
    // Демо-вывод для маленькой матрицы (5x5) с верификацией
    cout << "\n========== ДЕМО (5x5) ==========" << endl;
    int demo_n = 5;
    auto A = generate_matrix(demo_n);
    auto B = generate_matrix(demo_n);
    vector<vector<int>> C(demo_n, vector<int>(demo_n));
    
    auto start = high_resolution_clock::now();
    multiply_matrices(A, B, C);
    auto end = high_resolution_clock::now();
    double demo_time = duration_cast<microseconds>(end - start).count() / 1e6;
    
    save_matrix("matrix1.txt", A);
    save_matrix("matrix2.txt", B);
    save_matrix("result.txt", C);
    
    cout << "Время: " << demo_time << " с" << endl;
    cout << "Верификация: " << (verify_with_python(A, B, C) ? "УСПЕШНО" : "ОШИБКА") << endl;
    
    return 0;
}