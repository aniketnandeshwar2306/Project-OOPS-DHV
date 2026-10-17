@echo off
echo =========================================================
echo   Pushing branch 'ak' to GitHub...
echo =========================================================

git push -u origin ak

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Branch 'ak' pushed to GitHub successfully!
) else (
    echo.
    echo [NOTE] If a browser window opened, please complete the GitHub sign-in.
)

pause
