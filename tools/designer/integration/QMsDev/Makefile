#
#
#
MTLOPTIONS=/nologo /D "NDEBUG" /tlb QMsDev.tlb /h QMsDevTypes.h /mktyplib203 /win32 
STDAFXOPTIONS=/nologo /MD /W3 /GX /O2 /I "_$(QTDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fpqmsdev.pch /Yc"stdafx.h" /Fo.\\ /Fd.\\ /FD /c 
LINKOPTS=/nologo /subsystem:windows /dll /incremental:no /pdb:qmsdev.pdb /machine:I386 /def:QMsDev.def /out:qmsdev.dll /implib:qmsdev.lib 
CPPOPTS=/nologo /MD /W3 /GX /O2 /I "_$(QTDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fpqmsdev.pch /Yu"stdafx.h" /Fo.\\ /Fd.\\ /FD /c 
#
OBJS= Commands.obj DSAddIn.obj NewQtProjectDialog.obj QMsDev.obj StdAfx.obj QMsDev.res
DEF_FILE=QMsDev.def
#
MTL=midl.exe
RSC=rc.exe
LINK=link.exe
CPP=cl.exe
COPY=copy


ALL : qmsdev.dll QMsDev.tlb QMsDevTypes.h

CLEAN :
	-@erase "Commands.obj"
	-@erase "DSAddIn.obj"
	-@erase "NewQtProjectDialog.obj"
	-@erase "QMsDev.obj"
	-@erase "qmsdev.pch"
	-@erase "QMsDev.res"
	-@erase "QMsDev.tlb"
	-@erase "StdAfx.obj"
	-@erase "vc60.idb"
	-@erase "qmsdev.dll"
	-@erase "qmsdev.exp"
	-@erase "qmsdev.lib"
	-@erase "QMsDevTypes.h"

qmsdev.dll: $(DEF_FILE) $(OBJS)
   $(LINK) $(LINKOPTS) $(OBJS)
   $(COPY) qmsdev.dll "$(MSDEVDIR)\Addins"

QMsDev.tlb: QMsDev.odl
	$(MTL) $(MTLOPTIONS) QMsDev.odl

QMsDev.res: QMsDev.rc QMsDev.tlb
	$(RSC) /l 0x407 /foQMsDev.res /d "NDEBUG" /d "_AFXDLL" QMsDev.rc

StdAfx.obj: StdAfx.cpp StdAfx.pch
	$(CPP) $(STDAFXOPTIONS) StdAfx.cpp

StdAfx.pch: StdAfx.cpp
	$(CPP) $(STDAFXOPTIONS) StdAfx.cpp

Commands.obj: Commands.cpp StdAfx.pch QMsDevTypes.h
   $(CPP) $(CPPOPTS) Commands.cpp

DSAddIn.obj: DSAddIn.cpp StdAfx.pch
   $(CPP) $(CPPOPTS) DSAddIn.cpp

NewQtProjectDialog.obj: NewQtProjectDialog.cpp StdAfx.pch
   $(CPP) $(CPPOPTS) NewQtProjectDialog.cpp

QMsDev.obj: QMsDev.cpp StdAfx.pch
   $(CPP) $(CPPOPTS) QMsDev.cpp

QMsDevTypes.h: QMsDev.tlb
