"%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch x64 && cl ./src/main.c /Feparatechlib.exe /I"C:\Users\Louis\vcpkg\installed\x64-windows\include" /link /LIBPATH:"C:\Users\Louis\vcpkg\installed\x64-windows\lib" libssl.lib libcrypto.lib ws2_32.lib


