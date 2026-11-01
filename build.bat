@echo off
rem Thin wrapper so you can double-click / run from cmd.
rem   build.bat                 -> build + test
rem   build.bat -Target bench   -> build + run benchmark
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*
exit /b %errorlevel%
