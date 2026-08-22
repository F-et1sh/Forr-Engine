:: update GitHub submodules to the last commit

@echo off

pushd "%~dp0\..\"
git submodule update --init --recursive --remote
popd