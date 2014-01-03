@echo off

if [%1] == [] (
	goto printUsage
)
if [%2] == [] (
	goto printUsage
)

set CONFIGURATION=Release
rem set CONFIGURATION=Debug

set testerDir="tools/Benchmarks"
set tester="test.bat"
set solver="%cd%/build/msvc/MortalCoil/%CONFIGURATION%/MortalCoil.exe"


pushd %testerDir%
call %tester% %solver% %1 %2
popd

goto end

:printUsage
echo Usage: run_benchmarks.bat fromLevel toLevel

:end
