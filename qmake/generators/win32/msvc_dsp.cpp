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

#include "msvc_dsp.h"
#include "option.h"

#include <qdir.h>

#include <stdlib.h>

DspMakefileGenerator::DspMakefileGenerator() : Win32MakefileGenerator(), init_flag(false)
{
}

bool DspMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        /* for now just dump, I need to generated an empty dsp or something.. */
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").toLatin1().constData());
        return true;
    }

    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
        return writeDspParts(t);
    } else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
}

static inline QString postfixForConfig(const QString &config, bool &isDebug)
{
    if (config.contains("debug", Qt::CaseInsensitive)) {
        isDebug = true;
        return "DBG";
    }
    isDebug = false;
    return "REL";
}

#define begin_configs(config) \
for (int iconfig = 0; iconfig < configurations.count(); ++iconfig) { \
    QString config(configurations.at(iconfig)); \
    bool isDebug = false; \
    QString postfix(postfixForConfig(config, isDebug)); \
    if (!iconfig) \
        t << "!IF"; \
    t << "  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " " << config << "\"" << endl; \
    t << endl \

#define end_configs() \
    if (iconfig == configurations.count() - 1) \
        t << "!ENDIF " << endl; \
    else \
        t << "!ELSEIF"; \
} \

bool DspMakefileGenerator::hasBuiltinCompiler(const QString &filename) const
{
    for (int i = 0; i < Option::cpp_ext.count(); ++i)
        if (filename.endsWith(Option::cpp_ext.at(i)))
            return true;
    return filename.endsWith(".c");
}

QString DspMakefileGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out)
{
    QString ret = MakefileGenerator::replaceExtraCompilerVariables(var, in, out);
    ret.replace("$(DEFINES)",  varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") +
                varGlue("DEFINES"," -D"," -D",""));

    QString incpath = this->var("MSVCDSP_INCPATH");
    incpath.replace("/I", "-I");
    ret.replace("$(INCPATH)", incpath);
    return ret;
}

bool DspMakefileGenerator::writeBuildstepForFile(QTextStream &t, const QString &file)
{
    if (usePCH) {
        if (file.endsWith(".c")) {
            t << "# SUBTRACT CPP /FI\"" << namePCH << "\" /Yu\"" << namePCH << "\" /Fp" << endl;
            return true;
        } else if (precompH.endsWith(file)) {
            // ### dependency list quickly becomes too long for VS to grok...
            t << "USERDEP_" << file << "=" << valGlue(findDependencies(precompH), "\"", "\"\t\"", "\"") << endl;
            t << endl;
            begin_configs(config);
                t << "# Begin Custom Build - Creating precompiled header from " << file << "..." << endl;
                t << "InputPath=.\\" << file << endl << endl;
                t << precompPch + ": $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"" << endl;
                t << "\tcl.exe /TP /W3 /FD /c /Yc /Fp" << precompPch << " /Fo" << precompObj << " /Fd\"$(IntDir)\\\\\" " << file << " ";
                t << var("MSVCDSP_INCPATH") << " " << var("MSVCDSP_DEFINES") << " " << var("MSVCDSP_CXXFLAGS") << " " << var("MSVCDSP_CXXFLAGS_" + postfix) << endl;
                t << "# End Custom Build" << endl << endl;
            end_configs();

            return true;
        }
    }

    QString fileBase = file.left(file.lastIndexOf('.'));
    fileBase = fileBase.mid(fileBase.lastIndexOf('\\') + 1);

    bool hasBuiltin = hasBuiltinCompiler(file);
    BuildStep allSteps(configurations.count());

    if (!swappedBuildSteps.contains(file)) {
        QStringList compilers = project->variables()["QMAKE_EXTRA_COMPILERS"];
        for (int i = 0; i < compilers.count(); ++i) {
            QString compiler = compilers.at(i);
            if (project->variables()[compiler + ".input"].isEmpty())
                continue;
            QString input = project->variables()[compiler + ".input"].first();
            QStringList inputList = project->variables()[input];
            if (!inputList.contains(file))
                continue;

            QStringList compilerCommands = project->variables()[compiler + ".commands"];
            QStringList compilerOutput = project->variables()[compiler + ".output"];
            if (compilerCommands.isEmpty() || compilerOutput.isEmpty())
                continue;

            QStringList compilerName = project->variables()[compiler + ".name"];
            if (compilerName.isEmpty())
                compilerName << compiler;
            QStringList compilerDepends = project->variables()[compiler + ".depends"];
            QStringList compilerConfig = project->variables()[compiler + ".CONFIG"];
            
            if (!verifyExtraCompiler(compiler, file))
                continue;

            bool combineAll = compilerConfig.contains("combine");
            if (combineAll && inputList.first() != file)
                continue;

            QString fileIn("$(InputPath)");

            if (combineAll && !inputList.isEmpty()) {
                fileIn = inputList.join(" ");
                compilerDepends += inputList;
            }

            QString fileOut(compilerOutput.first());
            fileOut.replace("${QMAKE_FILE_BASE}", fileBase);
            fileOut.replace('/', '\\');

            BuildStep step(configurations.count());
            for (int i2 = 0; i2 < compilerDepends.count(); ++i2) {
                QString dependency = compilerDepends.at(i2);
                dependency.replace("${QMAKE_FILE_BASE}", fileBase);
                dependency.replace('/', '\\');
                if (!step.deps.contains(dependency, Qt::CaseInsensitive))
                    step.deps << dependency;
            }

            QString mappedFile;
            if (hasBuiltin) {
                mappedFile = fileOut;
                fileOut = fileIn;
                fileIn = file;
            }

            for (int iconfig = 0; iconfig < configurations.count(); ++iconfig) {
                QString config(configurations.at(iconfig));
                QString &buildName = step.buildNames[iconfig];
                QString &buildStep = step.buildSteps[iconfig];
                QStringList &buildOutput = step.buildOutputs[iconfig];

                if (buildStep.isEmpty())
                    buildStep = "BuildCmds= \\\n\t";
                else
                    buildStep += " \\\n\t";
                QString command(compilerCommands.join(" "));
                // Might be a macro, and not a valid filename, so the replaceExtraCompilerVariables() would eat it
                command.replace("${QMAKE_FILE_IN}", fileIn);
                command = replaceExtraCompilerVariables(command, fileIn, fileOut);

                buildName = compilerName.first();
                buildStep += command;
                buildOutput += fileOut;
            }
            if (hasBuiltin) {
                step.deps << fileIn;
                swappedBuildSteps[mappedFile] = step;
            } else {
                allSteps << step;
            }
        }
    } else {
        allSteps << swappedBuildSteps.value(file);
    }

    if (allSteps.buildSteps.at(0).isEmpty())
        return true;

    int i;
    QStringList dependencyList;
    // remove dependencies that are also output
    for (i = 0; i < configurations.count(); ++i) {
        QString config = configurations.at(i);
        QStringList buildOutput(allSteps.buildOutputs.at(configurations.indexOf(config)));

        for (int i2 = 0; i2 < allSteps.deps.count(); ++i2) {
            QString dependency = allSteps.deps.at(i2);
            if (!buildOutput.contains(dependency) && !dependencyList.contains(dependency))
                dependencyList << dependency;
        }
    }
    QString allDependencies = valGlue(dependencyList, "\"", "\"\t\"", "\"");
    t << "USERDEP_" << file << "=" << allDependencies << endl;
    begin_configs(config);
        QString buildName(allSteps.buildNames.at(iconfig));
        QString buildStep(allSteps.buildSteps.at(iconfig));
        QStringList buildOutputList(allSteps.buildOutputs.at(iconfig));
        t << "# Begin Custom Build - Running " << buildName << " on " << file << endl;
        t << "InputPath=" << file << endl;
        t << buildStep << endl;
        for (i = 0; i < buildOutputList.count(); ++i) {
            t << "\"" << buildOutputList.at(i)
              << "\": $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n\t$(BuildCmds)\n";
        }
        t << endl;
        t << "# End Custom Build" << endl;
    end_configs();

    return true;
}

bool DspMakefileGenerator::writeFileGroup(QTextStream &t, const QStringList &files, const QString &group, const QString &filter)
{
    if (files.isEmpty())
        return false;

    t << "# Begin Group \"" << group << "\"" << endl;
    t << "# PROP Default_Filter \"" << filter << "\"" << endl;
    for (int i = 0; i < files.count(); ++i) {
        QString file = files.at(i);
        t << "# Begin Source File" << endl;
        t << "SOURCE=" << file << endl;
        writeBuildstepForFile(t, file);
        t << "# End Source File" << endl;
        t << endl;
    }
    t << "# End Group" << endl;
    t << endl;

    return true;
}

bool
DspMakefileGenerator::writeDspParts(QTextStream &t)
{
    bool staticLibTarget = var("MSVCDSP_DSPTYPE") == "0x0104";

    t << "# Microsoft Developer Studio Project File - Name=\"" << var("MSVCDSP_PROJECT") << "\" - Package Owner=<4>" << endl;
    t << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << endl;
    t << "# ** DO NOT EDIT **" << endl;
    t << endl;
    t << "# TARGTYPE \"Win32 (x86) " << var("MSVCDSP_TARGETTYPE") << "\" " << var("MSVCDSP_DSPTYPE") << endl;
    t << endl;
    t << "CFG=" << var("MSVCDSP_PROJECT") << " - " << platform << " " << configurations.first() << endl;
    t << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << endl;
    t << "!MESSAGE use the Export Makefile command and run" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE NMAKE /f \"" << var("MSVCDSP_PROJECT") << ".mak\"." << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE You can specify a configuration when running NMAKE" << endl;
    t << "!MESSAGE by defining the macro CFG on the command line. For example:" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE NMAKE /f \"" << var("MSVCDSP_PROJECT") << ".mak\" CFG=\"" << var("MSVCDSP_PROJECT") << " - " << platform << " " << configurations.first() << "\"" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE Possible choices for configuration are:" << endl;
    t << "!MESSAGE " << endl;
    int i;
    for (i = 0; i < configurations.count(); ++i) {
        QString config = configurations.at(i);
        t << "!MESSAGE \"" << var("MSVCDSP_PROJECT") << " - " << platform << " " << config << "\" (based on \"Win32 (x86) " << var("MSVCDSP_TARGETTYPE") << "\")" << endl;
    }
    t << "!MESSAGE " << endl;
    t << endl;
    t << "# Begin Project" << endl;
    t << "# PROP AllowPerConfigDependencies 0" << endl;
    t << "# PROP Scc_ProjName \"\"" << endl;
    t << "# PROP Scc_LocalPath \"\"" << endl;
    t << "CPP=cl.exe" << endl;
    t << "MTL=midl.exe" << endl;
    t << "RSC=rc.exe" << endl;
    t << "BSC32=bscmake.exe" << endl;
    t << endl;

    begin_configs(config);
        t << "# PROP BASE Use_MFC 0" << endl;
        t << "# PROP BASE Use_Debug_Libraries " << (isDebug ? "1" : "0") << endl;
        t << "# PROP BASE Output_Dir \"" << var("MSVCDSP_TARGETDIR_" + postfix) << "\"" << endl;
        t << "# PROP BASE Intermediate_Dir \"" << var("MSVCDSP_OBJECTSDIR_" + postfix) << "\"" << endl;
        t << "# PROP BASE Target_Dir \"\"" << endl;
        t << "# PROP Use_MFC 0" << endl;
        t << "# PROP Use_Debug_Libraries " << (isDebug ? "1" : "0") << endl;
        t << "# PROP Output_Dir \"" << var("MSVCDSP_TARGETDIR_" + postfix) << "\"" << endl;
        t << "# PROP Intermediate_Dir \"" << var("MSVCDSP_OBJECTSDIR_" + postfix) << "\"" << endl;
        if (project->isActiveConfig("dll") || project->isActiveConfig("plugin"))
            t << "# PROP Ignore_Export_Lib 1" << endl;
        t << "# PROP Target_Dir \"\"" << endl;
        t << "# ADD CPP " << var("MSVCDSP_INCPATH") << " /c /FD " << var("MSVCDSP_CXXFLAGS_" + postfix) << " " << var("MSVCDSP_DEFINES") << " " << var("PRECOMPILED_FLAGS") << endl;
        t << "# ADD MTL /nologo /mktyplib203 /win32 /D " << (isDebug ? "\"_DEBUG\"" : "\"NDEBUG\"") << endl;
        t << "# ADD RSC /l 0x409 /d " << (isDebug ? "\"_DEBUG\"" : "\"NDEBUG\"") << endl;
        t << "# ADD BSC32 /nologo" << endl;
        if (staticLibTarget) {
            t << "LIB32=link.exe -lib" << endl;
            t << "# ADD LIB32 -nologo " << var("MSVCDSP_TARGET") << " " << var("PRECOMPILED_OBJECT") << endl;
        } else {
            t << "LINK32=link.exe" << endl;
            t << "# ADD LINK32 " << var("MSVCDSP_LFLAGS") << " " << var("MSVCDSP_LFLAGS_" + postfix) << " " << var("MSVCDSP_LIBS") << " " << var("MSVCDSP_TARGET") << " " << var("PRECOMPILED_OBJECT") << endl;
        }

        if (!project->variables()["MSVCDSP_POST_LINK_" + postfix].isEmpty())
            t << project->variables()["MSVCDSP_POST_LINK_" + postfix].first();
        t << endl;
    end_configs();

    t << endl;
    t << "# Begin Target" << endl;
    t << endl;
    for (i = 0; i < configurations.count(); ++i) {
        QString config = configurations.at(i);
        t << "# Name \"" << var("MSVCDSP_PROJECT") << " - " << platform << " " << config << "\"" << endl;
    }
    t << endl;

    if (project->isActiveConfig("flat")) {
        writeFileGroup(t, project->variables()["SOURCES"] + project->variables()["DEF_FILE"],
            "Source Files", "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat");
        writeFileGroup(t, project->variables()["HEADERS"], "Header Files", "h;hpp;hxx;hm;inl");
        writeFileGroup(t, project->variables()["FORMS"] + project->variables()["INTERFACES"],
            "Form Files", "ui");
        writeFileGroup(t, project->variables()["IMAGES"], "Image Files", "");
        writeFileGroup(t, project->variables()["RESOURCES"] + project->variables()["RC_FILE"],
            "Resources", "");
        writeFileGroup(t, project->variables()["TRANSLATIONS"], "Translations", "ts");
        writeFileGroup(t, project->variables()["LEXSOURCES"], "Lexables", "l");
        writeFileGroup(t, project->variables()["YACCSOURCES"], "Yaccables", "y");
        writeFileGroup(t, project->variables()["TYPELIBS"], "Type Libraries", "tlb;olb");
    } else { // directory mode
        QStringList list(project->variables()["SOURCES"]
            + project->variables()["GENERATED_SOURCES"]
            + project->variables()["DEF_FILE"]
            + project->variables()["IMAGES"]
            + project->variables()["RC_FILE"]
            + project->variables()["TRANSLATIONS"]
            + project->variables()["LEXSOURCES"]
            + project->variables()["YACCSOURCES"]);
        if(!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
            const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
            for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
                const QStringList &inputs = project->values((*it)+".input");
                for(QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); ++input)
                    list += project->values((*input));
            }
        }

        list.sort();
        for (i = 0; i < list.count(); ++i) {
            QString file = list.at(i);
            beginGroupForFile(file, t);
            t << "# Begin Source File" << endl;
            t << "SOURCE=" << file << endl;
            writeBuildstepForFile(t, file);
            t << "# End Source File" << endl;
            t << endl;
        }
        endGroups(t);
    }

    writeFileGroup(t, project->variables()["GENERATED_SOURCES"] 
                      + swappedBuildSteps.keys(), "Generated", "");

    t << "# End Target" << endl;
    t << "# End Project" << endl;
    return true;
}

void
DspMakefileGenerator::init()
{
    if(init_flag)
        return;
    QStringList::Iterator it;
    init_flag = true;

    platform = "Win32";
    if(!project->variables()["QMAKE_PLATFORM"].isEmpty())
        platform = varGlue("QMAKE_PLATFORM", "", " ", "");

    if(!project->isActiveConfig("debug"))
        configurations << "Release" << "Debug";
    else
        configurations << "Debug" << "Release";


	// this should probably not be here, but I'm using it to wrap the .t files
    if(project->first("TEMPLATE") == "vcapp")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");

    if(project->variables()["QMAKESPEC"].isEmpty())
        project->variables()["QMAKESPEC"].append(qgetenv("QMAKESPEC"));

    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"].first();
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(".", "");
        project->variables()["MSVCDSP_LFLAGS"].append("/VERSION:" + major + "." + minor);
    }

    if(project->isActiveConfig("qt")) {
        if(project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
        } else {
            if(!project->variables()["QMAKE_QT_DLL"].isEmpty()) {
                int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
                if(hver != -1) {
                    QString ver;
                    ver.sprintf("qt" QTDLL_POSTFIX "%d.lib", hver);
                    QStringList &libs = project->variables()["QMAKE_LIBS"];
                    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
                        (*libit).replace(QRegExp("qt\\.lib"), ver);
                }
            }
        }
    }

    if(project->isActiveConfig("debug")) {
        if(!project->first("OBJECTS_DIR").isEmpty())
            project->variables()["MSVCDSP_OBJECTSDIR_DBG"] = QStringList(project->first("OBJECTS_DIR"));
        else
            project->variables()["MSVCDSP_OBJECTSDIR_DBG"] = QStringList("Debug");
        project->variables()["MSVCDSP_OBJECTSDIR_REL"] = QStringList("Release");
        if(!project->first("DESTDIR").isEmpty())
            project->variables()["MSVCDSP_TARGETDIR_DBG"] = QStringList(project->first("DESTDIR"));
        else
            project->variables()["MSVCDSP_TARGETDIR_DBG"] = QStringList("Debug");
        project->variables()["MSVCDSP_TARGETDIR_REL"] = QStringList("Release");
    } else {
        if(!project->first("OBJECTS_DIR").isEmpty())
            project->variables()["MSVCDSP_OBJECTSDIR_REL"] = QStringList(project->first("OBJECTS_DIR"));
        else
            project->variables()["MSVCDSP_OBJECTSDIR_REL"] = QStringList("Release");
        project->variables()["MSVCDSP_OBJECTSDIR_DBG"] = QStringList("Debug");
        if(!project->first("DESTDIR").isEmpty())
            project->variables()["MSVCDSP_TARGETDIR_REL"] = QStringList(project->first("DESTDIR"));
        else
            project->variables()["MSVCDSP_TARGETDIR_REL"] = QStringList("Release");
        project->variables()["MSVCDSP_TARGETDIR_DBG"] = QStringList("Debug");
    }

    fixTargetExt();

    QString msvcdsp_project;
    if(project->variables()["TARGET"].count())
        msvcdsp_project = project->variables()["TARGET"].first();

    project->variables()["TARGET"].first() += project->first("TARGET_EXT");

    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    processFileTagsVar();

    MakefileGenerator::init();
    if(msvcdsp_project.isEmpty())
        msvcdsp_project = Option::output.fileName();

    msvcdsp_project = msvcdsp_project.right(msvcdsp_project.length() - msvcdsp_project.lastIndexOf("\\") - 1);
    int dotFind = msvcdsp_project.lastIndexOf(".");
    if(dotFind != -1)
        msvcdsp_project = msvcdsp_project.left(dotFind);
    msvcdsp_project.replace("-", "");

    project->variables()["MSVCDSP_PROJECT"].append(msvcdsp_project);
    QStringList &proj = project->variables()["MSVCDSP_PROJECT"];

    for(QStringList::Iterator it = proj.begin(); it != proj.end(); ++it)
        (*it).replace(QRegExp("\\.[a-zA-Z0-9_]*$"), "");

    if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        if(project->isActiveConfig("console")) {
            project->variables()["MSVCDSP_TARGETTYPE"].append("Console Application");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0103");
            project->variables()["MSVCDSP_DEFINES"].append(" /D \"_CONSOLE\" ");
        } else {
            project->variables()["MSVCDSP_TARGETTYPE"].append("Application");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0101");
            project->variables()["MSVCDSP_DEFINES"].append(" /D \"_WINDOWS\" ");
        }
    } else {
        if(project->isActiveConfig("dll")) {
            project->variables()["MSVCDSP_TARGETTYPE"].append("Dynamic-Link Library");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0102");
            project->variables()["MSVCDSP_DEFINES"].append(" /D \"_USRDLL\" ");
        } else {
            project->variables()["MSVCDSP_TARGETTYPE"].append("Static Library");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0104");
            project->variables()["MSVCDSP_DEFINES"].append(" /D \"_LIB\" ");
        }
    }

    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_WINDOWS"];

    findLibraries();
    processPrlFiles();

    project->variables()["MSVCDSP_LFLAGS"] += project->variables()["QMAKE_LFLAGS"];
    if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
        project->variables()["MSVCDSP_LFLAGS"].append(varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\""));

    if(!project->variables()["QMAKE_LFLAGS_DEBUG"].isEmpty())
        project->variables()["MSVCDSP_LFLAGS_DBG"].append(var("QMAKE_LFLAGS_DEBUG"));
    if(!project->variables()["QMAKE_LFLAGS_RELEASE"].isEmpty())
        project->variables()["MSVCDSP_LFLAGS_REL"].append(var("QMAKE_LFLAGS_RELEASE"));

    // Create different compiler flags for release and debug: use default, remove opposite config, add current config
    QStringList msvcdsp_cxxflags = project->variables()["QMAKE_CXXFLAGS"];
    QStringList msvcdsp_cxxflags_rel(msvcdsp_cxxflags);
    QStringList msvcdsp_cxxflags_deb(msvcdsp_cxxflags);

    QStringList relflags(project->variables()["QMAKE_CXXFLAGS_RELEASE"]);
    QStringList dbgflags(project->variables()["QMAKE_CXXFLAGS_DEBUG"]);
    if (project->isActiveConfig("shared")) {
        relflags += project->variables()["QMAKE_CXXFLAGS_MT_DLL"];
        dbgflags += project->variables()["QMAKE_CXXFLAGS_MT_DLLDBG"];
    } else {
        relflags += project->variables()["QMAKE_CXXFLAGS_MT"];
        dbgflags += project->variables()["QMAKE_CXXFLAGS_MT_DBG"];
    }

    int i;
    for (i = 0; i < relflags.count(); ++i) {
        QString flag(relflags.at(i));
        msvcdsp_cxxflags_deb.removeAll(flag);
        if (!msvcdsp_cxxflags_rel.contains(flag))
            msvcdsp_cxxflags_rel.append(flag);
    }
    for (i = 0; i < dbgflags.count(); ++i) {
        QString flag(dbgflags.at(i));
        msvcdsp_cxxflags_rel.removeAll(flag);
        if (!msvcdsp_cxxflags_deb.contains(flag))
            msvcdsp_cxxflags_deb.append(flag);
    }

    project->variables()["MSVCDSP_CXXFLAGS_REL"] += msvcdsp_cxxflags_rel;
    project->variables()["MSVCDSP_CXXFLAGS_DBG"] += msvcdsp_cxxflags_deb;

    project->variables()["MSVCDSP_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_DEFINES"].append(varGlue("PRL_EXPORT_DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_DEFINES"].append(" /D \"WIN32\" ");

    if (!project->variables()["RES_FILE"].isEmpty())
	project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];

    QStringList &libs = project->variables()["QMAKE_LIBS"];
    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit) {
        QString lib = (*libit);
        lib.replace(QRegExp("\""), "");
        project->variables()["MSVCDSP_LIBS"].append(" \"" + lib + "\"");
    }

    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
        QString inc = (*incit);
        inc.replace("\"", "");
        if(inc.endsWith("\\")) // Remove trailing \'s from paths
            inc.truncate(inc.length()-1);
        project->variables()["MSVCDSP_INCPATH"].append("/I\"" + inc + "\"");
    }
    project->variables()["MSVCDSP_INCPATH"].append("/I\"" + specdir() + "\"");

    QString dest;
    QString postLinkStep;
    QString copyDllStep;

    if(!project->variables()["QMAKE_POST_LINK"].isEmpty())
        postLinkStep += var("QMAKE_POST_LINK");

    if(!project->variables()["DESTDIR"].isEmpty()) {
        project->variables()["TARGET"].first().prepend(project->first("DESTDIR"));
        Option::fixPathToTargetOS(project->first("TARGET"));
        dest = project->first("TARGET");
        if(project->first("TARGET").startsWith("$(QTDIR)"))
            dest.replace("$(QTDIR)", qgetenv("QTDIR"));
        project->variables()["MSVCDSP_TARGET"].append(
            QString("/out:\"") + dest + "\"");
        if(project->isActiveConfig("dll")) {
            QString imp = dest;
            imp.replace(".dll", ".lib");
            project->variables()["MSVCDSP_TARGET"].append(QString(" /implib:\"") + imp + "\"");
        }
    }

    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
        QStringList dlldirs = project->variables()["DLLDESTDIR"];
        if(dlldirs.count())
            copyDllStep += "\t";
        for(QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir) {
            copyDllStep += "copy \"$(TargetPath)\" \"" + *dlldir + "\"\t";
        }
    }

    if(!postLinkStep.isEmpty() || !copyDllStep.isEmpty()) {
        project->variables()["MSVCDSP_POST_LINK_DBG"].append(
            "# Begin Special Build Tool\n"
            "SOURCE=$(InputPath)\n"
            "PostBuild_Desc=Post Build Step\n"
            "PostBuild_Cmds=" + postLinkStep + copyDllStep + "\n"
            "# End Special Build Tool\n");
        project->variables()["MSVCDSP_POST_LINK_REL"].append(
            "# Begin Special Build Tool\n"
            "SOURCE=$(InputPath)\n"
            "PostBuild_Desc=Post Build Step\n"
            "PostBuild_Cmds=" + postLinkStep + copyDllStep + "\n"
            "# End Special Build Tool\n");
    }

    QStringList &list = project->variables()["FORMS"];
    for(QStringList::ConstIterator hit = list.begin(); hit != list.end(); ++hit) {
        if(QFile::exists(*hit + ".h"))
            project->variables()["SOURCES"].append(*hit + ".h");
    }
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "MSVCDSP_LIBS";

    // Move some files around
    if (!project->variables()["IMAGES"].isEmpty()) {
        QString imageFactory(project->variables()["QMAKE_IMAGE_COLLECTION"].first());
        project->variables()["GENERATED_SOURCES"] += imageFactory;
        project->variables()["SOURCES"].removeAll(imageFactory);
    }

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    namePCH = QFileInfo(precompH).fileName();
    usePCH = !precompH.isEmpty() && project->isActiveConfig("precompile_header");
    if (usePCH) {
        // Created files
        QString origTarget = project->first("QMAKE_ORIG_TARGET");
        origTarget.replace(QRegExp("-"), "_");
        precompObj = "\"$(IntDir)\\" + origTarget + Option::obj_ext + "\"";
        precompPch = "\"$(IntDir)\\" + origTarget + ".pch\"";
        // Add PRECOMPILED_HEADER to HEADERS
        if (!project->variables()["HEADERS"].contains(precompH))
            project->variables()["HEADERS"] += precompH;
        // Add precompile compiler options
        project->variables()["PRECOMPILED_FLAGS"]  = QStringList("/Yu\"" + namePCH + "\" /FI\"" + namePCH + "\" ");
        // Return to variable pool
        project->variables()["PRECOMPILED_OBJECT"] = QStringList(precompObj);
        project->variables()["PRECOMPILED_PCH"]    = QStringList(precompPch);
    }
}

void DspMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_DEFINES") {
        QStringList &out = project->variables()["MSVCDSP_DEFINES"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(out.indexOf((*it)) == -1)
                out.append((" /D \"" + *it + "\""));
        }
    } else {
        MakefileGenerator::processPrlVariable(var, l);
    }
}

void DspMakefileGenerator::beginGroupForFile(QString file, QTextStream &t,
                                        const QString& filter)
{
    fileFixify(file, qmake_getpwd(), qmake_getpwd(), FileFixifyRelative);
    file = file.section(Option::dir_sep, 0, -2);
    if(file.right(Option::dir_sep.length()) != Option::dir_sep)
        file += Option::dir_sep;
    if(file == currentGroup)
        return;

    if(file.isEmpty() || !QDir::isRelativePath(file)) {
        endGroups(t);
        return;
    }

    QString tempFile = file;
    if(tempFile.startsWith(currentGroup))
        tempFile = tempFile.mid(currentGroup.length());
    int dirSep = currentGroup.lastIndexOf(Option::dir_sep);

    while(!tempFile.startsWith(currentGroup) && dirSep != -1) {
        currentGroup.truncate(dirSep);
        dirSep = currentGroup.lastIndexOf(Option::dir_sep);
        if(!tempFile.startsWith(currentGroup) && dirSep != -1)
            t << "\n# End Group\n";
    }
    if(!file.startsWith(currentGroup)) {
        t << "\n# End Group\n";
        currentGroup = "";
    }

    QStringList dirs = file.right(file.length() - currentGroup.length()).split(Option::dir_sep);
    for(QStringList::Iterator dir_it = dirs.begin(); dir_it != dirs.end(); ++dir_it) {
        if ((*dir_it).isEmpty())
            continue;
        t << "# Begin Group \"" << (*dir_it) << "\"" << endl;
        t << "# Prop Default_Filter \"" << filter << "\"" << endl;
    }
    currentGroup = file;
    if (currentGroup == "\\")
        currentGroup = 0;
}

void DspMakefileGenerator::endGroups(QTextStream &t)
{
    if(currentGroup.isEmpty())
        return;

    QStringList dirs = currentGroup.split(Option::dir_sep);
    for(QStringList::Iterator dir_it = dirs.end(); dir_it != dirs.begin(); --dir_it) {
        t << "\n# End Group\n";
    }
    currentGroup = "";
}

bool DspMakefileGenerator::openOutput(QFile &file, const QString &build) const
{
    QString outdir;
    if(!file.fileName().isEmpty()) {
        if(QDir::isRelativePath(file.fileName()))
            file.setFileName(Option::output_dir + file.fileName()); //pwd when qmake was run
        QFileInfo fi(file);
        if(fi.isDir())
            outdir = file.fileName() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.fileName().isEmpty())
        file.setFileName(outdir + project->first("QMAKE_ORIG_TARGET") + project->first("DSP_EXTENSION"));
    if(QDir::isRelativePath(file.fileName())) {
        QString ofile = Option::fixPathToLocalOS(file.fileName());
        int slashfind = ofile.lastIndexOf(Option::dir_sep);
        if(slashfind == -1) {
            ofile = ofile.replace(QRegExp("-"), "_");
        } else {
            int hypenfind = ofile.indexOf('-', slashfind);
            while (hypenfind != -1 && slashfind < hypenfind) {
                ofile = ofile.replace(hypenfind, 1, "_");
                hypenfind = ofile.indexOf('-', hypenfind + 1);
            }
        }
        file.setFileName(Option::fixPathToLocalOS(qmake_getpwd() + Option::dir_sep + ofile));
    }
    return Win32MakefileGenerator::openOutput(file, build);
}
