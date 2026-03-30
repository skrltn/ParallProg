@echo off
chcp 65001 >nul
echo ========================================
echo   Лабораторная работа №2
echo   Исследование масштабируемости OpenMP
echo   Процессор: 2 ядра / 4 потока
echo ========================================
echo.

echo Компиляция...
g++ -fopenmp -O2 matrix_mult.cpp -o matrix_mult.exe
if errorlevel 1 (
    echo Ошибка компиляции!
    pause
    exit /b 1
)
echo Компиляция успешна!
echo.

for %%t in (1 2 4) do (
    echo.
    echo ========== ЗАПУСК С %%t ПОТОКАМИ ==========
    matrix_mult.exe %%t
    echo.
)

echo.
echo ========================================
echo   Все эксперименты завершены!
echo   Результаты в файлах results_1.csv
echo   results_2.csv и results_4.csv
echo ========================================
pause