@echo off
examples\simple\simpleax -unregserver
examples\tetrix\tetrixax -unregserver
examples\opengl\openglax -unregserver

regsvr32 /u /s examples\multiple\multipleax.dll
regsvr32 /u /s examples\wrapper\wrapperax.dll
