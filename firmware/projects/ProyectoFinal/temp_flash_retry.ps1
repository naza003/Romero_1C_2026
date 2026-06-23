Set-ExecutionPolicy -Scope Process Bypass -Force
. 'C:\esp\v5.5.3\esp-idf\export.ps1'
& python "$Env:IDF_PATH\tools\idf.py" -p COM12 flash
