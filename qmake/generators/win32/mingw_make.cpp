/****************************************************************************
** $Id$
**
** Implementation of MingwMakefileGenerator class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of qmake.
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

#include "mingw_make.h"
#include "option.h"
#include <qregexp.h>
#include <qdir.h>
#include <stdlib.h>
#include <time.h>


MingwMakefileGenerator::MingwMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(false)
{
    Option::obj_ext = ".o";
}

bool MingwMakefileGenerator::findLibraries() // todo - pascal
{
    return true;
}

bool MingwMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        t << "all clean:" << "\n\t"
          << "@echo \"Some of the required modules ("
          << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
          << "@echo \"Skipped.\"" << endl << endl;
        writeMakeQmake(t);
        return true;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
        writeMingwParts(t);
        return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
 }

void createLdObjectScriptFile(const QString &fileName, QStringList &objList)
{
    QString filePath = Option::output_dir + QDir::separator() + fileName;
    QFile file(filePath);
    if(file.open(IO_WriteOnly | IO_Translate)) {
        QTextStream t(&file);
        t << "INPUT(" << endl;
        for (QStringList::Iterator it = objList.begin(); it != objList.end(); ++it) {
            t << *it << endl;
        }
        t << ");" << endl;
        file.close();
    }
}

void MingwMakefileGenerator::writeMingwParts(QTextStream &t)
{
    writeStandardParts(t);
}

void MingwMakefileGenerator::init()
{
    if(init_flag)
        return;
    init_flag = true;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
        MakefileGenerator::init();
        if(project->variables()["MAKEFILE"].isEmpty())
            project->variables()["MAKEFILE"].append("Makefile");
        if(project->variables()["QMAKE_QMAKE"].isEmpty())
            project->variables()["QMAKE_QMAKE"].append("qmake");
        return;
    }

    processVars();

    // LIBS defined in Profile comes first for gcc
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];

    QString targetfilename = project->variables()["TARGET"].first();
    QStringList &configs = project->variables()["CONFIG"];

    if(project->isActiveConfig("qt_dll"))
        if(configs.indexOf("qt") == -1)
            configs.append("qt");

    if(project->isActiveConfig("dll")) {
        QString destDir = "";
        if(!project->first("DESTDIR").isEmpty())
            destDir = project->first("DESTDIR") + Option::dir_sep;
        project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,--out-implib,") +
                                                    destDir + "lib" + project->first("TARGET") + ".a");
    }

    if(!project->variables()["DEF_FILE"].isEmpty())
        project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,") + project->first("DEF_FILE"));

    MakefileGenerator::init();
    if(project->isActiveConfig("dll")) {
        project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") +"lib" + project->first("TARGET") + ".a");
    }
}

void MingwMakefileGenerator::processLibsVar()
{
    QStringList &libs = project->variables()["QMAKE_LIBS"];
    for (QStringList::Iterator libit = libs.begin(); libit != libs.end();) {
        if((*libit).startsWith("-L")) {
            project->variables()["QMAKE_LIBDIR"] += (*libit).mid(2);
            libit = libs.erase(libit);
        } else {
            ++libit;
        }
    }
}

void MingwMakefileGenerator::fixTargetExt()
{
    if (!project->isActiveConfig("dll") && project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        project->variables()["TARGET_EXT"].append(".a");
        project->variables()["QMAKE_LFLAGS"].append("-static");
        if(project->variables()["TARGET"].value(0).left(3) != "lib")
            project->variables()["TARGET"].value(0).prepend("lib");
    } else {
        Win32MakefileGenerator::fixTargetExt();
    }
}

void MingwMakefileGenerator::writeLibsPart(QTextStream &t)
{
   if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
        t << "LINK        =        " << var("QMAKE_LINK") << endl;
        t << "LFLAGS        =        " << var("QMAKE_LFLAGS") << endl;
        t << "LIBS        =        ";
        if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
            writeLibDirPart(t);
        t << var("QMAKE_LIBS").replace(QRegExp("(\\slib|^lib)")," -l") << endl;
    }
    else {
        t << "LIB        =        " << var("QMAKE_LIB") << endl;
    }
}

void MingwMakefileGenerator::processQtConfig()
{
    if (project->isActiveConfig("qt")) {
        if (!(project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty())) {
            if (!project->variables()["QMAKE_QT_DLL"].isEmpty()) {
                int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
                if(hver != -1) {
                    QString ver;
                    ver.sprintf("libqt" QTDLL_POSTFIX "%d.a", hver);
                    QStringList &libs = project->variables()["QMAKE_LIBS"];
                    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
                        (*libit).replace(QRegExp("qt\\.lib"), ver);
                }
            }
            if(!project->isActiveConfig("dll") && !project->isActiveConfig("plugin")) {
                project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
            }

            // QMAKE_LIBS_QT_ENTRY should be first on the link line as it needs qt
            project->variables()["QMAKE_LIBS"].removeAll(project->variables()["QMAKE_LIBS_QT_ENTRY"].first());
            project->variables()["QMAKE_LIBS"].prepend(project->variables()["QMAKE_LIBS_QT_ENTRY"].first());
        }
    }
}

void MingwMakefileGenerator::writeObjectsPart(QTextStream &t)
{
    if(project->variables()["OBJECTS"].count() > var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        createLdObjectScriptFile(var("QMAKE_LINK_OBJECT_SCRIPT"), project->variables()["OBJECTS"]);
        objectsLinkLine = var("QMAKE_LINK_OBJECT_SCRIPT");
    } else {
        objectsLinkLine = "$(OBJECTS)";
    }
    Win32MakefileGenerator::writeObjectsPart(t);
}

void MingwMakefileGenerator::writeObjMocPart(QTextStream &t)
{
    if(project->variables()["OBJMOC"].count() > var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        createLdObjectScriptFile(var("QMAKE_LINK_OBJMOC_SCRIPT"), project->variables()["OBJMOC"]);
        objmocLinkLine = var("QMAKE_LINK_OBJMOC_SCRIPT");
    } else {
        objmocLinkLine = "$(OBJMOC)";
    }
    Win32MakefileGenerator::writeObjMocPart(t);
}

void MingwMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "all: " << fileFixify(Option::output.name()) << " " << varGlue("ALL_DEPS"," "," "," ") << " $(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) $(OBJMOC) " << var("POST_TARGETDEPS");
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
        t << "\n\t" << "$(LINK) $(LFLAGS) -o $(TARGET) " << objectsLinkLine << " " << objmocLinkLine << " $(LIBS)";
    } else {
        t << "\n\t" << "$(LIB) $(TARGET) " << objectsLinkLine << " " << objmocLinkLine;
    }
}

void MingwMakefileGenerator::writeRcFilePart(QTextStream &t)
{
    if(!project->variables()["RC_FILE"].isEmpty()) {
        t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
          << var("QMAKE_RC") << " -i " << var("RC_FILE") << " -o " << var("RC_FILE").replace(QRegExp("\\.rc"),".o") << " --include-dir=" << QFileInfo(var("RC_FILE")).dirPath() << endl << endl;
    }
    // Somewhat useless call I think...
    project->variables()["RES_FILE"].value(0).replace(QRegExp("\\.rc"),".o");
}
