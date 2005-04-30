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

#include "msvc_nmake.h"
#include "option.h"
#include <qregexp.h>
#include <qhash.h>
#include <qdir.h>
#include <time.h>

NmakeMakefileGenerator::NmakeMakefileGenerator() : Win32MakefileGenerator(), init_flag(false)
{

}

bool
NmakeMakefileGenerator::writeMakefile(QTextStream &t)
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
        writeMakeQmake(t);
        return true;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
        writeNmakeParts(t);
        return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
}

QStringList &NmakeMakefileGenerator::findDependencies(const QString &file)
{
    QStringList &aList = MakefileGenerator::findDependencies(file);
    // Note: The QMAKE_IMAGE_COLLECTION file have all images
    // as dependency, so don't add precompiled header then
    if (file == project->first("QMAKE_IMAGE_COLLECTION"))
        return aList;
    for(QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
        if(file.endsWith(*it)) {
            if(!aList.contains(precompH))
                aList += precompH;
            break;
        }
    }
    return aList;
}

void NmakeMakefileGenerator::writeNmakeParts(QTextStream &t)
{
    writeStandardParts(t);

    // precompiled header
    if(usePCH) {
        QString precompRule = QString("-c -Yc -Fp%1 -Fo%2").arg(precompPch).arg(precompObj);
        t << precompObj << ": " << precompH << " " << findDependencies(precompH).join(" \\\n\t\t")
          << "\n\t" << "$(CXX) " + precompRule +" $(CXXFLAGS) $(INCPATH) -TP " << precompH << endl << endl;
    }
}

QString NmakeMakefileGenerator::var(const QString &value)
{
    if (usePCH) {
        if ((value == "QMAKE_RUN_CXX_IMP_BATCH"
            || value == "QMAKE_RUN_CXX_IMP"
            || value == "QMAKE_RUN_CXX")) {
            QFileInfo precompHInfo(fileInfo(precompH));
            QString precompRule = QString("-c -FI%1 -Yu%2 -Fp%3")
                .arg(precompHInfo.fileName())
                .arg(precompHInfo.fileName())
                .arg(precompPch);
            QString p = MakefileGenerator::var(value);
            p.replace("-c", precompRule);
            // Cannot use -Gm with -FI & -Yu, as this gives an
            // internal compiler error, on the newer compilers
            p.remove("-Gm");
            return p;
        } else if (value == "QMAKE_CXXFLAGS") {
            // Remove internal compiler error option
            return MakefileGenerator::var(value).remove("-Gm");
        }
    }

    // Normal val
    return MakefileGenerator::var(value);
}

void NmakeMakefileGenerator::init()
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

    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    processVars();

    if (!project->variables()["RES_FILE"].isEmpty()) {
        project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];
    }

    if(!project->variables()["DEF_FILE"].isEmpty())
        project->variables()["QMAKE_LFLAGS"].append(QString("/DEF:") + project->first("DEF_FILE"));

    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"][0];
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(".", "");
        project->variables()["QMAKE_LFLAGS"].append("/VERSION:" + major + "." + minor);
    }

    // Base class init!
    MakefileGenerator::init();

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    usePCH = !precompH.isEmpty() && project->isActiveConfig("precompile_header");
    if (usePCH) {
        // Created files
        precompObj = var("OBJECTS_DIR") + project->first("TARGET") + "_pch" + Option::obj_ext;
        precompPch = var("OBJECTS_DIR") + project->first("TARGET") + "_pch.pch";
        // Add linking of precompObj (required for whole precompiled classes)
        project->variables()["OBJECTS"]                  += precompObj;
        // Add pch file to cleanup
        project->variables()["QMAKE_CLEAN"]          += precompPch;
        // Return to variable pool
        project->variables()["PRECOMPILED_OBJECT"] = QStringList(precompObj);
        project->variables()["PRECOMPILED_PCH"]    = QStringList(precompPch);
    }

    QString version = project->first("TARGET_VERSION_EXT");
    if(project->isActiveConfig("dll")) {
        project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".exp");
    }
    if(project->isActiveConfig("debug")) {
        project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".pdb");
        project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".ilk");
        project->variables()["QMAKE_CLEAN"].append("vc*.pdb");
        project->variables()["QMAKE_CLEAN"].append("vc*.idb");
    }
}

void NmakeMakefileGenerator::writeLibDirPart(QTextStream &t)
{
    t << varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\"") << " ";
}

void NmakeMakefileGenerator::writeImplicitRulesPart(QTextStream &t)
{
    t << ".SUFFIXES: .c";
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
        t << " " << (*cppit);
    t << endl << endl;

    if(!project->isActiveConfig("no_batch")) {
        // Batchmode doesn't use the non implicit rules QMAKE_RUN_CXX & QMAKE_RUN_CC
        project->variables().remove("QMAKE_RUN_CXX");
        project->variables().remove("QMAKE_RUN_CC");

        QHash<QString, void*> source_directories;
        source_directories.insertMulti(".", (void*)1);
        QString directories[] = { QString("UI_SOURCES_DIR"), QString("UI_DIR"), QString() };
        for(int y = 0; !directories[y].isNull(); y++) {
            QString dirTemp = project->first(directories[y]);
            if (dirTemp.endsWith("\\"))
                dirTemp.truncate(dirTemp.length()-1);
            if(!dirTemp.isEmpty())
                source_directories.insertMulti(dirTemp, (void*)1);
        }
        QString srcs[] = { QString("SOURCES"), QString("GENERATED_SOURCES"), QString() };
        for(int x = 0; !srcs[x].isNull(); x++) {
            QStringList &l = project->variables()[srcs[x]];
            for(QStringList::Iterator sit = l.begin(); sit != l.end(); ++sit) {
                QString sep = "\\";
                if((*sit).indexOf(sep) == -1)
                    sep = "/";
                QString dir = (*sit).section(sep, 0, -2);
                if(!dir.isEmpty() && !source_directories[dir])
                    source_directories.insertMulti(dir, (void*)1);
            }
        }

        for(QHash<QString, void*>::Iterator it(source_directories.begin()); it != source_directories.end(); ++it) {
            if(it.key().isEmpty())
                continue;
            QString objDir = var("OBJECTS_DIR");
            if (objDir == ".\\")
                objDir = "";
            for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
                t << "{" << it.key() << "}" << (*cppit) << "{" << objDir << "}" << Option::obj_ext << "::\n\t"
                  << var("QMAKE_RUN_CXX_IMP_BATCH").replace(QRegExp("\\$@"), var("OBJECTS_DIR")) << endl << "\t$<" << endl << "<<" << endl << endl;
            t << "{" << it.key() << "}" << ".c{" << var("OBJECTS_DIR") << "}" << Option::obj_ext << "::\n\t"
              << var("QMAKE_RUN_CC_IMP_BATCH").replace(QRegExp("\\$@"), var("OBJECTS_DIR")) << endl << "\t$<" << endl << "<<" << endl << endl;
        }
    } else {
        for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
            t << (*cppit) << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
        t << ".c" << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;
    }

}

void NmakeMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "first: all" << endl;
    t << "all: " << fileFixify(Option::output.fileName()) << " " << varGlue("ALL_DEPS"," "," "," ") << "$(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) " << var("POST_TARGETDEPS");
    if(project->isActiveConfig("staticlib")) {
        t << "\n\t" << "$(LIB) /OUT:\"$(TARGET)\" @<<" << "\n\t  "
          << "$(OBJECTS)";
    } else {
        t << "\n\t" << "$(LINK) $(LFLAGS) /OUT:\"$(TARGET)\" @<< " << "\n\t  "
          << "$(OBJECTS) $(LIBS)";
    }
    t << endl << "<<" << endl;
}
