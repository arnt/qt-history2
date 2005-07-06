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
#include <qset.h>

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

    if ((project->first("TEMPLATE") == "vcapp"
       || project->first("TEMPLATE") == "vclib")
       && !project->isActiveConfig("build_pass")) {
        return writeDspParts(t);
    } else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return true;
}

bool DspMakefileGenerator::hasBuiltinCompiler(const QString &filename) const
{
    for (int i = 0; i < Option::cpp_ext.count(); ++i)
        if (filename.endsWith(Option::cpp_ext.at(i)))
            return true;
    return filename.endsWith(".c");
}

QString DspMakefileGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out,
                                                            bool local)
{
    QString ret = MakefileGenerator::replaceExtraCompilerVariables(var, in, out, local);
    ret.replace("$(DEFINES)",  varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") +
                varGlue("DEFINES"," -D"," -D",""));

    QString incpath;
    const QStringList &incpaths = project->values("MSVCDSP_INCPATH");
    for(int i = 0; i < incpaths.size(); ++i) {
        QString inc = incpaths[i];
        if(inc.startsWith(QLatin1String("-I")) || inc.startsWith(QLatin1String("/I"))) {
            inc = inc.mid(2);
            if(local)
                inc = Option::fixPathToLocalOS(inc, false);
            else
                inc = Option::fixPathToTargetOS(inc, false);
            if(!incpath.isEmpty())
                incpath += " ";
            incpath += "/I" + inc;
        }
    }
    ret.replace("$(INCPATH)", incpath);
    return ret;
}


// if config is part of a multibuild thenthe gule (this) has the correct MSVCDSP_PROJECT
QString DspMakefileGenerator::configName(DspMakefileGenerator * config)
{
    return var("MSVCDSP_PROJECT") + config->var("MSVCDSP_CONFIG_NAME");
}

bool DspMakefileGenerator::writeDspHeader(QTextStream &t)
{
    DspMakefileGenerator * config = this;
    if (mergedProjects.count())
        config = mergedProjects.at(0);

    t << "# Microsoft Developer Studio Project File - Name=\"" << var("MSVCDSP_PROJECT") << "\" - Package Owner=<4>" << endl;
    t << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << endl;
    t << "# ** DO NOT EDIT **" << endl;
    t << endl;
    t << "# TARGTYPE \"Win32 (x86) " << var("MSVCDSP_TARGETTYPE") << "\" " << var("MSVCDSP_DSPTYPE") << endl;
    t << endl;
    t << "CFG=" << configName(config) << endl;
    t << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << endl;
    t << "!MESSAGE use the Export Makefile command and run" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE NMAKE /f \"" << var("TARGET") << ".mak\"." << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE You can specify a configuration when running NMAKE" << endl;
    t << "!MESSAGE by defining the macro CFG on the command line. For example:" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE NMAKE /f \"" << var("TARGET") << ".mak\" CFG=\"" << configName(config) << "\"" << endl;
    t << "!MESSAGE " << endl;
    t << "!MESSAGE Possible choices for configuration are:" << endl;
    t << "!MESSAGE " << endl;
    if (mergedProjects.count()) {
        for (int i = 0; i < mergedProjects.count(); ++i) {
            DspMakefileGenerator * config = mergedProjects.at(i);
            t << "!MESSAGE \"" << configName(config) << "\" (based on \"Win32 (x86) " << config->var("MSVCDSP_TARGETTYPE") << "\")" << endl;
        }
    } else {
        t << "!MESSAGE \"" << configName(config) << "\" (based on \"Win32 (x86) " << config->var("MSVCDSP_TARGETTYPE") << "\")" << endl;
    }
    t << "!MESSAGE " << endl;
    t << endl;
    t << "# Begin Project" << endl;
    t << "# PROP AllowPerConfigDependencies 0" << endl;
    t << "# PROP Scc_ProjName \"\"" << endl;
    t << "# PROP Scc_LocalPath \"\"" << endl;
    t << "CPP=" << config->var("QMAKE_CC") << endl;
    t << "MTL=" << config->var("QMAKE_IDL") << endl;
    t << "RSC=" << config->var("QMAKE_RC") << endl;
    t << "BSC32=bscmake.exe" << endl;

    return true;
}


bool DspMakefileGenerator::writeDspParts(QTextStream &t)
{
    bool staticLibTarget = var("MSVCDSP_DSPTYPE") == "0x0104";

    writeDspHeader(t);
    writeDspConfig(t, this);
    t << endl;
    t << "# Begin Target" << endl;
    t << endl;
    t << "# Name \"" << configName(this) << "\"" << endl;
    t << endl;

    if (project->isActiveConfig("flat")) {

        QStringList listNames = QString("SOURCES|DEF_FILE").split("|");
        QStringList allListNames = listNames;
        writeFileGroup(t, listNames, "Source Files", "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat");
        listNames = QStringList("HEADERS");
        allListNames += listNames;
        writeFileGroup(t, QStringList("HEADERS"), "Header Files", "h;hpp;hxx;hm;inl");
        listNames = QString("FORMS|INTERFACES").split("|");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Form Files", "ui");
        listNames = QStringList("IMAGES");
        allListNames += listNames;
        writeFileGroup(t, QStringList("IMAGES"), "Image Files", "");
        listNames = QString("RC_FILE|RESOURCES").split("|");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Resources", "rc;qrc");
        listNames = QStringList("TRANSLATIONS");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Translations", "ts");
        listNames = QStringList("LEXSOURCES");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Lexables", "l");
        listNames = QStringList("YACCSOURCES");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Yaccables", "y");
        listNames = QStringList("TYPELIBS");
        allListNames += listNames;
        writeFileGroup(t, listNames, "Type Libraries", "tlb;olb");

        if (!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
             const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
             for (QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
                const QStringList &inputs = project->values((*it)+".input");
                for (QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); ++input) {
                    if (!allListNames.contains((*input)))
                        writeFileGroup(t, QStringList((*input)), (*input) + " Files", "");
                }
            }
        }

        project->variables()["SWAPPED_BUILD_STEPS"] = swappedBuildSteps.keys();

        writeFileGroup(t, QString("GENERATED_SOURCES|GENERATED_FILES|UIC3_HEADERS|SWAPPED_BUILD_STEPS").split("|"), "Generated", "");

    } else { // directory mode
        /* //###
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
                const QStringList &outputs = project->values((*it)+".output");
                for(QStringList::ConstIterator output = outputs.begin(); output != outputs.end(); ++output)
                    list += project->values((*output));
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
        */
    }


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

    // this should probably not be here, but I'm using it to wrap the .t files
    if(project->first("TEMPLATE") == "vcapp")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");

    if(project->variables()["QMAKESPEC"].isEmpty())
        project->variables()["QMAKESPEC"].append(qgetenv("QMAKESPEC"));

    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    processVars();

    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"].first();
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(".", "");
        project->variables()["MSVCDSP_LFLAGS"].append("/VERSION:" + major + "." + minor);
    }

    QString msvcdsp_project;
    if(project->variables()["TARGET"].count())
        msvcdsp_project = project->variables()["TARGET"].first();

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

    project->variables()["MSVCDSP_LFLAGS"] += project->variables()["QMAKE_LFLAGS"];

    if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
        project->variables()["MSVCDSP_LFLAGS"].append(varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\""));

    project->variables()["MSVCDSP_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_DEFINES"].append(varGlue("PRL_EXPORT_DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_DEFINES"].append(" /D \"WIN32\" ");

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

    // dont destroy the target, it is used by prl writer.
    if(!project->variables()["DESTDIR"].isEmpty()) {
        dest = project->first("DESTDIR");
        project->variables()["DESTDIR"].first() = dest;
        dest = project->variables()["TARGET"].first() + project->first("TARGET_EXT");
        dest.prepend(project->first("DESTDIR"));
        Option::fixPathToTargetOS(dest);
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
            copyDllStep += "copy \"$(TargetPath)\" \"" + Option::fixPathToTargetOS(*dlldir) + "\"\t";
        }
    }

    if(!postLinkStep.isEmpty() || !copyDllStep.isEmpty()) {
        project->variables()["MSVCDSP_POST_LINK"].append(
            "# Begin Special Build Tool\n"
            "SOURCE=$(InputPath)\n"
            "PostBuild_Desc=Post Build Step\n"
            "PostBuild_Cmds=" + postLinkStep + copyDllStep + "\n"
            "# End Special Build Tool\n");
    }

    QStringList &list = project->variables()["FORMS"];
    for(QStringList::ConstIterator hit = list.begin(); hit != list.end(); ++hit) {
        if(exists(*hit + ".h"))
            project->variables()["SOURCES"].append(*hit + ".h");
    }
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "MSVCDSP_LIBS";

    // Move some files around //### is this compat?
    if (!project->variables()["IMAGES"].isEmpty()) {
        QString imageFactory(project->variables()["QMAKE_IMAGE_COLLECTION"].first());
        project->variables()["GENERATED_SOURCES"] += imageFactory;
        project->variables()["SOURCES"].removeAll(imageFactory);
    }

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    namePCH = fileInfo(precompH).fileName();
    usePCH = !precompH.isEmpty() && project->isActiveConfig("precompile_header");
    if (usePCH) {
        // Created files
        precompObj = var("OBJECTS_DIR") + project->first("TARGET") + "_pch" + Option::obj_ext;
        precompPch = var("OBJECTS_DIR") + project->first("TARGET") + "_pch.pch";

        // Add PRECOMPILED_HEADER to HEADERS
        if (!project->variables()["HEADERS"].contains(precompH))
            project->variables()["HEADERS"] += precompH;
        // Add precompile compiler options
        project->variables()["PRECOMPILED_FLAGS"]  = QStringList("/Fp" + precompPch + " /Yu\"" + namePCH + "\" /FI\"" + namePCH + "\" ");
        // Return to variable pool
        project->variables()["PRECOMPILED_OBJECT"] = QStringList(precompObj);
        project->variables()["PRECOMPILED_PCH"]    = QStringList(precompPch);
    }

    QString buildName;
    if (!var("BUILD_NAME").isEmpty())
        buildName =  var("BUILD_NAME");
    else if (project->isActiveConfig("debug"))
        buildName = "Debug";
    else
        buildName = "Release";

    project->variables()["MSVCDSP_CONFIG_NAME"] = QStringList(" - " + platform + " " + buildName);
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
        currentGroup = "";
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
            file.setFileName(Option::output_dir + "/" + file.fileName()); //pwd when qmake was run
        QFileInfo fi(fileInfo(file.fileName()));
        if(fi.isDir())
            outdir = file.fileName() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.fileName().isEmpty()) {
        if(!project->isEmpty("QMAKE_DSP_PROJECT_NAME"))
            file.setFileName(outdir + project->first("QMAKE_DSP_PROJECT_NAME") + project->first("DSP_EXTENSION"));
        else
            file.setFileName(outdir + project->first("QMAKE_ORIG_TARGET") + project->first("DSP_EXTENSION"));
    }
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

bool DspMakefileGenerator::mergeBuildProject(MakefileGenerator *other)
{

    mergedProjects.prepend(static_cast<DspMakefileGenerator*>(other));
    return true;
}

bool DspMakefileGenerator::writeProjectMakefile()
{
    bool ret = true;

    QTextStream t(&Option::output);
    // Check if all requirements are fullfilled
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").toLatin1().constData());
        return true;
    }

    // Generate project file
    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
        if (!mergedProjects.count()) {
            warn_msg(WarnLogic, "Generator: MSVC DSP: no single configuration created, cannot output project!");
            return false;
        }
        debug_msg(1, "Generator: MSVC 6: Writing project file");

        writeDspHeader(t);
        int i = 0;
        for (i = 0; i < mergedProjects.count(); ++i) {
            DspMakefileGenerator* config = mergedProjects.at(i);
            t << endl;
            if (i == 0)
                t << "!IF";
            else
                t << "!ELSEIF";
            t << "  \"$(CFG)\" == \"" << configName(config) << "\"" << endl;
            t << endl;
            writeDspConfig(t, config);
        }
        t << endl;
        t << "!ENDIF " << endl;
        t << endl;
        t << "# Begin Target" << endl;
        t << endl;
        for (i = 0; i < mergedProjects.count(); ++i)
            t << "# Name \"" << configName(mergedProjects.at(i)) << "\"" << endl;
        t << endl;

        if (project->isActiveConfig("flat")) {

            QMap< QString, QSet<QString> > files;

            // merge source files
            int i = 0;
            for (i = 0; i < mergedProjects.count(); ++i) {

                DspMakefileGenerator* config = mergedProjects.at(i);

                files["DEF_FILE"] += config->project->variables()["DEF_FILE"].toSet();
                files["SOURCES"] += config->project->variables()["SOURCES"].toSet();
                files["HEADERS"] += config->project->variables()["HEADERS"].toSet();
                files["INTERFACES"] += config->project->variables()["INTERFACES"].toSet();
                files["FORMS"] += config->project->variables()["FORMS"].toSet();
                files["IMAGES"] += config->project->variables()["IMAGES"].toSet();
                files["RC_FILE"] += config->project->variables()["RC_FILE"].toSet();
                files["RESOURCES"] += config->project->variables()["RESOURCES"].toSet();
                files["TRANSLATIONS"] += config->project->variables()["TRANSLATIONS"].toSet();
                files["LEXSOURCES"] += config->project->variables()["LEXSOURCES"].toSet();
                files["YACCSOURCES"] += config->project->variables()["YACCSOURCES"].toSet();
                files["TYPELIBS"] += config->project->variables()["TYPELIBS"].toSet();

                if (!config->project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
                    const QStringList &quc = config->project->variables()["QMAKE_EXTRA_COMPILERS"];
                    for (QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
                        const QStringList &inputs = project->values((*it)+".input");
                        for (QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); ++input) {
                            files[(*input)] += config->project->variables()[(*input)].toSet();
                        }
                    }
                }
            }

            QStringList keys = files.keys();
            for (int k = 0; k < keys.size(); ++k)
                project->variables()[keys.at(k)] = QList<QString>::fromSet(files[keys.at(k)]);

            QStringList listNames = QString("SOURCES|DEF_FILE").split("|");
            QStringList allListNames = listNames;
            writeFileGroup(t, listNames, "Source Files", "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat");
            listNames = QStringList("HEADERS");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Header Files", "h;hpp;hxx;hm;inl");
            listNames = QString("FORMS|INTERFACES").split("|");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Form Files", "ui");
            listNames = QStringList("IMAGES");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Image Files", "");
            listNames = QString("RC_FILE|RESOURCES").split("|");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Resources", "rc;qrc");
            listNames = QStringList("TRANSLATIONS");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Translations", "ts");
            listNames = QStringList("LEXSOURCES");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Lexables", "l");
            listNames = QStringList("YACCSOURCES");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Yaccables", "y");
            listNames = QStringList("TYPELIBS");
            allListNames += listNames;
            writeFileGroup(t, listNames, "Type Libraries", "tlb;olb");

            for (int l = 0; l < allListNames.size(); ++l)
                keys.removeAll(allListNames.at(l));

            for (int k = 0; k < keys.size(); ++k)
                writeFileGroup(t, QStringList(keys.at(k)), keys.at(k) + " Files", "");

            // done last as generated may have changed when creating build rules for the above
            for (i = 0; i < mergedProjects.count(); ++i) {

                DspMakefileGenerator* config = mergedProjects.at(i);

                files["UIC3_HEADERS"] += config->project->variables()["UIC3_HEADERS"].toSet();

                config->project->variables()["SWAPPED_BUILD_STEPS"] = config->swappedBuildSteps.keys();
                files["SWAPPED_BUILD_STEPS"] +=  config->project->variables()["SWAPPED_BUILD_STEPS"].toSet();

                files["GENERATED_SOURCES"] += config->project->variables()["GENERATED_SOURCES"].toSet();
                files["GENERATED_FILES"] += config->project->variables()["GENERATED_FILES"].toSet();
            }

            project->variables()["SWAPPED_BUILD_STEPS"] = QList<QString>::fromSet(files["SWAPPED_BUILD_STEPS"]);
            project->variables()["UIC3_HEADERS"] = QList<QString>::fromSet(files["UIC3_HEADERS"]);
            project->variables()["GENERATED_SOURCES"] = QList<QString>::fromSet(files["GENERATED_SOURCES"]);
            project->variables()["GENERATED_FILES"] = QList<QString>::fromSet(files["GENERATED_FILES"]);

            writeFileGroup(t, QString("GENERATED_SOURCES|GENERATED_FILES|UIC3_HEADERS|SWAPPED_BUILD_STEPS").split("|"), "Generated", "");

        }
    }
    t << endl;
    t << "# End Target" << endl;
    t << "# End Project" << endl;

    return ret;
}

bool DspMakefileGenerator::writeFileGroup(QTextStream &t, const QStringList &listNames, const QString &group, const QString &filter)
{
    QMap<QString, QString> files;
    for (int i = 0; i < listNames.count(); ++i) {
        QStringList list = project->variables()[listNames.at(i)];
        for (int j = 0; j < list.count(); ++j) {
            files[list.at(j)] = listNames.at(i);
        }
    }

    if (files.isEmpty())
        return false;

    t << "# Begin Group \"" << group << "\"" << endl;
    t << "# PROP Default_Filter \"" << filter << "\"" << endl;
    QMap<QString, QString>::const_iterator it = files.begin();
    while (it != files.end()) {
        t << "# Begin Source File" << endl;
        t << "SOURCE=" << it.key() << endl;
        writeBuildstepForFile(t, it.key(), it.value());
        t << "# End Source File" << endl;
        t << endl;
        ++it;
    }
    t << "# End Group" << endl;
    t << endl;

    return true;
}

bool DspMakefileGenerator::writeBuildstepForFile(QTextStream &t, const QString &file, const QString &listName)
{

    if (!mergedProjects.count()) {
        t << writeBuildstepForFileForConfig(file, listName, this);
        return true;
    }

    //only add special build rules when needed

    QStringList specialBuilds;
    int i = 0;
    for (i = 0; i < mergedProjects.count(); ++i)
        specialBuilds += writeBuildstepForFileForConfig(file, listName, mergedProjects.at(i));

    // no specail build just return
    if (specialBuilds.join("").isEmpty())
        return true;

    for (i = 0; i < mergedProjects.count(); ++i) {
        if (i == 0)
            t << "!IF";
        else
            t << "!ELSEIF";
        t << "\"$(CFG)\" == \"" << configName(mergedProjects.at(i)) << "\"" << endl;
        t << endl;
        t << specialBuilds.at(i);
        t << endl;
    }

    t << "!ENDIF" << endl;

    return true;
}

bool DspMakefileGenerator::writeDspConfig(QTextStream &t, DspMakefileGenerator *config)
{

    bool isDebug = config->project->isActiveConfig("debug");
    bool staticLibTarget = config->var("MSVCDSP_DSPTYPE") == "0x0104";

    QString outDir = Option::fixPathToTargetOS(config->project->first("DESTDIR"));
    while (outDir.endsWith(Option::dir_sep))
        outDir.chop(1);

    QString intDir = config->project->first("OBJECTS_DIR");
    while (intDir.endsWith(Option::dir_sep))
        intDir.chop(1);

    t << "# PROP BASE Use_MFC 0" << endl;
    t << "# PROP BASE Use_Debug_Libraries " << (isDebug ? "1" : "0") << endl;
    t << "# PROP BASE Output_Dir \"" << outDir << "\"" << endl;
    t << "# PROP BASE Intermediate_Dir \"" << intDir << "\"" << endl;
    t << "# PROP BASE Target_Dir \"\"" << endl;
    t << "# PROP Use_MFC 0" << endl;
    t << "# PROP Use_Debug_Libraries " << (isDebug ? "1" : "0") << endl;

    t << "# PROP Output_Dir \"" << outDir << "\"" << endl;
    t << "# PROP Intermediate_Dir \"" << intDir << "\"" << endl;
    if (config->project->isActiveConfig("dll") || config->project->isActiveConfig("plugin"))
        t << "# PROP Ignore_Export_Lib 1" << endl;
    t << "# PROP Target_Dir \"\"" << endl;
    t << "# ADD CPP " << config->var("MSVCDSP_INCPATH") << " /c /FD " << config->var("QMAKE_CXXFLAGS") << " " << config->var("MSVCDSP_DEFINES") << " " << config->var("PRECOMPILED_FLAGS") << endl;
    t << "# ADD MTL /nologo /mktyplib203 /win32 /D " << (isDebug ? "\"_DEBUG\"" : "\"NDEBUG\"") << endl;
    t << "# ADD RSC /l 0x409 /d " << (isDebug ? "\"_DEBUG\"" : "\"NDEBUG\"") << endl;
    t << "# ADD BSC32 /nologo" << endl;
    if (staticLibTarget) {
        t << "LIB32=" << config->var("QMAKE_LIB") << endl;
        t << "# ADD LIB32 " << config->var("MSVCDSP_TARGET") << " " << config->var("PRECOMPILED_OBJECT") << endl;
    } else {
        t << "LINK32=" << config->var("QMAKE_LINK") << endl;
        t << "# ADD LINK32 " << config->var("MSVCDSP_LFLAGS") << " " << config->var("MSVCDSP_LIBS") << " " << config->var("MSVCDSP_TARGET") << " " << config->var("PRECOMPILED_OBJECT") << endl;
    }

    if (!config->project->variables()["MSVCDSP_POST_LINK"].isEmpty())
        t << config->project->variables()["MSVCDSP_POST_LINK"].first();

    return true;
}

QString DspMakefileGenerator::writeBuildstepForFileForConfig(const QString &file, const QString &listName, DspMakefileGenerator *config)
{
    QString ret;
    QTextStream t(&ret);

    // exclude from build
    if (!config->project->variables()[listName].contains(file)) {
        t << "# PROP Exclude_From_Build 1" << endl;
        return ret;
    }

    if (config->usePCH) {
        if (file.endsWith(".c")) {
            t << "# SUBTRACT CPP /FI\"" << config->namePCH << "\" /Yu\"" << config->namePCH << "\" /Fp" << endl;
            return ret;
        } else if (config->precompH.endsWith(file)) {
            // ### dependency list quickly becomes too long for VS to grok...
            t << "USERDEP_" << file << "=" << config->valGlue(config->findDependencies(config->precompH), "\"", "\"\t\"", "\"") << endl;
            t << endl;
            t << "# Begin Custom Build - Creating precompiled header from " << file << "..." << endl;
            t << "InputPath=.\\" << file << endl << endl;
            t << config->precompPch + ": $(SOURCE) \"$(IntDir)\" \"$(OUTDIR)\"" << endl;
            t << "\t" << config->var("QMAKE_CC") << " /TP /W3 /FD /c /Yc /Fp" << config->precompPch << " /Fo" << config->precompObj << " /Fd\"$(IntDir)\\\\\" " << file << " ";
            t << config->var("MSVCDSP_INCPATH") << " " << config->var("MSVCDSP_DEFINES") << " " << config->var("QMAKE_CXXFLAGS") << endl;
            t << "# End Custom Build" << endl << endl;
            return ret;
        }
    }

    QString fileBase = QFileInfo(file).completeBaseName();

    bool hasBuiltin = config->hasBuiltinCompiler(file);
    BuildStep allSteps;

    if (!config->swappedBuildSteps.contains(file)) {
        QStringList compilers = config->project->variables()["QMAKE_EXTRA_COMPILERS"];
        for (int i = 0; i < compilers.count(); ++i) {
            QString compiler = compilers.at(i);
            if (config->project->variables()[compiler + ".input"].isEmpty())
                continue;
            QString input = config->project->variables()[compiler + ".input"].first();
            QStringList inputList = config->project->variables()[input];
            if (!inputList.contains(file))
                continue;

            QStringList compilerCommands = config->project->variables()[compiler + ".commands"];
            QStringList compilerOutput = config->project->variables()[compiler + ".output"];
            if (compilerCommands.isEmpty() || compilerOutput.isEmpty())
                continue;

            QStringList compilerName = config->project->variables()[compiler + ".name"];
            if (compilerName.isEmpty())
                compilerName << compiler;
            QStringList compilerDepends = config->project->variables()[compiler + ".depends"];
            QString compilerDependsCommand = config->project->variables()[compiler + ".depend_command"].join(" ");
            if (!compilerDependsCommand.isEmpty()) {
                QString argv0 = Option::fixPathToLocalOS(compilerDependsCommand.split(' ').first());
                if (!config->exists(argv0))
                compilerDependsCommand = QString();
            }
            QStringList compilerConfig = config->project->variables()[compiler + ".CONFIG"];

            if (!config->verifyExtraCompiler(compiler, file))
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

            BuildStep step;
            for (int i2 = 0; i2 < compilerDepends.count(); ++i2) {
                QString dependency = compilerDepends.at(i2);
                dependency.replace("${QMAKE_FILE_BASE}", fileBase);
                dependency.replace('/', '\\');
                if (!step.deps.contains(dependency, Qt::CaseInsensitive))
                    step.deps << dependency;
            }
            // depends command
            if (!compilerDependsCommand.isEmpty() && config->doDepends()) {
                char buff[256];
                QString dep_cmd = config->replaceExtraCompilerVariables(compilerDependsCommand, file,
                                                                        fileOut, true);
                dep_cmd = Option::fixPathToLocalOS(dep_cmd);
                if(FILE *proc = QT_POPEN(dep_cmd.toLatin1().constData(), "r")) {
                    QString indeps;
                    while(!feof(proc)) {
                        int read_in = (int)fread(buff, 1, 255, proc);
                        if(!read_in)
                            break;
                        indeps += QByteArray(buff, read_in);
                    }
                    fclose(proc);
                    if(!indeps.isEmpty())
                        step.deps += config->fileFixify(indeps.replace('\n', ' ').simplified().split(' '));
                }
            }


            QString mappedFile;
            if (hasBuiltin) {
                mappedFile = fileOut;
                fileOut = fileIn;
                fileIn = file;
            }

            step.buildStep += " \\\n\t";
            QString command(compilerCommands.join(" "));
            // Might be a macro, and not a valid filename, so the replaceExtraCompilerVariables() would eat it
            command.replace("${QMAKE_FILE_IN}", fileIn);
            command.replace("${QMAKE_FILE_BASE}", fileBase);
            command.replace("${QMAKE_FILE_OUT}", fileOut);

            command = config->replaceExtraCompilerVariables(command, fileIn, fileOut, false);

            step.buildName = compilerName.first();
            step.buildStep += command;
            step.buildOutputs += fileOut;

            if (hasBuiltin) {
                step.deps << fileIn;
                config->swappedBuildSteps[mappedFile] = step;
            } else {
                allSteps << step;
            }
        }
    } else {
        allSteps << config->swappedBuildSteps.value(file);
    }

    if (allSteps.buildStep.isEmpty())
        return ret;

    int i;
    QStringList dependencyList;
    // remove dependencies that are also output
    for (i = 0; i < 1; ++i) {
        QStringList buildOutput(allSteps.buildOutputs.at(i));

        for (int i2 = 0; i2 < allSteps.deps.count(); ++i2) {
            QString dependency = allSteps.deps.at(i2);
            if (!buildOutput.contains(dependency) && !dependencyList.contains(dependency))
                dependencyList << dependency;
        }
    }
    QString allDependencies = config->valGlue(dependencyList, "\"", "\"\t\"", "\"");
    t << "USERDEP_" << file << "=" << allDependencies << endl;
    t << "# PROP Ignore_Default_Tool 1" << endl;
    t << "# Begin Custom Build - Running " << allSteps.buildName << " on " << file << endl;
    t << "InputPath=" << file << endl;
    t << "BuildCmds= " << allSteps.buildStep << endl;
    for (i = 0; i < allSteps.buildOutputs.count(); ++i) {
        t << "\"" << allSteps.buildOutputs.at(i)
          << "\": $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n\t$(BuildCmds)\n";
    }
    t << endl;
    t << "# End Custom Build" << endl;

    return ret;
}
