/****************************************************************************
** $Id: $
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "unixmake.h"
#include "option.h"
#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <time.h>


void
UnixMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) /* no point */
	return;

    QStringList &configs = project->variables()["CONFIG"];
    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["QMAKE"].isEmpty())
	    project->variables()["QMAKE"].append("qmake");
	if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].findIndex("qmake_all") == -1)
	    project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");
	return; /* subdirs is done */
    }

    if( project->variables()["QMAKE_EXTENSION_SHLIB"].isEmpty() )
	project->variables()["QMAKE_EXTENSION_SHLIB"].append( "so" );
    if( project->isEmpty("QMAKE_COPY_FILE") )
	project->variables()["QMAKE_COPY_FILE"].append( "$(COPY) -p" );
    if( project->isEmpty("QMAKE_COPY_DIR") )
	project->variables()["QMAKE_COPY_DIR"].append( "$(COPY) -pR" );
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];


    bool extern_libs = !project->variables()["QMAKE_APP_FLAG"].isEmpty() ||
		       (!project->variables()["QMAKE_LIB_FLAG"].isEmpty() && project->isActiveConfig("dll")) ||
                       (project->first("TARGET") == "qt" ||
			project->first("TARGET") == "qte" ||
			project->first("TARGET") == "qt-mt");
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if ( (!project->variables()["QMAKE_LIB_FLAG"].isEmpty() && !project->isActiveConfig("staticlib") ) ||
	 (project->isActiveConfig("qt") &&  project->isActiveConfig( "plugin" ) )) {
	if(configs.findIndex("dll") == -1) configs.append("dll");
    } else if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() || project->isActiveConfig("dll") ) {
	configs.remove("staticlib");
    }
    if ( project->isActiveConfig("warn_off") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_OFF"];
    } else if ( project->isActiveConfig("warn_on") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_ON"];
    }
    if ( project->isActiveConfig("debug") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_DEBUG"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_DEBUG"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_DEBUG"];
    } else if ( project->isActiveConfig("release") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RELEASE"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RELEASE"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_RELEASE"];
    }
    if ( !project->variables()["QMAKE_INCDIR"].isEmpty() ) 
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if ( !project->variables()["QMAKE_LIBDIR"].isEmpty() ) 
	project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" + project->first("QMAKE_LIBDIR"));
    if ( extern_libs && (project->isActiveConfig("qt") || project->isActiveConfig("opengl")) ) {
	if(configs.findIndex("x11lib") == -1) 
	    configs.append("x11lib");
	if ( project->isActiveConfig("opengl") && configs.findIndex("x11inc") == -1 )
	    configs.append("x11inc");
    }
    if ( project->isActiveConfig("x11") ) {
	if(configs.findIndex("x11lib") == -1) 
	    configs.append("x11lib");
	if(configs.findIndex("x11inc") == -1) 
	    configs.append("x11inc");
    }
    if ( project->isActiveConfig("accessibility" ) ) 
	project->variables()["DEFINES"].append("QT_ACCESSIBILITY_SUPPORT");
    if ( project->isActiveConfig("tablet") )
	project->variables()["DEFINES"].append("QT_TABLET_SUPPORT");
    if ( project->isActiveConfig("qt") ) {
	if(configs.findIndex("moc")) configs.append("moc");
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_QT"];
	if ( !project->isActiveConfig("debug") ) {
	    project->variables()["DEFINES"].append("QT_NO_DEBUG");
	}
	if ( !( (project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
		(project->first("TARGET") == "qt-mt") ) ) {
	    if(!project->variables()["QMAKE_LIBDIR_QT"].isEmpty()) {
		project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" +
								  project->first("QMAKE_LIBDIR_QT"));
	    }
	    if (project->isActiveConfig("thread") && !project->variables()["QMAKE_LIBS_QT_THREAD"].isEmpty()) 
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    else 
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	}
    }
    if ( project->isActiveConfig("thread") ) {
	project->variables()["DEFINES"].append("QT_THREAD_SUPPORT");
	if ( !project->variables()["QMAKE_CFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_THREAD"];
	if( !project->variables()["QMAKE_CXXFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_THREAD"];
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_THREAD"];
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_THREAD"];
	if(!project->variables()["QMAKE_LFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_THREAD"];
    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_OPENGL"];
	if(!project->variables()["QMAKE_LIBDIR_OPENGL"].isEmpty()) {
	    project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" + project->first("QMAKE_LIBDIR_OPENGL"));
	}
	if ( (project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
	     (project->first("TARGET") == "qt-mt") )
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL_QT"];
	else 
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
    }
    if ( project->isActiveConfig("x11sm") )
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_X11SM"];
    if ( project->isActiveConfig("dylib") )
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_DYNLOAD"];
    if ( project->isActiveConfig("x11inc") )
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_X11"];
    if ( project->isActiveConfig("x11lib") ) {
	if(!project->variables()["QMAKE_LIBDIR_X11"].isEmpty()) 
	    project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" + project->first("QMAKE_LIBDIR_X11"));
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_X11"];
    }
    if ( project->isActiveConfig("moc") )
	setMocAware(TRUE);
    if ( project->variables()["QMAKE_RUN_CC"].isEmpty() )
	project->variables()["QMAKE_RUN_CC"].append("$(CC) -c $(CFLAGS) $(INCPATH) -o $obj $src");
    if ( project->variables()["QMAKE_RUN_CC_IMP"].isEmpty() )
	project->variables()["QMAKE_RUN_CC_IMP"].append("$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<");
    if ( project->variables()["QMAKE_RUN_CXX"].isEmpty() )
	project->variables()["QMAKE_RUN_CXX"].append("$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $obj $src");
    if ( project->variables()["QMAKE_RUN_CXX_IMP"].isEmpty() )
	project->variables()["QMAKE_RUN_CXX_IMP"].append("$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<");
    project->variables()["QMAKE_FILETAGS"] += QStringList::split("HEADERS SOURCES TARGET DESTDIR", " ");
    if ( !project->variables()["PRECOMPH"].isEmpty() ) {
	// Need to fix MOC_DIR since we do this before init()
	QString allmoc = project->first("MOC_DIR");
	if(allmoc.right(Option::dir_sep.length()) != Option::dir_sep)
	    allmoc += Option::dir_sep;
	if(QDir::isRelativePath(allmoc) && !project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty())
	    allmoc.prepend(Option::output_dir + Option::dir_sep);
	allmoc += "/allmoc.cpp";
	project->variables()["SOURCES"].prepend(allmoc);
	project->variables()["HEADERS_ORIG"] = project->variables()["HEADERS"];
	project->variables()["HEADERS"].clear();
    }
    if( project->isActiveConfig("GNUmake") && !project->isEmpty("QMAKE_CFLAGS_DEPS"))
	include_deps = TRUE; //do not generate deps

    MakefileGenerator::init();
    if ( project->isActiveConfig("resource_fork") && !project->isActiveConfig("console")) {
	if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	    if(project->isEmpty("DESTDIR"))
		project->values("DESTDIR").append("");
	    project->variables()["DESTDIR"].first() += project->variables()["TARGET"].first() + ".app/Contents/MacOS/";

	    project->variables()["QMAKE_PKGINFO"].append(project->first("DESTDIR") + "../PkgInfo");
	    project->variables()["ALL_DEPS"] += project->first("QMAKE_PKGINFO");
	}
    }
    //version handling
    if ( project->variables()["VER_PAT"].isEmpty() )
	project->variables()["VER_PAT"].append( "0" );
    if(project->variables()["VERSION"].isEmpty())
	project->variables()["VERSION"].append("1.0." + project->first("VER_PAT") );
    QStringList l = QStringList::split('.', project->first("VERSION")) << "0" << "0"; //make sure there are three
    project->variables()["VER_MAJ"].append(l[0]);
    project->variables()["VER_MIN"].append(l[1]);
    project->variables()["VER_PAT"].append(l[2]);

    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
#if 0
	if ( project->isActiveConfig("dll") ) {
	    project->variables()["TARGET"] += project->variables()["TARGET.so"];
	    if(project->variables()["QMAKE_LFLAGS_SHAPP"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SHAPP"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET");
	}
#endif
	project->variables()["TARGET"].first().prepend(project->first("DESTDIR"));
    } else if ( project->isActiveConfig("staticlib") ) {
	project->variables()["TARGET"].first().prepend(project->first("DESTDIR") + "lib");
	project->variables()["TARGET"].first() += ".a";
	if(project->variables()["QMAKE_AR_CMD"].isEmpty())
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)");
    } else {
	project->variables()["TARGETA"].append(project->first("DESTDIR") + "lib" + project->first("TARGET") + ".a");
	if ( !project->variables()["QMAKE_AR_CMD"].isEmpty() )
	    project->variables()["QMAKE_AR_CMD"].first().replace(QRegExp("\\(TARGET\\)"),"(TARGETA)");
	else
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)");
	if( project->isActiveConfig("plugin") ) {
	    project->variables()["TARGET_x.y.z"].append("lib" +
							project->first("TARGET") + "." + project->first("QMAKE_EXTENSION_SHLIB"));
	    project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
						    project->first("QMAKE_EXTENSION_SHLIB") +
						    "." + project->first("VER_MAJ"));
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	    project->variables()["DEFINES"].append("QT_PLUGIN");
	} else if ( !project->variables()["QMAKE_HPUX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".sl");
	    project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." + project->first("VER_MAJ"));
	    project->variables()["TARGET"] = project->variables()["TARGET_x"];
	} else if ( !project->variables()["QMAKE_AIX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".a");
	    project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
						    project->first("QMAKE_EXTENSION_SHLIB") +
						    "." + project->first("VER_MAJ"));
	    project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
						      project->first("QMAKE_EXTENSION_SHLIB")
						      + "." + project->first("VER_MAJ") +
						      "." + project->first("VER_MIN"));
	    project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
							project->first("QMAKE_EXTENSION_SHLIB") + "." +
							project->first("VER_MAJ") + "." +
							project->first("VER_MIN") + "." +
							project->first("VER_PAT"));
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	} else {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + "." +
						   project->first("QMAKE_EXTENSION_SHLIB"));
	    project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
						    project->first("QMAKE_EXTENSION_SHLIB") +
						    "." + project->first("VER_MAJ"));
	    project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
						      project->first("QMAKE_EXTENSION_SHLIB")
						      + "." + project->first("VER_MAJ") +
						      "." + project->first("VER_MIN"));
	    project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") +
							"." +
							project->variables()[
							    "QMAKE_EXTENSION_SHLIB"].first() + "." +
							project->first("VER_MAJ") + "." +
							project->first("VER_MIN") +  "." +
							project->first("VER_PAT"));
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	}
	project->variables()["QMAKE_LN_SHLIB"].append("-ln -s");
	project->variables()["DESTDIR_TARGET"].append("$(TARGET)");
	if ( !project->variables()["DESTDIR"].isEmpty() )
	    project->variables()["DESTDIR_TARGET"].first().prepend(project->first("DESTDIR"));
	if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty() && !project->variables()["TARGET_x"].isEmpty())
	    project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET_x");
	if(project->variables()["QMAKE_LINK_SHLIB_CMD"].isEmpty())
	    project->variables()["QMAKE_LINK_SHLIB_CMD"].append(
		"$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)");
    }
    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHAPP"];
    } else if ( project->isActiveConfig("dll") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_SHLIB"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_SHLIB"];
	if( project->isActiveConfig("plugin") ) {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PLUGIN"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	} else {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	}
	QString destdir = project->first("DESTDIR");
	if(!destdir.isEmpty() && !project->variables()["QMAKE_RPATH"].isEmpty())
	    project->variables()["QMAKE_LFLAGS"] += project->first("QMAKE_RPATH") + destdir;
    }
}

QString
UnixMakefileGenerator::defaultInstall(const QString &t)
{
    QString ret, destdir=project->first("DESTDIR");
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
	destdir += Option::dir_sep;
    QString target="$(TARGET)";
    QStringList links;
    if(t == "target") {
	if(project->first("TEMPLATE") == "app") {

	} else if(!project->isActiveConfig("staticlib")) {
	    if(project->isActiveConfig("plugin")) {

	    } else if ( !project->variables()["QMAKE_HPUX_SHLIB"].isEmpty() ) {
		links << "$(TARGET0)";
	    }else
		links << "$(TARGET0)" << "$(TARGET1)" << "$(TARGET2)";
	} else {
	    target = "$(TARGETA)";
	}
    }
    QString targetdir = Option::fixPathToTargetOS(project->first("target.path"), FALSE);
    if(targetdir.right(1) != Option::dir_sep)
	targetdir += Option::dir_sep;
    QString dst_targ = Option::fixPathToTargetOS(targetdir + target, FALSE);

    QString src_targ = target;
    if(!destdir.isEmpty())
	src_targ = Option::fixPathToTargetOS(destdir + target, FALSE);


    ret = QString("$(COPY) ") + src_targ + " " + dst_targ;
    if(!links.isEmpty()) {
	for(QStringList::Iterator it = links.begin(); it != links.end(); it++) {
	    if(Option::target_mode == Option::TARG_WIN_MODE || Option::target_mode == Option::TARG_MAC9_MODE) {
	    } else if(Option::target_mode == Option::TARG_UNIX_MODE || Option::target_mode == Option::TARG_MACX_MODE) {
		QString link = Option::fixPathToTargetOS(destdir + (*it), FALSE);
		int lslash = link.findRev(Option::dir_sep);
		if(lslash != -1)
		    link = link.right(link.length() - (lslash + 1));
		ret += "\n\tln -sf " + dst_targ + " " + Option::fixPathToTargetOS(targetdir + link, FALSE);
	    }
	}
    }
    return ret;
}
