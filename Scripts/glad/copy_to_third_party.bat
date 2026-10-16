::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy glad include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\glad\\build" "..\\..\\ThirdParty\\glad" >nul
copy "..\\..\\External\\glad\\LICENSE" "..\\..\\ThirdParty\\glad\\GLAD_LICENSE" >nul
