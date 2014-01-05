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

set levelDataFile="..\..\data\levels\Level%%i"

for /l %%i in (%fromLevel%, 1, %toLevel%) do (
	< nul set /p a="Level %%i:	"

	if exist %levelDataFile% (
		copy %levelDataFile% "mortal_coil.txt" > nul
	) else (
		echo No data file for level %%i!
		exit /b
	)
	if exist "output.txt" del /q "output.txt"

	for %%e in (%solver%) do (
		start "" /d "%cd%" /b /w "%%e" > "times\time%%i"
	)
	
	if exist "output.txt" "..\Solution Tester\Solution Tester.exe" < "output.txt"
)

:cleanup
if exist "output.txt" del /q "output.txt"
if exist "mortal_coil.txt" del /q "mortal_coil.txt"
exit /b

:printUsage
echo Usage: test.bat "solver.exe" levelFrom levelTo
exit /b
