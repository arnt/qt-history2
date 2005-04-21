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
    Option::res_ext = ".o";
}

bool MingwMakefileGenerator::findLibraries()
{
    QStringList &l = project->variables()["QMAKE_LIBS"];

    QList<QMakeLocalFileName> dirs;
    {
        QStringList &libpaths = project->variables()["QMAKE_LIBDIR"];
        for(QStringList::Iterator libpathit = libpaths.begin();
            libpathit != libpaths.end(); ++libpathit)
            dirs.append(QMakeLocalFileName((*libpathit)));
    }
    
    QStringList::Iterator it = l.begin();
    while (it != l.end()) {
        if ((*it).startsWith("-l")) {
            QString steam = (*it).mid(2);
            QString suffix;
            if (!project->isEmpty("QMAKE_" + steam.toUpper() + "_SUFFIX"))
                suffix = project->first("QMAKE_" + steam.toUpper() + "_SUFFIX");
            QString extension;
            for (QList<QMakeLocalFileName>::Iterator dir_it = dirs.begin(); dir_it != dirs.end(); ++dir_it) {
                int ver = findHighestVersion((*dir_it).local(), steam);
                if (ver != -1) {
                    extension += QString::number(ver);
                    break;
                }
            }
	        extension += suffix;
	        (*it) += extension;
	    }
        ++it;
    }
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
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream t(&file);
        t << "INPUT(" << endl;
        for (QStringList::Iterator it = objList.begin(); it != objList.end(); ++it) {
            if (QDir::isRelativePath(*it))
		t << "./" << *it << endl;
	    else
		t << *it << endl;
        }
        t << ");" << endl;
	t.flush();
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

    project->variables()["TARGET_PRL"].append(project->first("TARGET"));

    processVars();

    if (!project->variables()["RES_FILE"].isEmpty()) {
        project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];
    }

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
            destDir = Option::fixPathToTargetOS(project->first("DESTDIR") + Option::dir_sep, false, false);
        project->variables()["MINGW_IMPORT_LIB"].prepend(destDir + "lib" + project->first("TARGET")
                                                         + project->first("TARGET_VERSION_EXT") + ".a");
	project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,--out-implib,") + project->first("MINGW_IMPORT_LIB"));
    }

    if(!project->variables()["DEF_FILE"].isEmpty())
        project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,") + project->first("DEF_FILE"));

    MakefileGenerator::init();
    if(project->isActiveConfig("dll")) {
        project->variables()["QMAKE_CLEAN"].append(project->first("MINGW_IMPORT_LIB"));
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
    if(project->isActiveConfig("staticlib")) {
        t << "LIB        =        " << var("QMAKE_LIB") << endl;
    } else {
        t << "LINK        =        " << var("QMAKE_LINK") << endl;
        t << "LFLAGS        =        " << var("QMAKE_LFLAGS") << endl;
        t << "LIBS        =        ";
        if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
            writeLibDirPart(t);
        t << var("QMAKE_LIBS").replace(QRegExp("(\\slib|^lib)")," -l") << endl;
    }
}

void MingwMakefileGenerator::writeObjectsPart(QTextStream &t)
{
    if(project->isActiveConfig("staticlib")
        || project->variables()["OBJECTS"].count() < var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        objectsLinkLine = "$(OBJECTS)";
    } else {
        QString ld_script_file = var("QMAKE_LINK_OBJECT_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ld_script_file += "." + var("BUILD_NAME");
	}
	createLdObjectScriptFile(ld_script_file, project->variables()["OBJECTS"]);
        objectsLinkLine = ld_script_file;
    }
    Win32MakefileGenerator::writeObjectsPart(t);
}

void MingwMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "first: all" << endl;
    t << "all: " << fileFixify(Option::output.fileName()) << " " << varGlue("ALL_DEPS"," "," "," ") << " $(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) " << var("POST_TARGETDEPS");
    if(project->isActiveConfig("staticlib")) {
        t << "\n\t" << "$(LIB) \"$(TARGET)\" " << objectsLinkLine << " " ;
    } else {
        t << "\n\t" << "$(LINK) $(LFLAGS) -o \"$(TARGET)\" " << objectsLinkLine << " " << " $(LIBS)";
    }
}

void MingwMakefileGenerator::writeRcFilePart(QTextStream &t)
{
    if (!project->variables()["RC_FILE"].isEmpty()) {
        t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
          << var("QMAKE_RC") << " -i " << var("RC_FILE") << " -o " << var("RES_FILE") << " --include-dir="
          << fileInfo(var("RC_FILE")).path() << endl << endl;
    }
}
