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

set inputFile="mortal_coil.txt"
set outputFile="output.txt"
set levelDataFile="..\..\data\levels\Level%%i"
set timesFile="times\time%%i"
set checker="..\Solution Checker\Solution Checker.exe"

for /l %%i in (%fromLevel%, 1, %toLevel%) do (
	< nul set /p a="Level %%i:	"

	if exist %levelDataFile% (
		copy %levelDataFile% %inputFile% > nul
	) else (
		echo No data file for level %%i!
		exit /b
	)
	if exist %outputFile% del /q %outputFile%
	if exist %timesFile% del /q %timesFile%

	for %%e in (%solver%) do (
		start "" /d "%cd%" /b /w "%%e" > %timesFile%
rem		start "" /d "%cd%" /b /w "%%e"
	)

	if exist %outputFile% (
		start "" /b /w %checker% < %outputFile%
		< nul set /p a=.	Time elapsed: 
		type %timesFile%
	) else (
		echo Solver did not write to output file!
	)
)

:cleanup
if exist %outputFile% del /q %outputFile%
if exist %inputFile% del /q %inputFile%
exit /b

:printUsage
echo Usage: test.bat "solver.exe" levelFrom levelTo
exit /b
