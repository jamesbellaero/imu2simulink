@echo off
if NOT [%1] == [] cd %1

if exist *.ncb del /F /Q *.ncb
if exist *.suo del /F /Q /AH *.suo

if exist x64 rmdir /s /q x64
if exist Win32 rmdir /s /q Win32

if exist scenarios.xsb del scenarios.xsb

if NOT [%2] == [] cd %2
