@echo off
rem Use to copy files from project folders into local folder so they're where the user will have them

call:removeDir xstypes
call:removeDir xcommunication
call:removeDir include
call:removeDir winusb

xcopy /s /y ..\..\xstypes\src\*.* xstypes\
xcopy /s /y ..\..\xcommunication\src\*.* xcommunication\
xcopy /s /y ..\..\3p\microsoft\*.* winusb\

xcopy /s /y ..\..\xstypes\src\*.h include\xsens\
xcopy /s /y ..\..\xcommunication\src\*.h include\xcommunication\
xcopy /s /y ..\..\3p\microsoft\*.* winusb\

goto:eof

:removeDir
	if exist %~1 (
		move /y %~1 %~1.1
		rmdir /s /q %~1.1
	)
goto:eof
