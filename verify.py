import numpy as np
import sys

def verify():
    try:
        with open('temp_verify.txt', 'r') as f:
            n = int(f.readline().strip())
            
            # Читаем A
            A = []
            for _ in range(n):
                A.append(list(map(int, f.readline().strip().split())))
            
            # Читаем B
            B = []
            for _ in range(n):
                B.append(list(map(int, f.readline().strip().split())))
            
            # Читаем C (результат C++)
            C_cpp = []
            for _ in range(n):
                C_cpp.append(list(map(int, f.readline().strip().split())))
        
        # Преобразуем в numpy
        A_np = np.array(A)
        B_np = np.array(B)
        C_cpp_np = np.array(C_cpp)
        
        # Вычисляем правильный результат
        C_correct = np.dot(A_np, B_np)
        
        # Сравниваем
        if np.array_equal(C_cpp_np, C_correct):
            print("VERIFICATION_SUCCESS")
            sys.exit(0)
        else:
            print("VERIFICATION_FAILED")
            sys.exit(1)
            
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

if __name__ == "__main__":
    verify()