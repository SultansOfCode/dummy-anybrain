cls
del *.obj
del *.dll
cl /LD /EHsc /nologo /D "NDEBUG" /O2 *.cpp /Z7 /DEBUG:NONE
