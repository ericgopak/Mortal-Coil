@echo off

if [%1] == [] goto printUsage
if [%2] == [] goto printUsage

set solver=%1
set fromLevel=%2

if [%3] == [] (
	set toLevel=%2
) else (
	set toLevel=%3
)

for /l %%i in (%fromLevel%, 1, %toLevel%) do (
	< nul set /p a="Level %%i:	"
	copy "..\..\data\levels\Level%%i" "mortal_coil.txt" > nul
	if exist "output.txt" del /q "output.txt"

	for %%e in (%solver%) do (
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
