::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy EnTT include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\entt\\single_include" "..\\..\\ThirdParty\\entt\\include" >nul
copy "..\\..\\External\\entt\\LICENSE" "..\\..\\ThirdParty\\entt\\ENTT_LICENSE" >nul
