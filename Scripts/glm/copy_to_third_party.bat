::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy GLM include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\glm\\glm" "..\\..\\ThirdParty\\glm\\include\\glm" >nul
copy "..\\..\\External\\glm\\copying.txt" "..\\..\\ThirdParty\\glm\\GLM_LICENSE.txt" >nul
