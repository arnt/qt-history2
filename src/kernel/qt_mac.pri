mac {
   macx {
        LIBS += -framework Carbon -framework QuickTime
	DEFINES += CARBON_ON_MACH_O=1 ALLOW_OLD_BLOCKING_APIS=0 ALLOW_OLD_EVENT_LOOP_APIS=0
  }
  *-mwerks:INCLUDEPATH += compat
  mac9 {
     LIBS       += "MSL C++.PPC.Lib" "MSL SIOUX.Carbon.Lib" "CarbonLib" \
                   "MSL RuntimePPC.Lib" "MSL C.Carbon.Lib" 
     #INCLUDEPATH += "MacOS 9:CarbonLib_1.2_SDK:Carbon Support:Universal Interfaces:CIncludes" 
   }
}
