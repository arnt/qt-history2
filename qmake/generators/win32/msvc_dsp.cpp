/****************************************************************************
**
** Implementation of DspMakefileGenerator class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "msvc_dsp.h"
#include "option.h"
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>

DspMakefileGenerator::DspMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(false)
{

}

bool
DspMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        /* for now just dump, I need to generated an empty dsp or something.. */
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").latin1());
        return true;
    }

    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
        return writeDspParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
}

bool
DspMakefileGenerator::writeDspParts(QTextStream &t)
{
    QString dspfile;
    if(!project->variables()["DSP_TEMPLATE"].isEmpty()) {
        dspfile = project->first("DSP_TEMPLATE");
    } else {
        dspfile = project->first("MSVCDSP_TEMPLATE");
    }
    if (dspfile.startsWith("\"") && dspfile.endsWith("\""))
	dspfile = dspfile.mid(1, dspfile.length() - 2);
    QString dspfile_loc = findTemplate(dspfile);

    QFile file(dspfile_loc);
    if(!file.open(IO_ReadOnly)) {
        fprintf(stderr, "Cannot open dsp file: %s\n", dspfile.latin1());
        return false;
    }
    QTextStream dsp(&file);

    QString platform = "Win32";
    if(!project->variables()["QMAKE_PLATFORM"].isEmpty())
        platform = varGlue("QMAKE_PLATFORM", "", " ", "");

    QString mocargs;
    // defines
    mocargs = varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") + varGlue("DEFINES"," -D"," -D","") +
              varGlue("QMAKE_COMPILER_DEFINES"," -D"," -D","");
    // includes
    mocargs += " -I" + specdir();
    if(!project->isActiveConfig("no_include_pwd")) {
        QString pwd = fileFixify(QDir::currentPath());
        if(pwd.isEmpty())
            pwd = ".";
        mocargs += " -I" + pwd;
    }
    mocargs += varGlue("INCLUDEPATH"," -I", " -I", "");

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    QString namePCH = QFileInfo(precompH).fileName();
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
        project->variables()["PRECOMPILED_FLAGS_REL"]  = "/Yu\"" + namePCH + "\" /FI\"" + namePCH + "\" ";
        project->variables()["PRECOMPILED_FLAGS_DEB"]  = "/Yu\"" + namePCH + "\" /FI\"" + namePCH + "\" ";
        // Return to variable pool
        project->variables()["PRECOMPILED_OBJECT"] = precompObj;
        project->variables()["PRECOMPILED_PCH"]    = precompPch;
    }
    int rep;
    QString line;
    while (!dsp.eof()) {
        line = dsp.readLine();
        while((rep = line.indexOf(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
            QString torep = line.mid(rep, line.indexOf(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
            QString variable = torep.right(torep.length()-2);

            t << line.left(rep); //output the left side
            line = line.right(line.length() - (rep + torep.length())); //now past the variable
            if(variable == "MSVCDSP_SOURCES") {
                if(project->variables()["SOURCES"].isEmpty())
                    continue;

                QStringList list = project->variables()["SOURCES"] + project->variables()["DEF_FILE"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    beginGroupForFile((*it), t);
                    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;
                    if(usePCH && (*it).endsWith(".c"))
                        t << "# SUBTRACT CPP /FI\"" << namePCH << "\" /Yu\"" << namePCH << "\" /Fp" << endl;
                    t << "# End Source File" << endl;
                }
                endGroups(t);
            } else if(variable == "MSVCDSP_IMAGES") {
                if(project->variables()["IMAGES"].isEmpty())
                    continue;
                t << "# Begin Source File\n\nSOURCE=" << project->first("QMAKE_IMAGE_COLLECTION") << endl;
                t << "# End Source File" << endl;
            } else if(variable == "MSVCDSP_HEADERS") {
                if(project->variables()["HEADERS"].isEmpty())
                    continue;

                QStringList list = project->variables()["HEADERS"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
//                    beginGroupForFile((*it), t);
                    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl << endl;
                    QString compilePCH;
                    QStringList customDependencies;
                    QString createMOC;
                    QString buildCmdsR, buildCmdsD;
                    QString buildCmds = "\nBuildCmds= \\\n";
                    // Create unique baseID
                    QString base = (*it);
                    {
                        base.replace(QRegExp("\\..*$"), "").toUpper();
                        base.replace(QRegExp("[^a-zA-Z]"), "_");
                    }
                    if (usePCH && precompH.endsWith(*it)) {
                        QString basicBuildCmd = QString("\tcl.exe /TP /W3 /FD /c /D \"WIN32\" /Yc /Fp\"%1\" /Fo\"%2\" %3 %4 %5 %6 %7 %8 %9 /D \"")
                                                        .arg(precompPch)
                                                        .arg(precompObj)
                                                        .arg(var("MSVCDSP_INCPATH"))
                                                        .arg(var("MSVCDSP_DEFINES"))
                                                        .arg(var("MSVCDSP_CXXFLAGS"));
                        buildCmdsR = basicBuildCmd
                                            .arg("/D \"NDEBUG\"")
                                            .arg(var("MSVCDSP_CXXFLAGS_REL"))
                                            .arg(var("MSVCDSP_MTDEF"))
                                            .arg(var("MSVCDSP_RELDEFS"));
                        buildCmdsD = basicBuildCmd
                                            .arg("/D \"_DEBUG\"")
                                            .arg(var("MSVCDSP_CXXFLAGS_DEB"))
                                            .arg(var("MSVCDSP_MTDEFD"))
                                            .arg(var("MSVCDSP_DEBDEFS"));
                        if (project->first("TEMPLATE") == "vcapp") {        // App
                            buildCmdsR += var("MSVCDSP_WINCONDEF");
                            buildCmdsD += var("MSVCDSP_WINCONDEF");
                        } else if (project->isActiveConfig("dll")) {        // Dll
                            buildCmdsR += "_WINDOWS\" /D \"_USRDLL";
                            buildCmdsD += "_WINDOWS\" /D \"_USRDLL";
                        } else {                                        // Lib
                            buildCmdsR += "_LIB";
                            buildCmdsD += "_LIB";
                        }
                        buildCmdsR += "\" /Fd\"$(IntDir)\\\\\" " + (*it) + " \\\n";
                        buildCmdsD += "\" /Fd\"$(IntDir)\\\\\" " + (*it) + " \\\n";

                        compilePCH = precompPch + " : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n   $(BuildCmds)\n\n";

                        QStringList &tmp = findDependencies(precompH);
                        if(!tmp.isEmpty()) // Got Deps for PCH
                            customDependencies += tmp;
                    }

                    QString mocFile = QMakeSourceFileInfo::mocFile((*it));
                    if (project->isActiveConfig("moc") && !mocFile.isEmpty()) {
                        QString mocpath = var("QMAKE_MOC");
                        mocpath = mocpath.replace(QRegExp("\\..*$"), "") + " ";

                        buildCmds += "\t" + mocpath + (*it)  + mocargs + " -o " + mocFile + " \\\n";
                        createMOC  = "\"" + mocFile +        "\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n   $(BuildCmds)\n\n";
                        customDependencies += "\"$(QTDIR)\\bin\\moc.exe\"";
                    }
                    if (!createMOC.isEmpty() || !compilePCH.isEmpty()) {
                        bool doMOC = !createMOC.isEmpty();
                        bool doPCH = !compilePCH.isEmpty();
                        QString build = "\n\n# Begin Custom Build - "+
                                        QString(doMOC?"Moc'ing ":"") +
                                        QString((doMOC&&doPCH)?" and ":"") +
                                        QString(doPCH?"Creating PCH cpp from ":"") +
                                        (*it) + "...\nInputPath=.\\" + (*it) + "\n\n" +
                                        buildCmds + "%1\n" +
                                        createMOC +
                                        compilePCH +
                                        "# End Custom Build\n\n";

                        t << "USERDEP_" << base << "=" << valGlue(customDependencies, "\"", "\" \"", "\"") << endl << endl;
                        t << "!IF  \"$(CFG)\" == \""     << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build.arg(buildCmdsR)
                          << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\""   << build.arg(buildCmdsD)
                          << "!ENDIF " << endl << endl;
                    }
                    t << "# End Source File" << endl;
                }
//                endGroups(t);
            } else if(variable == "MSVCDSP_FORMSOURCES" || variable == "MSVCDSP_FORMHEADERS") {
                if(project->variables()["FORMS"].isEmpty())
                    continue;

                QString uiSourcesDir;
                QString uiHeadersDir;
                if(!project->variables()["UI_DIR"].isEmpty()) {
                    uiSourcesDir = project->first("UI_DIR");
                    uiHeadersDir = project->first("UI_DIR");
                } else {
                    if(!project->variables()["UI_SOURCES_DIR"].isEmpty())
                        uiSourcesDir = project->first("UI_SOURCES_DIR");
                    else
                        uiSourcesDir = "";
                    if(!project->variables()["UI_HEADERS_DIR"].isEmpty())
                        uiHeadersDir = project->first("UI_HEADERS_DIR");
                    else
                        uiHeadersDir = "";
                }

                QStringList list = project->variables()["FORMS"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                QString ext = variable == "MSVCDSP_FORMSOURCES" ? ".cpp" : ".h";
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString base = (*it);
                    int dot = base.lastIndexOf(".");
                    base.replace(dot, base.length() - dot, ext);
                    QString fname = base;

                    int lbs = fname.lastIndexOf("\\");
                    QString fpath;
                    if(lbs != -1)
                        fpath = fname.left(lbs + 1);
                    fname = fname.right(fname.length() - lbs - 1);

                    if(ext == ".cpp" && !uiSourcesDir.isEmpty())
                        fname.prepend(uiSourcesDir);
                    else if(ext == ".h" && !uiHeadersDir.isEmpty())
                        fname.prepend(uiHeadersDir);
                    else
                        fname = base;
//                    beginGroupForFile(fname, t);
                    t << "# Begin Source File\n\nSOURCE=" << fname
                      << "\n# End Source File" << endl;
                }
//                endGroups(t);
            } else if(variable == "MSVCDSP_TRANSLATIONS") {
                if(project->variables()["TRANSLATIONS"].isEmpty())
                    continue;

                t << "# Begin Group \"Translations\"\n";
                t << "# Prop Default_Filter \"ts\"\n";

                QStringList list = project->variables()["TRANSLATIONS"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString sify = *it;
                    sify.replace('/', '\\');
                    QString base = (*it);
                    base.replace(QRegExp("\\..*$"), "").toUpper();
                    base.replace(QRegExp("[^a-zA-Z]"), "_");

//                    beginGroupForFile(sify, t);
                    t << "# Begin Source File\n\nSOURCE=" << sify << endl;
                    t << "\n# End Source File" << endl;
                }
//                endGroups(t);
                t << "\n# End Group\n";
            } else if(variable == "MSVCDSP_MOCSOURCES" && project->isActiveConfig("moc")) {
                QString mocpath = var("QMAKE_MOC");
                mocpath = mocpath.replace(QRegExp("\\..*$"), "") + " ";

                QStringList &objl = project->variables()["OBJMOC"],
                    &srcl = project->variables()["SRCMOC"];

                QStringList list;
                for(int index = 0; index < objl.count() && index < srcl.count(); ++index)
                {
                    list << srcl.at(index);
                }

                if(!list.isEmpty()) {
                    if(!project->isActiveConfig("flat"))
                        list.sort();
                    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                        t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;
                        t << "# End Source File" << endl;
                    }
                }

                list = project->variables()["SOURCES"];
                if(list.isEmpty())
                    continue;

                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    if(!QMakeSourceFileInfo::mocable(*it))
                        continue;
                    QString mocSource = *it;
                    QString mocTarget = QMakeSourceFileInfo::mocFile(*it);

                    if (mocTarget.endsWith(Option::cpp_moc_ext)) {
//                        beginGroupForFile((*it), t);
                        t << "# Begin Source File\n\nSOURCE=" << mocTarget << endl;
                        QString base = mocSource;
                        base.replace(QRegExp("\\..*$"), "").toUpper();
                        base.replace(QRegExp("[^a-zA-Z]"), "_");

                        QString build = "\n\n# Begin Custom Build - Moc'ing " + mocSource +
                                        "...\n" "InputPath=.\\" + mocSource + "\n\n" "\"" + mocTarget + "\""
                                        " : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
                                        "\t" + mocpath + mocSource + mocargs + " -o " +
                                        mocTarget + "\n\n" "# End Custom Build\n\n";

                        t << "USERDEP_" << base << "=\".\\" << mocSource << "\" \"$(QTDIR)\\bin\\moc.exe\"" << endl << endl;

                        t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
                          << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\""
                          << build << "!ENDIF " << endl << endl;
                        t << "# End Source File" << endl;
                    }
                }
//                endGroups(t);
            } else if(variable == "MSVCDSP_PICTURES") {
                if(project->variables()["IMAGES"].isEmpty())
                    continue;

                t << "# Begin Group \"Images\"\n"
                  << "# Prop Default_Filter \"png jpeg bmp xpm\"\n";

                QStringList list = project->variables()["IMAGES"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                QStringList::Iterator it;

                // dump the image list to a file UIC can read.
                QFile f("images.tmp");
                f.open(IO_WriteOnly);
                QTextStream ts(&f);
                for(it = list.begin(); it != list.end(); ++it)
                    ts << " " << *it;
                f.close();

                // create an output step for images not more than once
                bool imagesBuildDone = false;
                for(it = list.begin(); it != list.end(); ++it) {
//                    beginGroupForFile((*it), t);
                    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;

                    QString base = (*it);
                    QString uicpath = var("QMAKE_UIC");
                    uicpath = uicpath.replace(QRegExp("\\..*$"), "") + " ";

                    if(!imagesBuildDone) {
                        imagesBuildDone = true;
                        QString build = "\n\n# Begin Custom Build - Creating image collection...\n"
                            "InputPath=.\\" + base + "\n\n";

                        build += "\"" + project->first("QMAKE_IMAGE_COLLECTION") + "\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n";
                        build += "\t" + uicpath + "-embed " + project->first("QMAKE_ORIG_TARGET") + " -f images.tmp -o "
                                      + project->first("QMAKE_IMAGE_COLLECTION") + "\n\n";
                        build.append("# End Custom Build\n\n");

                        t << "USERDEP_" << base << "=";
                        QStringList::Iterator it2 = list.begin();
                        while (it2 != list.end()) {
                            t << "\"" << (*it2) << "\"";
                            it2++;
                            if(it2 != list.end())
                                t << "\\\n";
                        }
                        t << endl << endl;

                        t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
                          << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\"" << build
                          << "!ENDIF \n\n" << endl;
                    }

                    t << "# End Source File" << endl;
                }
//                endGroups(t);
                t << "\n# End Group\n";
	    } else if(variable == "MSVCDSP_COMPILEROUTS") {
                if(project->variables()["QMAKE_EXTRA_COMPILERS"].isEmpty())
                    continue;
		
		const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
		for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
		    if(project->variables().contains((*it) + ".variable_out") ||
		       project->variables()[(*it) + ".CONFIG"].indexOf("no_link") != -1)
			continue;

		    QString tmp_out = project->variables()[(*it) + ".output"].first();
                    if(tmp_out.isEmpty())
                        continue;

                    if(project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1) {
                        QString deps;
                        const QStringList &tmp = project->variables()[(*it) + ".input"];
                        for(QStringList::ConstIterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
                            const QStringList &inputs = project->variables()[(*it2)];
                            for(QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); ++input) 
                                deps += " " + Option::fixPathToTargetOS((*input), false);
                        }
                        t << "# Begin Group \"" << (*it) << "\"\n"
                          <<  "# Begin Source File\n\nSOURCE=" << tmp_out << endl
                          << "USERDEP_" << tmp_out.section('.', 0, 0) << "=\"" << deps << "\"" << endl
                          << "# End Source File" << endl
                          << "\n# End Group\n";
                        continue;
                    }

		    int output_count = 0;
		    QStringList &tmp = project->variables()[(*it) + ".input"];
		    for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
			QStringList &inputs = project->variables()[(*it2)];
			for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); 
			    ++input) {
			    QString in = (*input);
			    QString out = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
			    QString deps = replaceExtraCompilerVariables(in, (*input), out);
			    if(!output_count) 
				t << "# Begin Group \"" << (*it2) << "\"\n";
			    t <<  "# Begin Source File\n\nSOURCE=" << out << endl
			      << "USERDEP_" << out.section('.', 0, 0) << "=\"" << deps << "\"" << endl
			      << "# End Source File" << endl;
			    output_count++;
			}
		    }
		    if(output_count)
			t << "\n# End Group\n";
		}
	    } else if(variable == "MSVCDSP_COMPILERSOURCES") {
                if(project->variables()["QMAKE_EXTRA_COMPILERS"].isEmpty())
                    continue;

		const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
		for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
		    int output_count = 0;
		    QString tmp_out = project->variables()[(*it) + ".output"].first();
		    QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
		    QString tmp_cmd_name = project->variables()[(*it) + ".name"].join(" ");
		    QString tmp_dep = project->variables()[(*it) + ".depends"].join(" ");
		    QString tmp_dep_cmd = project->variables()[(*it) + ".depend_command"].join(" ");
		    //QStringList &vars = project->variables()[(*it) + ".variables"];
		    if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
			continue;
                    if(project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1) {
                        if(tmp_out.indexOf("$") != -1) {
                            warn_msg(WarnLogic, "QMAKE_EXTRA_COMPILERS(%s) with combine has variable output.",
                                     (*it).latin1());
                            continue;
                        }
                        QString inputs;
                        const QStringList &tmp = project->variables()[(*it) + ".input"];
                        for(QStringList::ConstIterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
                            const QStringList &tmp2 = project->variables()[(*it2)];
                            for(QStringList::ConstIterator input = tmp2.begin(); input != tmp2.end(); ++input) 
                                inputs += " " + Option::fixPathToTargetOS((*input), false);
                        }
                        QString cmd = replaceExtraCompilerVariables(tmp_cmd, QString::null, tmp_out), deps;
                        if(!tmp_dep.isEmpty())
                            deps = " " + tmp_dep;
                        if(!tmp_dep_cmd.isEmpty() && doDepends()) {
                            char buff[256];
                            QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, QString::null, tmp_out);
                            if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
                                while(!feof(proc)) {
                                    int read_in = fread(buff, 1, 255, proc);
                                    if(!read_in)
                                        break;
                                    int l = 0;
                                    for(int i = 0; i < read_in; i++) {
                                        if(buff[i] == '\n' || buff[i] == ' ') {
                                            deps += " " + QByteArray(buff+l, (i - l) + 1);
                                            l = i;
                                        }
                                    }
                                }
                                fclose(proc);
                            }
                        }
                        deps = replaceExtraCompilerVariables(deps, QString::null, tmp_out);
                        //need to do this!!!
                        continue;
                    }
		    const QStringList &tmp = project->variables()[(*it) + ".input"];
		    for(QStringList::ConstIterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
			const QStringList &inputs = project->variables()[(*it2)];
			for(QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); 
			    ++input) {
			    QString in = Option::fixPathToTargetOS((*input), false);
			    QString out = replaceExtraCompilerVariables(tmp_out, (*input), 
									QString::null);
			    QString cmd = replaceExtraCompilerVariables(tmp_cmd, (*input), out), deps;
			    if(!tmp_dep.isEmpty())
				deps = " " + tmp_dep;
			    if(!tmp_dep_cmd.isEmpty()) {
				char buff[256];
				QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, 
										(*input), out);
				if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
				    while(!feof(proc)) {
					int read_in = fread(buff, 1, 255, proc);
					if(!read_in)
					    break;
					int l = 0;
					for(int i = 0; i < read_in; i++) {
					    if(buff[i] == '\n' || buff[i] == ' ') {
						deps += " " + QByteArray(buff+l, (i - l) + 1);
						l = i;
					    }
					}
				    }
				    fclose(proc);
				}
			    }
			    deps = replaceExtraCompilerVariables(deps, (*input), out);
			    if(!output_count) 
				t << "# Begin Group \"" << (*it2) << "\"\n";
			    t <<  "# Begin Source File\n\nSOURCE=" << in << endl
			      << "USERDEP_" << in.section('.', 0, 0) << "=\"" << deps << "\"" << endl;
			    QString cmd_name;
			    if(!tmp_cmd_name.isEmpty()) {
				cmd_name = replaceExtraCompilerVariables(tmp_cmd_name, (*input), out);
			    } else {
				int space = cmd.indexOf(' ');
				if(space != -1)
				    cmd_name = cmd.left(space);
				else
				    cmd_name = cmd;
				if((cmd_name[0] == '\'' || cmd_name[0] == '"') &&
				   cmd_name[0] == cmd_name[cmd_name.length()-1])
				    cmd_name = cmd_name.mid(1,cmd_name.length()-2);
			    }
			    QString build = "\n\n# Begin Custom Build - " + cmd_name + "'ing " + in + "...\n"
					    "InputPath=" + in + "\n\n" 
					    "BuildCmds= \\\n\t" + cmd + " \\\n";
			    build.append("\n\"" + out + "\" "
					 " : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\""  "\n"
					 "\t$(BuildCmds)\n\n");
			    build.append("# End Custom Build\n\n");

			    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
			      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\"" << build
			      << "!ENDIF \n\n" << "# End Source File" << endl;
			    output_count++;
			}
		    }
		    if(output_count)
			t << "\n# End Group\n";
		}
            } else if(variable == "MSVCDSP_FORMS") {
                if(project->variables()["FORMS"].isEmpty())
                    continue;

                t << "# Begin Group \"Forms\"\n"
                  << "# Prop Default_Filter \"ui\"\n";

                QString uicpath = var("QMAKE_UIC");
                uicpath = uicpath.replace(QRegExp("\\..*$"), "") + " ";
                QString mocpath = var("QMAKE_MOC");
                mocpath = mocpath.replace(QRegExp("\\..*$"), "") + " ";

                QStringList list = project->variables()["FORMS"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString base = (*it);
//                    beginGroupForFile(base, t);
                    t <<  "# Begin Source File\n\nSOURCE=" << base << endl;

                    QString fname = base;
                    fname.replace(".ui", "");
                    int lbs = fname.lastIndexOf("\\");
                    QString fpath;
                    if(lbs != -1)
                        fpath = fname.left(lbs + 1);
                    fname = fname.right(fname.length() - lbs - 1);

                    QString mocFile;
                    if(!project->variables()["MOC_DIR"].isEmpty())
                        mocFile = project->first("MOC_DIR");
                    else
                        mocFile = fpath;

                    QString uiSourcesDir;
                    QString uiHeadersDir;
                    if(!project->variables()["UI_DIR"].isEmpty()) {
                        uiSourcesDir = project->first("UI_DIR");
                        uiHeadersDir = project->first("UI_DIR");
                    } else {
                        if(!project->variables()["UI_SOURCES_DIR"].isEmpty())
                            uiSourcesDir = project->first("UI_SOURCES_DIR");
                        else
                            uiSourcesDir = fpath;
                        if(!project->variables()["UI_HEADERS_DIR"].isEmpty())
                            uiHeadersDir = project->first("UI_HEADERS_DIR");
                        else
                            uiHeadersDir = fpath;
                    }

                    t << "USERDEP_" << base << "=\"$(QTDIR)\\bin\\moc.exe\" \"$(QTDIR)\\bin\\uic.exe\"" << endl << endl;

                    QString build = "\n\n# Begin Custom Build - Uic'ing " + base + "...\n"
                        "InputPath=.\\" + base + "\n\n" "BuildCmds= \\\n\t" + uicpath + base +
                                    " -o " + uiHeadersDir + fname + ".h \\\n" "\t" + uicpath  + base +
                                    " -i " + fname + ".h -o " + uiSourcesDir + fname + ".cpp \\\n"
                                    "\t" + mocpath + " " + uiHeadersDir +
                                    fname + ".h " + mocargs + " -o " + mocFile + Option::h_moc_mod + fname + Option::h_moc_ext + " \\\n";

                    build.append("\n\"" + uiHeadersDir + fname + ".h\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\""  "\n"
                                 "\t$(BuildCmds)\n\n"
                                 "\"" + uiSourcesDir + fname + ".cpp\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
                                 "\t$(BuildCmds)\n\n"
                                 "\"" + mocFile + Option::h_moc_mod + fname + Option::h_moc_ext + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
                                 "\t$(BuildCmds)\n\n");

                    build.append("# End Custom Build\n\n");

                    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
                      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\"" << build
                      << "!ENDIF \n\n" << "# End Source File" << endl;
                }
//                endGroups(t);
                t << "\n# End Group\n";
            } else if(variable == "MSVCDSP_LEXSOURCES") {
                if(project->variables()["LEXSOURCES"].isEmpty())
                    continue;

                t << "# Begin Group \"Lexables\"\n"
                  << "# Prop Default_Filter \"l\"\n";

                QString lexpath = var("QMAKE_LEX") + varGlue("QMAKE_LEXFLAGS", " ", " ", "") + " ";

                QStringList list = project->variables()["LEXSOURCES"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString fname = (*it);
//                    beginGroupForFile(fname, t);
                    t <<  "# Begin Source File\n\nSOURCE=" << fname << endl;
                    fname.replace(".l", Option::lex_mod + Option::cpp_ext.first());

                    QString build = "\n\n# Begin Custom Build - Lex'ing " + (*it) + "...\n"
                        "InputPath=.\\" + (*it) + "\n\n"
                                    "\"" + fname + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
                                    "\t" + lexpath + (*it) + "\\\n"
                                    "\tdel " + fname + "\\\n"
                                    "\tcopy lex.yy.c " + fname + "\n\n" +
                                    "# End Custom Build\n\n";
                    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
                      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\"" << build
                      << "!ENDIF \n\n" << build

                      << "# End Source File" << endl;
                }
//                endGroups(t);
                t << "\n# End Group\n";
            } else if(variable == "MSVCDSP_YACCSOURCES") {
                if(project->variables()["YACCSOURCES"].isEmpty())
                    continue;

                t << "# Begin Group \"Yaccables\"\n"
                  << "# Prop Default_Filter \"y\"\n";

                QString yaccpath = var("QMAKE_YACC") + varGlue("QMAKE_YACCFLAGS", " ", " ", "") + " ";

                QStringList list = project->variables()["YACCSOURCES"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString fname = (*it);
//                    beginGroupForFile(fname, t);
                    t <<  "# Begin Source File\n\nSOURCE=" << fname << endl;
                    fname.replace(".y", Option::yacc_mod);

                    QString build = "\n\n# Begin Custom Build - Yacc'ing " + (*it) + "...\n"
                        "InputPath=.\\" + (*it) + "\n\n"
                                    "\"" + fname + Option::cpp_ext.first() + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
                                    "\t" + yaccpath + (*it) + "\\\n"
                                    "\tdel " + fname + Option::h_ext.first() + "\\\n"
                                    "\tmove y.tab.h " + fname + Option::h_ext.first() + "\n\n" +
                                    "\tdel " + fname + Option::cpp_ext.first() + "\\\n"
                                    "\tmove y.tab.c " + fname + Option::cpp_ext.first() + "\n\n" +
                                    "# End Custom Build\n\n";

                    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Release\"" << build
                      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - " << platform << " Debug\"" << build
                      << "!ENDIF \n\n"
                      << "# End Source File" << endl;
                }
//                endGroups(t);
                t << "\n# End Group\n";
            } else if(variable == "MSVCDSP_CONFIGMODE") {
                if(project->isActiveConfig("debug"))
                    t << "Debug";
                else
                    t << "Release";
            } else if(variable == "MSVCDSP_IDLSOURCES") {
                QStringList list = project->variables()["MSVCDSP_IDLSOURCES"];
                if(!project->isActiveConfig("flat"))
                    list.sort();
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    t << "# Begin Source File" << endl << endl;
                    t << "SOURCE=" << (*it) << endl;
                    t << "# PROP Exclude_From_Build 1" << endl;
                    t << "# End Source File" << endl << endl;
                }
            }
            else
                t << var(variable);
        }
        t << line << endl;
    }
    t << endl;
    file.close();
    return true;
}



void
DspMakefileGenerator::init()
{
    if(init_flag)
        return;
    QStringList::Iterator it;
    init_flag = true;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "vcapp")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");
    if(project->variables()["QMAKESPEC"].isEmpty())
        project->variables()["QMAKESPEC"].append(getenv("QMAKESPEC"));

    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    processDllConfig();

    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"][0];
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(".", "");
        project->variables()["MSVCDSP_VERSION"].append("/VERSION:" + major + "." + minor);
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

            if(!project->isActiveConfig("dll") && !project->isActiveConfig("plugin")) {
                project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
            }
        }
    }

    if(project->isActiveConfig("debug")) {
        if(!project->first("OBJECTS_DIR").isEmpty())
            project->variables()["MSVCDSP_OBJECTSDIRDEB"] = project->first("OBJECTS_DIR");
        else
            project->variables()["MSVCDSP_OBJECTSDIRDEB"] = "Debug";
        project->variables()["MSVCDSP_OBJECTSDIRREL"] = "Release";
        if(!project->first("DESTDIR").isEmpty())
            project->variables()["MSVCDSP_TARGETDIRDEB"] = project->first("DESTDIR");
        else
            project->variables()["MSVCDSP_TARGETDIRDEB"] = "Debug";
        project->variables()["MSVCDSP_TARGETDIRREL"] = "Release";
    } else {
        if(!project->first("OBJECTS_DIR").isEmpty())
            project->variables()["MSVCDSP_OBJECTSDIRREL"] = project->first("OBJECTS_DIR");
        else
            project->variables()["MSVCDSP_OBJECTSDIRREL"] = "Release";
        project->variables()["MSVCDSP_OBJECTSDIRDEB"] = "Debug";
        if(!project->first("DESTDIR").isEmpty())
            project->variables()["MSVCDSP_TARGETDIRREL"] = project->first("DESTDIR");
        else
            project->variables()["MSVCDSP_TARGETDIRREL"] = "Release";
        project->variables()["MSVCDSP_TARGETDIRDEB"] = "Debug";
    }

    fixTargetExt();
    project->variables()["MSVCDSP_VER"] = "6.00";

    QString msvcdsp_project;
    if(project->variables()["TARGET"].count())
        msvcdsp_project = project->variables()["TARGET"].first();

    QString targetfilename = project->variables()["TARGET"].first();
    project->variables()["TARGET"].first() += project->first("TARGET_EXT");
    if(project->isActiveConfig("moc"))
        setMocAware(true);

    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];

    processFileTagsVar();

    MakefileGenerator::init();
    if(msvcdsp_project.isEmpty())
        msvcdsp_project = Option::output.name();

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
        project->variables()["MSVCDSP_TEMPLATE"].append("win32app" + project->first("DSP_EXTENSION"));
        if(project->isActiveConfig("console")) {
            project->variables()["MSVCDSP_CONSOLE"].append("Console");
            project->variables()["MSVCDSP_WINCONDEF"].append("_CONSOLE");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0103");
        } else {
            project->variables()["MSVCDSP_CONSOLE"].clear();
            project->variables()["MSVCDSP_WINCONDEF"].append("_WINDOWS");
            project->variables()["MSVCDSP_DSPTYPE"].append("0x0101");
        }
    } else {
        if(project->isActiveConfig("dll")) {
            project->variables()["MSVCDSP_TEMPLATE"].append("win32dll" + project->first("DSP_EXTENSION"));
        } else {
            project->variables()["MSVCDSP_TEMPLATE"].append("win32lib" + project->first("DSP_EXTENSION"));
        }
    }

    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_WINDOWS"];

    findLibraries();
    processPrlFiles();
    processLibsVar();

    project->variables()["MSVCDSP_LFLAGS"] += project->variables()["QMAKE_LFLAGS"];
    if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
        project->variables()["MSVCDSP_LFLAGS"].append(varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\""));

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
    project->variables()["MSVCDSP_CXXFLAGS_DEB"] += msvcdsp_cxxflags_deb;

    project->variables()["MSVCDSP_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_DEFINES"].append(varGlue("PRL_EXPORT_DEFINES","/D ","" " /D ",""));

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
        project->variables()["MSVCDSP_INCPATH"].append("/I \"" + inc + "\"");
    }
    project->variables()["MSVCDSP_INCPATH"].append("/I \"" + specdir() + "\"");

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
            dest.replace("$(QTDIR)", getenv("QTDIR"));
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

    if(!project->variables()["SOURCES"].isEmpty() || !project->variables()["RC_FILE"].isEmpty())
        project->variables()["SOURCES"] += project->variables()["RC_FILE"];
    QStringList &list = project->variables()["FORMS"];
    for(QStringList::ConstIterator hit = list.begin(); hit != list.end(); ++hit) {
        if(QFile::exists(*hit + ".h"))
            project->variables()["SOURCES"].append(*hit + ".h");
    }
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "MSVCDSP_LIBS";
}


QString
DspMakefileGenerator::findTemplate(const QString &file)
{
    QString ret;
    if(!QFile::exists((ret = file)) &&
       !QFile::exists((ret = QString(Option::mkfile::qmakespec + "/" + file))) &&
       !QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/win32-msvc/" + file)) &&
       !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))))
        return "";
    return ret;
}


void
DspMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
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


void
DspMakefileGenerator::beginGroupForFile(QString file, QTextStream &t,
                                        const QString& filter)
{
    if(project->isActiveConfig("flat"))
        return;
    fileFixify(file, QDir::currentPath(), QDir::currentPath(), FileFixifyRelative);
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
        t << "# Begin Group \"" << (*dir_it) << "\"\n"
            << "# Prop Default_Filter \"" << filter << "\"\n";
    }
    currentGroup = file;
}


void
DspMakefileGenerator::endGroups(QTextStream &t)
{
    if(project->isActiveConfig("flat"))
        return;
    else if(currentGroup.isEmpty())
        return;

    QStringList dirs = currentGroup.split(Option::dir_sep);
    for(QStringList::Iterator dir_it = dirs.end(); dir_it != dirs.begin(); --dir_it) {
        t << "\n# End Group\n";
    }
    currentGroup = "";
}

bool
DspMakefileGenerator::openOutput(QFile &file, const QString &build) const
{
    QString outdir;
    if(!file.name().isEmpty()) {
        if(QDir::isRelativePath(file.name()))
            file.setName(Option::output_dir + file.name()); //pwd when qmake was run
        QFileInfo fi(file);
        if(fi.isDir())
            outdir = file.name() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.name().isEmpty())
        file.setName(outdir + project->first("TARGET") + project->first("DSP_EXTENSION"));
    if(QDir::isRelativePath(file.name())) {
        QString ofile;
        ofile = file.name();
        int slashfind = ofile.lastIndexOf('\\');
        if(slashfind == -1) {
            ofile = ofile.replace(QRegExp("-"), "_");
        } else {
            int hypenfind = ofile.indexOf('-', slashfind);
            while (hypenfind != -1 && slashfind < hypenfind) {
                ofile = ofile.replace(hypenfind, 1, "_");
                hypenfind = ofile.indexOf('-', hypenfind + 1);
            }
        }
        file.setName(Option::fixPathToLocalOS(QDir::currentPath() + Option::dir_sep + ofile));
    }
    return Win32MakefileGenerator::openOutput(file, build);
}
