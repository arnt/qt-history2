mac {
   macx:LIBS += -framework Carbon -framework QuickTime
   *-mwerks:INCLUDEPATH += compat
   mac9 {
     LIBS       += "MSL C++.PPC.Lib" "MSL SIOUX.Carbon.Lib" "CarbonLib" \
                   "MSL RuntimePPC.Lib" "MSL C.Carbon.Lib" 
     #INCLUDEPATH += "MacOS 9:CarbonLib_1.2_SDK:Carbon Support:Universal Interfaces:CIncludes" 
   }
}
