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

#include "mingw_make.h"
#include "option.h"
#include <qregexp.h>
#include <qdir.h>
#include <stdlib.h>
#include <time.h>


MingwMakefileGenerator::MingwMakefileGenerator() : Win32MakefileGenerator(), init_flag(false)
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
    if (file.open(QIODevice::WriteOnly | QIODevice::Translate)) {
        QTextStream t(&file);
        t << "INPUT(" << endl;
        for (QStringList::Iterator it = objList.begin(); it != objList.end(); ++it) {
            if (QDir::isRelativePath(*it))
		t << "./" << *it << endl;
	    else
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

void MingwMakefileGenerator::fixTargetExt()
{
    if (project->isActiveConfig("staticlib")) {
        project->variables()["TARGET_EXT"].append(".a");
        project->variables()["QMAKE_LFLAGS"].append("-static");
        project->variables()["TARGET"].first() =  "lib" + project->first("TARGET");
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
        }
    }
}

void MingwMakefileGenerator::writeObjectsPart(QTextStream &t)
{
    if(project->variables()["OBJECTS"].count() > var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        QString ld_script_file = var("QMAKE_LINK_OBJECT_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ld_script_file += "." + var("BUILD_NAME");
	}
	createLdObjectScriptFile(ld_script_file, project->variables()["OBJECTS"]);
        objectsLinkLine = ld_script_file;
    } else {
        objectsLinkLine = "$(OBJECTS)";
    }
    Win32MakefileGenerator::writeObjectsPart(t);
}

void MingwMakefileGenerator::writeObjMocPart(QTextStream &t)
{
    if(project->variables()["OBJMOC"].count() > var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        QString ld_script_file = var("QMAKE_LINK_OBJMOC_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ld_script_file += "." + var("BUILD_NAME");
	}
	createLdObjectScriptFile(ld_script_file, project->variables()["OBJMOC"]);
        objmocLinkLine = ld_script_file;
    } else {
        objmocLinkLine = "$(OBJMOC)";
    }
    Win32MakefileGenerator::writeObjMocPart(t);
}

void MingwMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "first: all" << endl;
    t << "all: " << fileFixify(Option::output.fileName()) << " " << varGlue("ALL_DEPS"," "," "," ") << " $(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) $(OBJMOC) " << var("POST_TARGETDEPS");
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
        t << "\n\t" << "$(LINK) $(LFLAGS) -o $(TARGET) " << objectsLinkLine << " " << objmocLinkLine << " $(LIBS)";
    } else {
        t << "\n\t" << "$(LIB) $(TARGET) " << objectsLinkLine << " " << objmocLinkLine;
    }
}

void MingwMakefileGenerator::writeRcFilePart(QTextStream &t)
{
    project->variables()["RES_FILE"].value(0).replace(QRegExp("\\.rc"), Option::obj_ext);
    
    if (!project->variables()["RC_FILE"].isEmpty()) {
        t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
          << var("QMAKE_RC") << " -i " << var("RC_FILE") << " -o " << var("RES_FILE") << " --include-dir=" << QFileInfo(var("RC_FILE")).path() << endl << endl;
    }
}
