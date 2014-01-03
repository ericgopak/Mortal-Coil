@echo off

if [%1] == [] goto printUsage
if [%2] == [] goto printUsage
if [%3] == [] goto printUsage


for /l %%i in (%2, 1, %3) do (
	< nul set /p a="Level %%i:	"
	copy "..\..\data\levels\Level%%i" "mortal_coil.txt" > nul
	if exist "output.txt" del /q "output.txt"

	for %%e in (%1) do (
		start "" /d "%cd%" /b /w "%%e" > "times\time%%i"
	)
	
	if exist "output.txt" "..\Solution Tester\Solution Tester.exe" < "output.txt"
)

rem if exist "output.txt" del /q "output.txt"
rem if exist "mortal_coil.txt" del /q "mortal_coil.txt"
goto end

:printUsage
echo Usage: test.bat "solver.exe" levelFrom levelTo
goto end

:end
