cl /Feparatechlib.exe ./src/main.c /I"./include" /DOPENSSL_USE_STATIC_LIBS /MT /link /LIBPATH:"./lib" libssl.lib libcrypto.lib ws2_32.lib

