@echo off
if exist "./DFU_USR_CODE.bin" (
  DEL DFU_USR_CODE.bin
)
if exist "./DFU_CODE.bin" (
  DEL DFU_CODE.bin
)
if exist "./*.bin" (
  for /f "delims=" %%i in ('dir /a-d /b *.bin') do (
    echo %%~zi>"usr_size.cfg"
  )
  copy /b sys.cfg + usr_size.cfg + *.bin + *.fot  DFU_USR_CODE.bin
  DEL usr_size.cfg
) else (
  copy /b *.fot DFU_CODE.bin
)
pause
