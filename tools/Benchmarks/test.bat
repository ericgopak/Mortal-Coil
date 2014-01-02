rem test.bat "solver.exe" level_from level_to

@echo off
for /l %%i in (%2, 1, %3) do (
rem for /l %%i in (0, 1, 100) do (
	< nul set /p a="Level %%i:	"
	copy "levels\Level%%i" "mortal_coil.txt" > nul
	if exist "output.txt" del /q "output.txt"
	"%1" > "times\time%%i"
	if exist "output.txt" "..\Solution Tester.exe" < "output.txt"
)
