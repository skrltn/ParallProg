#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <mpi.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace chrono;

// Функция для генерации случайной матрицы (одинаковой на всех процессах)
vector<vector<int>> generate_random_matrix(int n, int base_seed) {
    vector<vector<int>> matrix(n, vector<int>(n));
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = ((base_seed + i * 7 + j * 13) % 9) + 1;
            matrix[i][j] = val;
        }
    }
    return matrix;
}

// Сохранение матрицы в файл (только на процессе 0)
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

// Верификация с Python
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

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    // Инициализация MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        cout << "MPI Умножение матриц" << endl;
        cout << "Количество процессов: " << size << endl;
    }
    
    // ДЕМО (5x5)
    if (rank == 0) {
        cout << "\nДЕМО (5x5)" << endl;
    }
    
    int demo_n = 5;
    vector<vector<int>> A_demo, B_demo, C_demo;
    
    if (rank == 0) {
        int base_seed = time(NULL) % 1000;
        A_demo = generate_random_matrix(demo_n, base_seed);
        B_demo = generate_random_matrix(demo_n, base_seed + 1000);
        C_demo.resize(demo_n, vector<int>(demo_n, 0));
    }
    
    // Преобразуем матрицы A и B в одномерные массивы для передачи через MPI
    double* A_flat = nullptr;
    double* B_flat = nullptr;
    double* C_flat = nullptr;
    
    if (rank == 0) {
        A_flat = new double[demo_n * demo_n];
        B_flat = new double[demo_n * demo_n];
        C_flat = new double[demo_n * demo_n];
        
        for (int i = 0; i < demo_n; i++) {
            for (int j = 0; j < demo_n; j++) {
                A_flat[i * demo_n + j] = A_demo[i][j];
                B_flat[i * demo_n + j] = B_demo[i][j];
                C_flat[i * demo_n + j] = 0.0;
            }
        }
    }
    
    // Распределяем строки матрицы A между процессами
    int rows_per_proc = demo_n / size;
    int extra_rows = demo_n % size;
    int my_rows = (rank < extra_rows) ? rows_per_proc + 1 : rows_per_proc;
    int start_row = (rank < extra_rows) ? rank * my_rows : extra_rows * (rows_per_proc + 1) + (rank - extra_rows) * rows_per_proc;
    
    // Массивы для распределенных данных
    double* A_local = new double[my_rows * demo_n];
    double* B_local = new double[demo_n * demo_n];
    double* C_local = new double[my_rows * demo_n];
    
    // Инициализация B_local нулями, если не rank 0
    for (int i = 0; i < demo_n * demo_n; i++) {
        B_local[i] = 0.0;
    }
    
    if (rank == 0) {
        // Копируем свою часть A
        for (int i = 0; i < my_rows; i++) {
            for (int j = 0; j < demo_n; j++) {
                A_local[i * demo_n + j] = A_flat[i * demo_n + j];
            }
        }
        // Копируем B целиком
        for (int i = 0; i < demo_n * demo_n; i++) {
            B_local[i] = B_flat[i];
        }
        
        // Отправляем части A другим процессам
        for (int p = 1; p < size; p++) {
            int p_rows = (p < extra_rows) ? rows_per_proc + 1 : rows_per_proc;
            int p_start = (p < extra_rows) ? p * p_rows : extra_rows * (rows_per_proc + 1) + (p - extra_rows) * rows_per_proc;
            MPI_Send(&A_flat[p_start * demo_n], p_rows * demo_n, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(A_local, my_rows * demo_n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // Рассылаем матрицу B всем процессам
    MPI_Bcast(B_local, demo_n * demo_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Измеряем время (только на процессе 0)
    double start_time = MPI_Wtime();
    
    // Умножение: каждый процесс вычисляет свою часть строк
    for (int i = 0; i < my_rows; i++) {
        for (int j = 0; j < demo_n; j++) {
            double sum = 0.0;
            for (int k = 0; k < demo_n; k++) {
                sum += A_local[i * demo_n + k] * B_local[k * demo_n + j];
            }
            C_local[i * demo_n + j] = sum;
        }
    }
    
    // Собираем результаты на процессе 0
    if (rank == 0) {
        // Копируем свой результат
        for (int i = 0; i < my_rows; i++) {
            for (int j = 0; j < demo_n; j++) {
                C_flat[i * demo_n + j] = C_local[i * demo_n + j];
            }
        }
        // Получаем результаты от других процессов
        for (int p = 1; p < size; p++) {
            int p_rows = (p < extra_rows) ? rows_per_proc + 1 : rows_per_proc;
            int p_start = (p < extra_rows) ? p * p_rows : extra_rows * (rows_per_proc + 1) + (p - extra_rows) * rows_per_proc;
            MPI_Recv(&C_flat[p_start * demo_n], p_rows * demo_n, MPI_DOUBLE, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(C_local, my_rows * demo_n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
    
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        // Преобразуем обратно в вектор векторов
        for (int i = 0; i < demo_n; i++) {
            for (int j = 0; j < demo_n; j++) {
                C_demo[i][j] = (int)C_flat[i * demo_n + j];
            }
        }
        
        double seconds = end_time - start_time;
        cout << "Время: " << fixed << setprecision(6) << seconds << " секунд" << endl;
        
        save_matrix("matrix1.txt", A_demo);
        save_matrix("matrix2.txt", B_demo);
        save_matrix("result.txt", C_demo);
        
        cout << "Верификация: " << (verify_with_python(A_demo, B_demo, C_demo) ? "успешно" : "ошибка") << endl;
    }
    
    delete[] A_flat;
    delete[] B_flat;
    delete[] C_flat;
    delete[] A_local;
    delete[] B_local;
    delete[] C_local;
    
    // ===== ЭКСПЕРИМЕНТЫ =====
    if (rank == 0) {
        cout << "\n\n ЗАПУСК ЭКСПЕРИМЕНТОВ " << endl;
    }
    
    // Размеры матриц для исследования
    vector<int> sizes = {200, 400, 800, 1200, 1600, 2000};
    
    // Открываем файл для результатов только на процессе 0
    ofstream results;
    if (rank == 0) {
        results.open("experiment_results_mpi.csv");
        results << "N,time_sec,MFLOPS,processes\n";
    }
    
    for (int n : sizes) {
        // Генерируем матрицы на процессе 0
        vector<vector<int>> A, B;
        double *A_flat_n = nullptr, *B_flat_n = nullptr, *C_flat_n = nullptr;
        
        if (rank == 0) {
            cout << "\n Размер матрицы: " << n << "x" << n << " " << endl;
            int base_seed = time(NULL) % 1000;
            A = generate_random_matrix(n, base_seed);
            B = generate_random_matrix(n, base_seed + 1000);
            
            A_flat_n = new double[n * n];
            B_flat_n = new double[n * n];
            C_flat_n = new double[n * n];
            
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    A_flat_n[i * n + j] = A[i][j];
                    B_flat_n[i * n + j] = B[i][j];
                    C_flat_n[i * n + j] = 0.0;
                }
            }
        }
        
        // Распределяем строки
        int rows_per_proc_n = n / size;
        int extra_rows_n = n % size;
        int my_rows_n = (rank < extra_rows_n) ? rows_per_proc_n + 1 : rows_per_proc_n;
        int start_row_n = (rank < extra_rows_n) ? rank * my_rows_n : extra_rows_n * (rows_per_proc_n + 1) + (rank - extra_rows_n) * rows_per_proc_n;
        
        double* A_local_n = new double[my_rows_n * n];
        double* B_local_n = new double[n * n];
        double* C_local_n = new double[my_rows_n * n];
        
        for (int i = 0; i < n * n; i++) B_local_n[i] = 0.0;
        
        if (rank == 0) {
            for (int i = 0; i < my_rows_n; i++) {
                for (int j = 0; j < n; j++) {
                    A_local_n[i * n + j] = A_flat_n[i * n + j];
                }
            }
            for (int i = 0; i < n * n; i++) {
                B_local_n[i] = B_flat_n[i];
            }
            
            for (int p = 1; p < size; p++) {
                int p_rows = (p < extra_rows_n) ? rows_per_proc_n + 1 : rows_per_proc_n;
                int p_start = (p < extra_rows_n) ? p * p_rows : extra_rows_n * (rows_per_proc_n + 1) + (p - extra_rows_n) * rows_per_proc_n;
                MPI_Send(&A_flat_n[p_start * n], p_rows * n, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
            }
        } else {
            MPI_Recv(A_local_n, my_rows_n * n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        MPI_Bcast(B_local_n, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        // Несколько повторений для точности
        int repeats = 3;
        double total_time = 0;
        
        for (int r = 0; r < repeats; r++) {
            MPI_Barrier(MPI_COMM_WORLD);
            double start_t = MPI_Wtime();
            
            for (int i = 0; i < my_rows_n; i++) {
                for (int j = 0; j < n; j++) {
                    double sum = 0.0;
                    for (int k = 0; k < n; k++) {
                        sum += A_local_n[i * n + k] * B_local_n[k * n + j];
                    }
                    C_local_n[i * n + j] = sum;
                }
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            double end_t = MPI_Wtime();
            total_time += (end_t - start_t);
        }
        
        double avg_time = total_time / repeats;
        
        // Собираем результаты
        if (rank == 0) {
            for (int i = 0; i < my_rows_n; i++) {
                for (int j = 0; j < n; j++) {
                    C_flat_n[i * n + j] = C_local_n[i * n + j];
                }
            }
            for (int p = 1; p < size; p++) {
                int p_rows = (p < extra_rows_n) ? rows_per_proc_n + 1 : rows_per_proc_n;
                int p_start = (p < extra_rows_n) ? p * p_rows : extra_rows_n * (rows_per_proc_n + 1) + (p - extra_rows_n) * rows_per_proc_n;
                MPI_Recv(&C_flat_n[p_start * n], p_rows * n, MPI_DOUBLE, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        } else {
            MPI_Send(C_local_n, my_rows_n * n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        }
        
        if (rank == 0) {
            long long operations = 2LL * n * n * n;
            double mflops = (operations / 1000000.0) / avg_time;
            
            cout << fixed << setprecision(4);
            cout << "Время выполнения: " << avg_time << " секунд" << endl;
            cout << "Количество операций: " << operations << endl;
            cout << "Производительность: " << mflops << " MFLOPS" << endl;
            
            results << n << "," << avg_time << "," << mflops << "," << size << endl;
        }
        
        delete[] A_flat_n;
        delete[] B_flat_n;
        delete[] C_flat_n;
        delete[] A_local_n;
        delete[] B_local_n;
        delete[] C_local_n;
        
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    if (rank == 0) {
        results.close();
        cout << "\n\n РЕЗУЛЬТАТЫ СОХРАНЕНЫ " << endl;
        cout << "Файл: experiment_results_mpi.csv" << endl;
    }
    
    MPI_Finalize();
    return 0;
}