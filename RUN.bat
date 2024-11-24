@echo off
rem Verifica se o GCC está instalado
gcc --version >nul 2>&1
if errorlevel 1 (
    echo GCC nao foi encontrado. Por favor, instale o MinGW ou outro compilador C.
    pause
    exit /b
)

rem Compila o codigo imgFourrier_Comentado.c
echo Compilando o codigo...
gcc imgFourrier_Comentado.c -o imgFourrier_Comentado.exe -lm
if errorlevel 1 (
    echo Ocorreu um erro durante a compilacao.
    pause
    exit /b
)

rem Executa o programa compilado
echo Executando o programa...
imgFourrier_Comentado.exe
if errorlevel 1 (
    echo Ocorreu um erro durante a execucao do programa.
    pause
    exit /b
)

rem Fim
echo Programa executado com sucesso.
pause
