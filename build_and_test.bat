@echo off
setlocal EnableDelayedExpansion

echo =========================================================
echo   CIVICCARE: Compiling Member 2 (User Management and Auth)
echo =========================================================

set SOURCES=src/models/User.cpp src/models/Citizen.cpp src/models/MunicipalStaff.cpp src/models/Administrator.cpp src/models/Session.cpp src/repositories/FileUserRepository.cpp src/services/AuthenticationService.cpp src/utils/PasswordHasher.cpp

g++ -std=c++14 -Iinclude %SOURCES% tests/test_user_management.cpp -o test_user_management.exe

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Test compilation failed!
    exit /b %ERRORLEVEL%
)

echo [+] test_user_management.exe compiled successfully!
echo.
echo Running Tests...
test_user_management.exe

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Tests failed!
    exit /b %ERRORLEVEL%
)

echo.
echo =========================================================
echo   Compiling Member 2 Demonstration Program
echo =========================================================

g++ -std=c++14 -Iinclude %SOURCES% demo/main_member2_demo.cpp -o civiccare_member2_demo.exe

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Demo compilation failed!
    exit /b %ERRORLEVEL%
)

echo [+] civiccare_member2_demo.exe compiled successfully!
echo Run 'civiccare_member2_demo.exe' to execute live demonstration.
