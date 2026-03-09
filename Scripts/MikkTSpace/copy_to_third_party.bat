::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy MikkTSpace include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\MikkTSpace\\mikktspace.h" "..\\..\\ThirdParty\\MikkTSpace\\include\\" >nul
xcopy /y /i /s "..\\..\\External\\MikkTSpace\\mikktspace.c" "..\\..\\ThirdParty\\MikkTSpace\\src\\" >nul