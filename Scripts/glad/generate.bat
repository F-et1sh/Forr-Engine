::===============================================
::
::  Forr Engine - Scripts
::
::  File : generate.bat
::  Role : generate glad include and sources
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\glad"

python -m glad --api gl:core=4.6 --out-path build

popd