/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "borland_bmake.h"
#include "option.h"
#include <qdir.h>
#include <qregexp.h>
#include <time.h>


BorlandMakefileGenerator::BorlandMakefileGenerator() : Win32MakefileGenerator(), init_flag(false)
{

}

bool
BorlandMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
        for(QStringList::ConstIterator it = qut.begin(); it != qut.end(); ++it)
            t << *it << " ";
        t << "all first clean:" << "\n\t"
          << "@echo \"Some of the required modules ("
          << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
          << "@echo \"Skipped.\"" << endl << endl;
        return true;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
        writeBorlandParts(t);
        return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
}

void
BorlandMakefileGenerator::writeBorlandParts(QTextStream &t)
{
    t << "!if !$d(BCB)" << endl;
    t << "BCB = $(MAKEDIR)\\.." << endl;
    t << "!endif" << endl << endl;

    writeStandardParts(t);
}

void
BorlandMakefileGenerator::init()
{
    if(init_flag)
        return;
    init_flag = true;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if (project->first("TEMPLATE") == "app") {
        project->variables()["QMAKE_APP_FLAG"].append("1");
    } else if(project->first("TEMPLATE") == "lib"){
        project->variables()["QMAKE_LIB_FLAG"].append("1");
    } else if(project->first("TEMPLATE") == "subdirs") {
        MakefileGenerator::init();
        if(project->variables()["MAKEFILE"].isEmpty())
            project->variables()["MAKEFILE"].append("Makefile");
        if(project->variables()["QMAKE_QMAKE"].isEmpty())
            project->variables()["QMAKE_QMAKE"].append("qmake");
        return;
    }

    processVars();

    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];

    MakefileGenerator::init();

    if (project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        // bcc does not generate a .tds file for static libs
        QString tdsPostfix;
        if (!project->variables()["VERSION"].isEmpty())
            tdsPostfix = project->first("TARGET_VERSION_EXT");
        tdsPostfix += ".tds";
        project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + tdsPostfix);
    }
}

void BorlandMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "first: all" << endl;
    t << "all: " << fileFixify(Option::output.fileName()) << " " << varGlue("ALL_DEPS"," "," "," ") << " $(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) " << var("POST_TARGETDEPS");
    if(project->isActiveConfig("staticlib")) {
        t << "\n\t-$(DEL_FILE) $(TARGET)"
	      << "\n\t" << "$(LIB) $(TARGET) @&&|" << " \n+"
	      << project->variables()["OBJECTS"].join(" \\\n+") << " \\\n+"
	      << project->variables()["OBJMOC"].join(" \\\n+");
    } else {
        t << "\n\t" << "$(LINK) @&&|" << "\n\t"
	      << "$(LFLAGS) $(OBJECTS) $(OBJMOC),$(TARGET),,$(LIBS),$(DEF_FILE),$(RES_FILE)";
    }
    t << endl << "|" << endl;
}

void BorlandMakefileGenerator::writeCleanParts(QTextStream &t)
{
    t << "clean: "
      << varGlue("OBJECTS","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n")
      << varGlue("CLEAN_FILES","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n");

    if(!project->isEmpty("IMAGES"))
        t << varGlue("QMAKE_IMAGE_COLLECTION", "\n\t-$(DEL_FILE) ", "\n\t-$(DEL_FILE) ", "");
    t << endl;

    t << "distclean: clean"
      << "\n\t-$(DEL_FILE) $(TARGET)"
      << endl << endl;
}
