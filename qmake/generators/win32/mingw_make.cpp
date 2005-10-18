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

void createLdObjectScriptFile(const QString &fileName, const QStringList &objList)
{
    QString filePath = Option::output_dir + QDir::separator() + fileName;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream t(&file);
        t << "INPUT(" << endl;
        for (QStringList::ConstIterator it = objList.constBegin(); it != objList.constEnd(); ++it) {
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

void createArObjectScriptFile(const QString &fileName, const QString &target, const QStringList &objList)
{
    QString filePath = Option::output_dir + QDir::separator() + fileName;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream t(&file);
        t << "CREATE " << target << endl;
        for (QStringList::ConstIterator it = objList.constBegin(); it != objList.constEnd(); ++it) {
            if (QDir::isRelativePath(*it))
		t << "ADDMOD " << *it << endl;
	    else
		t << *it << endl;
        }
        t << "SAVE" << endl;
	t.flush();
        file.close();
    }
}

void MingwMakefileGenerator::writeMingwParts(QTextStream &t)
{
    writeStandardParts(t);

    if (!preCompHeaderOut.isEmpty()) {
	QString header = project->first("PRECOMPILED_HEADER");
	QString cHeader = preCompHeaderOut + Option::dir_sep + "c";
	t << cHeader << ": " << header << " " 
          << findDependencies(header).join(" \\\n\t\t")
	  << "\n\t" << mkdir_p_asstring(preCompHeaderOut)
	  << "\n\t" << "$(CC) -x c-header -c $(CFLAGS) $(INCPATH) -o " << cHeader << " " << header
          << endl << endl;
	QString cppHeader = preCompHeaderOut + Option::dir_sep + "c++";
	t << cppHeader << ": " << header << " " 
          << findDependencies(header).join(" \\\n\t\t")
	  << "\n\t" << mkdir_p_asstring(preCompHeaderOut)
	  << "\n\t" << "$(CXX) -x c++-header -c $(CXXFLAGS) $(INCPATH) -o " << cppHeader << " " << header
          << endl << endl;
    }
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
        if(project->isEmpty("QMAKE_COPY_FILE"))
            project->variables()["QMAKE_COPY_FILE"].append("$(COPY)");
        if(project->isEmpty("QMAKE_COPY_DIR"))
            project->variables()["QMAKE_COPY_DIR"].append("xcopy /s /q /y /i");
        if(project->isEmpty("QMAKE_INSTALL_FILE"))
            project->variables()["QMAKE_INSTALL_FILE"].append("$(COPY_FILE)");
        if(project->isEmpty("QMAKE_INSTALL_DIR"))
            project->variables()["QMAKE_INSTALL_DIR"].append("$(COPY_DIR)");
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

    // precomp
    if (!project->first("PRECOMPILED_HEADER").isEmpty() 
        && project->isActiveConfig("precompile_header")) {
        QString preCompHeader = var("OBJECTS_DIR") 
		         + QFileInfo(project->first("PRECOMPILED_HEADER")).fileName();
	preCompHeaderOut = preCompHeader + ".gch";
	project->variables()["QMAKE_CLEAN"].append(preCompHeaderOut + Option::dir_sep + "c");
	project->variables()["QMAKE_CLEAN"].append(preCompHeaderOut + Option::dir_sep + "c++");

	project->variables()["QMAKE_RUN_CC"].clear();
	project->variables()["QMAKE_RUN_CC"].append("$(CC) -c -include " + preCompHeader +
                                                    " $(CFLAGS) $(INCPATH) -o $obj $src");
        project->variables()["QMAKE_RUN_CC_IMP"].clear();
	project->variables()["QMAKE_RUN_CC_IMP"].append("$(CC)  -c -include " + preCompHeader +
                                                        " $(CFLAGS) $(INCPATH) -o $@ $<");
        project->variables()["QMAKE_RUN_CXX"].clear();
	project->variables()["QMAKE_RUN_CXX"].append("$(CXX) -c -include " + preCompHeader +
                                                     " $(CXXFLAGS) $(INCPATH) -o $obj $src");
        project->variables()["QMAKE_RUN_CXX_IMP"].clear();
	project->variables()["QMAKE_RUN_CXX_IMP"].append("$(CXX) -c -include " + preCompHeader +
                                                         " $(CXXFLAGS) $(INCPATH) -o $@ $<");
    }
    
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

QString MingwMakefileGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out)
{
    QString ret = MakefileGenerator::replaceExtraCompilerVariables(var, in, out);

    if (ret.contains("$(OBJECTS_DIR)")) {
        ret.replace("$(OBJECTS_DIR)/",  project->first("OBJECTS_DIR"));
        ret.replace("$(OBJECTS_DIR)\\",  project->first("OBJECTS_DIR"));
        ret.replace("$(OBJECTS_DIR)",  project->first("OBJECTS_DIR"));
    }
    return ret;
}


void MingwMakefileGenerator::writeObjectsPart(QTextStream &t)
{
    if (project->variables()["OBJECTS"].count() < var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        objectsLinkLine = "$(OBJECTS)";
    } else if (project->isActiveConfig("staticlib")) {
	QString ar_script_file = var("QMAKE_LINK_OBJECT_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ar_script_file += "." + var("BUILD_NAME");
	}
	createArObjectScriptFile(ar_script_file, var("DEST_TARGET"), project->variables()["OBJECTS"]);
        objectsLinkLine = "ar -M < " + ar_script_file;
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
    t << "all: " << fileFixify(Option::output.fileName()) << " " << varGlue("ALL_DEPS"," "," "," ") << " $(DESTDIR_TARGET)" << endl << endl;
    t << "$(DESTDIR_TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) " << var("POST_TARGETDEPS");
    if(project->isActiveConfig("staticlib")) {
	if (project->variables()["OBJECTS"].count() < var("QMAKE_LINK_OBJECT_MAX").toInt()) {
            t << "\n\t" << "$(LIB) \"$(DESTDIR_TARGET)\" " << objectsLinkLine << " " ;
        } else {
            t << "\n\t" << objectsLinkLine << " " ;
        }
    } else {
        t << "\n\t" << "$(LINK) $(LFLAGS) -o \"$(DESTDIR_TARGET)\" " << objectsLinkLine << " " << " $(LIBS)";
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

void MingwMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if (var == "QMAKE_PRL_LIBS") {
        QString where = "QMAKE_LIBS";
        if (!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            where = project->first("QMAKE_INTERNAL_PRL_LIBS");
        QStringList &out = project->variables()[where];
        for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            out.removeAll((*it));
            out.append((*it));
        }
    } else {
        Win32MakefileGenerator::processPrlVariable(var, l);
    }
}

QStringList &MingwMakefileGenerator::findDependencies(const QString &file)
{
	QStringList &aList = MakefileGenerator::findDependencies(file);
    // Note: The QMAKE_IMAGE_COLLECTION file have all images
    // as dependency, so don't add precompiled header then
    if (file == project->first("QMAKE_IMAGE_COLLECTION") 
        || preCompHeaderOut.isEmpty())
        return aList;
    if (file.endsWith(".c")) {
	QString cHeader = preCompHeaderOut + Option::dir_sep + "c";
        if (!aList.contains(cHeader))
               aList += cHeader;
    } else {
        for (QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
	    if (file.endsWith(*it)) {
                QString cppHeader = preCompHeaderOut + Option::dir_sep + "c++";
	        if (!aList.contains(cppHeader))
                    aList += cppHeader;
                    break;
            }
        }
    }
    return aList;
}
    
