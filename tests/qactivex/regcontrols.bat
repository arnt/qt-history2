@echo off
examples\simple\simpleax -regserver
examples\tetrix\tetrixax -regserver
examples\opengl\openglax -regserver

regsvr32 /s examples\multiple\multipleax.dll
regsvr32 /s examples\wrapper\wrapperax.dll
