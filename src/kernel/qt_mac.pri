mac {
   macx {
	macx-*:INCLUDEPATH += /System/Library/Frameworks/QuickTime.framework/Headers/ \
	               /System/Library/Frameworks/Carbon.framework/Headers/ \
          /System/Library/Frameworks/Carbon.framework/Frameworks/HIToolbox.framework/Headers \
        /System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Headers/
	LIBS += -framework Carbon -framework QuickTime
	macx-*: QMAKE_LFLAGS_SHLIB += -prebind -seg1addr 0x90000000
  }
  *-mwerks:INCLUDEPATH += compat
  mac9 {
     LIBS       += "MSL C++.PPC.Lib" "MSL SIOUX.Carbon.Lib" "CarbonLib" \
                   "MSL RuntimePPC.Lib" "MSL C.Carbon.Lib" 
     #INCLUDEPATH += "MacOS 9:CarbonLib_1.2_SDK:Carbon Support:Universal Interfaces:CIncludes" 
  }
}
