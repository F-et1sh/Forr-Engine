:: copy all licenses to the build folder.
:: this script can't be called from the Explorer.
:: it must be called from MS Visual Studio when build as Post Build Event

@echo off

set OUTPUT_DIR=%~1
set SOLUTION_DIR=%~2

set LICENSE_DIR=%OUTPUT_DIR%ThirdPartyLicenses

if not exist "%LICENSE_DIR%" (
    mkdir "%LICENSE_DIR%"
)

for /r "%SOLUTION_DIR%ThirdParty" %%f in (*LICENSE*) do (
    echo Copying %%f
    copy /y "%%f" "%LICENSE_DIR%" >nul
)

if exist "%SOLUTION_DIR%LICENSE.txt" (
    copy /y "%SOLUTION_DIR%LICENSE.txt" "%OUTPUT_DIR%LICENSE.txt" >nul
)

echo Licenses collected