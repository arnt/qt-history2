/****************************************************************************
**
** Implementation of VcprojGenerator class.
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

#include "msvc_vcproj.h"
#include "option.h"
#include "qtmd5.h" // SG's MD5 addon
#include "xmloutput.h"
#include <qdir.h>
#include <qregexp.h>
#include <qhash.h>
#include <quuid.h>
#include <stdlib.h>
#include <qsettings.h>

//#define DEBUG_SOLUTION_GEN
//#define DEBUG_PROJECT_GEN

// Registry keys for .NET version detection -------------------------
const char* _regNet2002                = "Microsoft\\VisualStudio\\7.0\\Setup\\VC\\ProductDir";
const char* _regNet2003                = "Microsoft\\VisualStudio\\7.1\\Setup\\VC\\ProductDir";

bool use_net2003_version()
{
#ifndef Q_OS_WIN32
    return false; // Always generate 7.0 versions on other platforms
#else
    // Only search for the version once
    static int current_version = -1;
    if(current_version!=-1)
        return (current_version==71);

    // Fallback to .NET 2002
    current_version = 70;

    // Get registry entries for both versions
    QSettings setting;
    QString path2002 = setting.readEntry(_regNet2002);
    QString path2003 = setting.readEntry(_regNet2003);

    if(path2002.isNull() || path2003.isNull()) {
        // Only have one MSVC, so use that one
        current_version = (path2003.isNull() ? 70 : 71);
    } else {
        // Have both, so figure out the current
        QString paths = getenv("PATH");
        QStringList pathlist = paths.toLower().split(";");

        path2003 = path2003.toLower();
        QStringList::iterator it;
        for(it=pathlist.begin(); it!=pathlist.end(); ++it) {
            if((*it).contains(path2003)) {
                current_version = 71;
            } else if((*it).contains(path2002)
                       && current_version == 71) {
                fprintf(stderr, "Both .NET 2002 & .NET 2003 directories for VC found in you PATH variable!\nFallback to .NET 2002 project generation");
                current_version = 70;
                break;
            }
        }
    }
    return (current_version==71);
#endif
};

// Flatfile Tags ----------------------------------------------------
const char* _slnHeader70        = "Microsoft Visual Studio Solution File, Format Version 7.00";
const char* _slnHeader71        = "Microsoft Visual Studio Solution File, Format Version 8.00";
                                  // The following UUID _may_ change for later servicepacks...
                                  // If so we need to search through the registry at
                                  // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\7.0\Projects
                                  // to find the subkey that contains a "PossibleProjectExtension"
                                  // containing "vcproj"...
                                  // Use the hardcoded value for now so projects generated on other
                                  // platforms are actually usable.
const char* _slnMSVCvcprojGUID  = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}";
const char* _slnProjectBeg        = "\nProject(\"";
const char* _slnProjectMid        = "\") = ";
const char* _slnProjectEnd        = "\nEndProject";
const char* _slnGlobalBeg        = "\nGlobal";
const char* _slnGlobalEnd        = "\nEndGlobal";
const char* _slnSolutionConf        = "\n\tGlobalSection(SolutionConfiguration) = preSolution"
                                  "\n\t\tConfigName.0 = Debug"
                                  "\n\t\tConfigName.1 = Release"
                                  "\n\tEndGlobalSection";
const char* _slnProjDepBeg        = "\n\tGlobalSection(ProjectDependencies) = postSolution";
const char* _slnProjDepEnd        = "\n\tEndGlobalSection";
const char* _slnProjConfBeg        = "\n\tGlobalSection(ProjectConfiguration) = postSolution";
const char* _slnProjRelConfTag1        = ".Release.ActiveCfg = Release|Win32";
const char* _slnProjRelConfTag2        = ".Release.Build.0 = Release|Win32";
const char* _slnProjDbgConfTag1        = ".Debug.ActiveCfg = Debug|Win32";
const char* _slnProjDbgConfTag2        = ".Debug.Build.0 = Debug|Win32";
const char* _slnProjConfEnd        = "\n\tEndGlobalSection";
const char* _slnExtSections        = "\n\tGlobalSection(ExtensibilityGlobals) = postSolution"
                                  "\n\tEndGlobalSection"
                                  "\n\tGlobalSection(ExtensibilityAddIns) = postSolution"
                                  "\n\tEndGlobalSection";
// ------------------------------------------------------------------

VcprojGenerator::VcprojGenerator() : Win32MakefileGenerator(), init_flag(false)
{
}
bool VcprojGenerator::writeMakefile(QTextStream &t)
{
    // Generate solution file
    if(project->first("TEMPLATE") == "vcsubdirs") {
        debug_msg(1, "Generator: MSVC.NET: Writing solution file");
        writeSubDirs(t);
        return true;
    } else 
    // Generate single configuration project file
    if((project->first("TEMPLATE") == "vcapp" ||
        project->first("TEMPLATE") == "vclib") &&
        !project->isActiveConfig("build_pass")) {
        debug_msg(1, "Generator: MSVC.NET: Writing single configuration project file");
        XmlOutput xmlOut(t);
        xmlOut << vcProject;
        return true;
    }
    return false;
}

bool VcprojGenerator::writeProjectMakefile()
{
    usePlatformDir();
    QTextStream t(&Option::output);

    // Check if all requirements are fullfilled
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").latin1());
        return true;
    }

    // Generate project file
    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
        if (!mergedProjects.count()) {
            warn_msg(WarnLogic, "Generator: MSVC.NET: no single configuration created, cannot output project!");
            return false;
        }

        debug_msg(1, "Generator: MSVC.NET: Writing project file");
        VCProject mergedProject;
        for (int i = 0; i < mergedProjects.count(); ++i)
            mergedProject.SingleProjects += mergedProjects.at(i)->vcProject;
        mergedProject.Name = mergedProjects.at(0)->vcProject.Name;
        mergedProject.Version = mergedProjects.at(0)->vcProject.Version;
        mergedProject.ProjectGUID = getProjectUUID();
        mergedProject.SccProjectName = mergedProjects.at(0)->vcProject.SccProjectName;
        mergedProject.SccLocalPath = mergedProjects.at(0)->vcProject.SccLocalPath;
        mergedProject.PlatformName = mergedProjects.at(0)->vcProject.PlatformName;

        XmlOutput xmlOut(t);
        xmlOut << mergedProject;
        return true;
    }

    return false;
}

struct VcsolutionDepend {
    QString uuid;
    QString vcprojFile, orig_target, target;
    ::target targetType;
    bool debugBuild;
    QStringList dependencies;
};

QUuid VcprojGenerator::getProjectUUID(const QString &filename)
{
    bool validUUID = true;

    // Read GUID from variable-space
    QUuid uuid = project->first("GUID");

    // If none, create one based on the MD5 of absolute project path
    if(uuid.isNull() || !filename.isNull()) {
        QString abspath = filename.isNull()?project->first("QMAKE_MAKEFILE"):filename;
        qtMD5(abspath.utf8(), (unsigned char*)(&uuid));
        validUUID = !uuid.isNull();
        uuid.data4[0] = (uuid.data4[0] & 0x3F) | 0x80; // UV_DCE variant
        uuid.data3 = (uuid.data3 & 0x0FFF) | (QUuid::Name<<12);
    }

    // If still not valid, generate new one, and suggest adding to .pro
    if(uuid.isNull() || !validUUID) {
        uuid = QUuid::createUuid();
        fprintf(stderr,
                "qmake couldn't create a GUID based on filepath, and we couldn't\nfind a valid GUID in the .pro file (Consider adding\n'GUID = %s'  to the .pro file)\n",
                uuid.toString().toUpper().latin1());
    }

    // Store GUID in variable-space
    project->values("GUID") = uuid.toString().toUpper();
    return uuid;
}

QUuid VcprojGenerator::increaseUUID(const QUuid &id)
{
    QUuid result(id);
    Q_LONG dataFirst = (result.data4[0] << 24) +
                       (result.data4[1] << 16) +
                       (result.data4[2] << 8) +
                        result.data4[3];
    Q_LONG dataLast =  (result.data4[4] << 24) +
                       (result.data4[5] << 16) +
                       (result.data4[6] <<  8) +
                        result.data4[7];

    if(!(dataLast++))
        dataFirst++;

    result.data4[0] = uchar((dataFirst >> 24) & 0xff);
    result.data4[1] = uchar((dataFirst >> 16) & 0xff);
    result.data4[2] = uchar((dataFirst >>  8) & 0xff);
    result.data4[3] = uchar(dataFirst        & 0xff);
    result.data4[4] = uchar((dataLast  >> 24) & 0xff);
    result.data4[5] = uchar((dataLast  >> 16) & 0xff);
    result.data4[6] = uchar((dataLast  >>  8) & 0xff);
    result.data4[7] = uchar(dataLast         & 0xff);
    return result;
}

void VcprojGenerator::writeSubDirs(QTextStream &t)
{
    if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return;
    }

    t << (use_net2003_version() ? _slnHeader71 : _slnHeader70);

    QHash<QString, VcsolutionDepend*> solution_depends;
    QList<VcsolutionDepend*> solution_cleanup;

    QStringList subdirs = project->variables()["SUBDIRS"];
    QString oldpwd = QDir::currentPath();


    for(int i = 0; i < subdirs.size(); ++i) {
        QString tmp = subdirs.at(i);
        QFileInfo fi(Option::fixPathToLocalOS(tmp, true));
        if(fi.exists()) {
            if(fi.isDir()) {
                QString profile = tmp;
                if(!profile.endsWith(Option::dir_sep))
                    profile += Option::dir_sep;
                profile += fi.baseName() + ".pro";
                subdirs.append(profile);
            } else {
                QMakeProject tmp_proj;
                QString dir = fi.path(), fn = fi.fileName();
                if(!dir.isEmpty()) {
                    if(!QDir::setCurrent(dir))
                        fprintf(stderr, "Cannot find directory: %s\n", dir.latin1());
                }
                if(tmp_proj.read(fn)) {
                    if(tmp_proj.first("TEMPLATE") == "vcsubdirs") {
                        subdirs += fileFixify(tmp_proj.variables()["SUBDIRS"]);
                    } else if(tmp_proj.first("TEMPLATE") == "vcapp" || tmp_proj.first("TEMPLATE") == "vclib") {
                        // Initialize a 'fake' project to get the correct variables
                        // and to be able to extract all the dependencies
                        VcprojGenerator tmp_vcproj;
                        tmp_vcproj.setNoIO(true);
                        tmp_vcproj.setProjectFile(&tmp_proj);
                        if(Option::debug_level) {
                            QMap<QString, QStringList> &vars = tmp_proj.variables();
                            for(QMap<QString, QStringList>::Iterator it = vars.begin();
                                it != vars.end(); ++it) {
                                if(it.key().left(1) != "." && !it.value().isEmpty())
                                    debug_msg(1, "%s: %s === %s", fn.latin1(), it.key().latin1(),
                                                it.value().join(" :: ").latin1());
                            }
                        }

                        // We assume project filename is [QMAKE_ORIG_TARGET].vcproj
                        QString vcproj = fixFilename(tmp_vcproj.project->first("QMAKE_ORIG_TARGET")) + project->first("VCPROJ_EXTENSION");

                        // If file doesn't exsist, then maybe the users configuration
                        // doesn't allow it to be created. Skip to next...
                        if(!QFile::exists(QDir::currentPath() + Option::dir_sep + vcproj)) {
                            warn_msg(WarnLogic, "Ignored (not found) '%s'", QString(QDir::currentPath() + Option::dir_sep + vcproj).latin1());
                            goto nextfile; // # Dirty!
                        }

                        VcsolutionDepend *newDep = new VcsolutionDepend;
                        newDep->vcprojFile = fileFixify(vcproj);
                        newDep->orig_target = tmp_proj.first("QMAKE_ORIG_TARGET");
                        newDep->target = tmp_proj.first("MSVCPROJ_TARGET").section(Option::dir_sep, -1);
                        newDep->targetType = tmp_vcproj.projectTarget;
                        newDep->debugBuild = tmp_proj.isActiveConfig("debug");
                        newDep->uuid = getProjectUUID(Option::fixPathToLocalOS(QDir::currentPath() + QDir::separator() + vcproj)).toString().toUpper();

                        // We want to store it as the .lib name.
                        if(newDep->target.endsWith(".dll"))
                            newDep->target = newDep->target.left(newDep->target.length()-3) + "lib";

                        // All projects using Forms are dependent on uic.exe
                        if(!tmp_proj.isEmpty("FORMS"))
                            newDep->dependencies << "uic.exe";

                        // Add all unknown libs to the deps
                        QStringList where("QMAKE_LIBS");
                        if(!tmp_proj.isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
                            where = tmp_proj.variables()["QMAKE_INTERNAL_PRL_LIBS"];
                        for(QStringList::iterator wit = where.begin();
                            wit != where.end(); ++wit) {
                            QStringList &l = tmp_proj.variables()[(*wit)];
                            for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                                QString opt = (*it);
                                if(!opt.startsWith("/") &&   // Not a switch
                                    opt != newDep->target && // Not self
                                    opt != "opengl32.lib" && // We don't care about these libs
                                    opt != "glu32.lib" &&    // to make depgen alittle faster
                                    opt != "kernel32.lib" &&
                                    opt != "user32.lib" &&
                                    opt != "gdi32.lib" &&
                                    opt != "comdlg32.lib" &&
                                    opt != "advapi32.lib" &&
                                    opt != "shell32.lib" &&
                                    opt != "ole32.lib" &&
                                    opt != "oleaut32.lib" &&
                                    opt != "uuid.lib" &&
                                    opt != "imm32.lib" &&
                                    opt != "winmm.lib" &&
                                    opt != "wsock32.lib" &&
                                    opt != "winspool.lib" &&
                                    opt != "delayimp.lib")
                                {
                                    newDep->dependencies << opt.section(Option::dir_sep, -1);
                                }
                            }
                        }
#ifdef DEBUG_SOLUTION_GEN
                        qDebug("Deps for %20s: [%s]", newDep->target.latin1(), newDep->dependencies.join(" :: ").latin1());
#endif
                        solution_cleanup.append(newDep);
                        solution_depends.insert(newDep->target, newDep);
                        t << _slnProjectBeg << _slnMSVCvcprojGUID << _slnProjectMid
                            << "\"" << newDep->orig_target << "\", \"" << newDep->vcprojFile
                            << "\", \"" << newDep->uuid << "\"";
                        t << _slnProjectEnd;
                    }
                }
nextfile:
                QDir::setCurrent(oldpwd);
            }
        }
    }
    t << _slnGlobalBeg;
    t << _slnSolutionConf;
    t << _slnProjDepBeg;

    // Figure out dependencies
    for(QList<VcsolutionDepend*>::Iterator it = solution_cleanup.begin(); it != solution_cleanup.end(); ++it) {
        if((*it)->targetType == StaticLib)
            continue; // Shortcut, Static libs are not dep.
        int cnt = 0;
        for(QStringList::iterator dit = (*it)->dependencies.begin();  dit != (*it)->dependencies.end(); ++dit) {
            if(VcsolutionDepend *vc = solution_depends[*dit])
                t << "\n\t\t" << (*it)->uuid << "." << cnt++ << " = " << vc->uuid;
        }
    }
    t << _slnProjDepEnd;
    t << _slnProjConfBeg;
    for(QList<VcsolutionDepend*>::Iterator it = solution_cleanup.begin(); it != solution_cleanup.end(); ++it) {
        t << "\n\t\t" << (*it)->uuid << _slnProjDbgConfTag1;
        t << "\n\t\t" << (*it)->uuid << _slnProjDbgConfTag2;
        t << "\n\t\t" << (*it)->uuid << _slnProjRelConfTag1;
        t << "\n\t\t" << (*it)->uuid << _slnProjRelConfTag2;
    }
    t << _slnProjConfEnd;
    t << _slnExtSections;
    t << _slnGlobalEnd;


    while (!solution_cleanup.isEmpty())
        delete solution_cleanup.takeFirst();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

bool VcprojGenerator::hasBuiltinCompiler(const QString &file)
{
    // Source files
    for (int i = 0; i < Option::cpp_ext.count(); ++i)
        if (file.endsWith(Option::cpp_ext.at(i)))
            return true;
    if (file.endsWith(".c"))
        return true;
    // Resource files
    if (file.endsWith(".rc")) 
        return true;
    return false;
}

void VcprojGenerator::init()
{
    if(init_flag)
        return;
    if(project->first("TEMPLATE") == "vcsubdirs") { //too much work for subdirs
        init_flag = true;
        return;
    }

    debug_msg(1, "Generator: MSVC.NET: Initializing variables");

    initOld();           // Currently calling old DSP code to set variables. CLEAN UP!

    // Figure out what we're trying to build
    if(project->first("TEMPLATE") == "vcapp") {
        projectTarget = Application;
    } else if(project->first("TEMPLATE") == "vclib") {
        if(project->isActiveConfig("staticlib"))
            projectTarget = StaticLib;
        else
            projectTarget = SharedLib;
    }

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    usePCH = !precompH.isEmpty() && project->isActiveConfig("precompile_header");
    if (usePCH) {
        precompHFilename = QFileInfo(precompH).fileName();
        // Created files
        QString origTarget = project->first("QMAKE_ORIG_TARGET");
        precompObj = origTarget + Option::obj_ext;
        precompPch = origTarget + ".pch";
        // Add PRECOMPILED_HEADER to HEADERS
        if (!project->variables()["HEADERS"].contains(precompH))
            project->variables()["HEADERS"] += precompH;
        // Return to variable pool
        project->variables()["PRECOMPILED_OBJECT"] = precompObj;
        project->variables()["PRECOMPILED_PCH"]    = precompPch;
    }

    // Add all input files for a custom compiler into a map for uniqueness,
    // unless the compiler is configure as a combined stage, then use the first one
    const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::ConstIterator it = quc.constBegin(); it != quc.constEnd(); ++it) {
        const QStringList &invar = project->variables().value((*it) + ".input");
        for(QStringList::ConstIterator iit = invar.constBegin(); iit != invar.constEnd(); ++iit) {
            QStringList fileList = project->variables().value(*iit);
            if (!fileList.isEmpty()) {
                if (project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1)
                    fileList = fileList.first();
                for(QStringList::ConstIterator fit = fileList.constBegin(); fit != fileList.constEnd(); ++fit) {
                    const QString &file = (*fit);
                    if (hasBuiltinCompiler(file))
                        warn_msg(WarnLogic, "extracompiler will override builtin compiler (%s)", file.latin1());
                    extraCompilerSources[file] += *it;
                }
            }
        }
    }

#if 0 // Debuging
    Q_FOREACH(QString aKey, extraCompilerSources.keys()) {
        qDebug("Extracompilers for %s are (%s)", aKey.latin1(), extraCompilerSources.value(aKey).join(", ").latin1());
    }
#endif

    initProject(); // Fills the whole project with proper data
}

bool VcprojGenerator::mergeBuildProject(MakefileGenerator *other)
{
    VcprojGenerator *otherVC = static_cast<VcprojGenerator*>(other);
    if (!otherVC) {
        warn_msg(WarnLogic, "VcprojGenerator: Cannot merge other types of projects! (ignored)");
        return false;
    }
    mergedProjects += otherVC;
    return true;
}

void VcprojGenerator::initProject()
{
    // Initialize XML sub elements
    // - Do this first since project elements may need
    // - to know of certain configuration options
    initConfiguration();
    initSourceFiles();
    initHeaderFiles();
    initMOCFiles();
    initFormsFiles();
    initTranslationFiles();
    initLexYaccFiles();
    initResourceFiles();

    initExtraCompilerOutputs();

    // Own elements -----------------------------
    vcProject.Name = project->first("QMAKE_ORIG_TARGET");
    vcProject.Version = use_net2003_version() ? "7.10" : "7.00";
    vcProject.ProjectGUID = getProjectUUID().toString().toUpper();
    vcProject.PlatformName = (vcProject.Configuration.idl.TargetEnvironment == midlTargetWin64 ? "Win64" : "Win32");
    // These are not used by Qt, but may be used by customers
    vcProject.SccProjectName = project->first("SCCPROJECTNAME");
    vcProject.SccLocalPath = project->first("SCCLOCALPATH");
    vcProject.flat_files = project->isActiveConfig("flat");
}

void VcprojGenerator::initConfiguration()
{
    // Initialize XML sub elements
    // - Do this first since main configuration elements may need
    // - to know of certain compiler/linker options
    VCConfiguration &conf = vcProject.Configuration;

    initCompilerTool();
    if(projectTarget == StaticLib)
        initLibrarianTool();
    else
        initLinkerTool();
    initIDLTool();

    // Own elements -----------------------------
    QString temp = project->first("BuildBrowserInformation");
    switch (projectTarget) {
    case SharedLib:
        conf.ConfigurationType = typeDynamicLibrary;
        break;
    case StaticLib:
        conf.ConfigurationType = typeStaticLibrary;
        break;
    case Application:
    default:
        conf.ConfigurationType = typeApplication;
        break;
    }

    // Only on configuration per build
    bool isDebug = project->isActiveConfig("debug");

    conf.Name = isDebug ? "Debug" : "Release";
    conf.Name += (conf.idl.TargetEnvironment == midlTargetWin64 ? "|Win64" : "|Win32");
    conf.ATLMinimizesCRunTimeLibraryUsage = (project->first("ATLMinimizesCRunTimeLibraryUsage").isEmpty() ? _False : _True);
    conf.BuildBrowserInformation = triState(temp.isEmpty() ? (short)unset : temp.toShort());
    temp = project->first("CharacterSet");
    conf.CharacterSet = charSet(temp.isEmpty() ? (short)charSetNotSet : temp.toShort());
    conf.DeleteExtensionsOnClean = project->first("DeleteExtensionsOnClean");
    conf.ImportLibrary = conf.linker.ImportLibrary;
    conf.IntermediateDirectory = project->first("OBJECTS_DIR");
    conf.OutputDirectory = ".";
    conf.PrimaryOutput = project->first("PrimaryOutput");
    conf.WholeProgramOptimization = conf.compiler.WholeProgramOptimization;
    temp = project->first("UseOfATL");
    if(!temp.isEmpty())
        conf.UseOfATL = useOfATL(temp.toShort());
    temp = project->first("UseOfMfc");
    if(!temp.isEmpty())
        conf.UseOfMfc = useOfMfc(temp.toShort());

    // Configuration does not need parameters from
    // these sub XML items;
    initCustomBuildTool();
    initPreBuildEventTools();
    initPostBuildEventTools();
    initPreLinkEventTools();

    // Set definite values in both configurations
    if (isDebug) {
        // Special debug options
        conf.linker.GenerateDebugInformation = _True;
        conf.compiler.PreprocessorDefinitions.removeAll("NDEBUG");
    } else {
        // Special release options
        conf.linker.GenerateDebugInformation = _False;
        conf.compiler.PreprocessorDefinitions += "NDEBUG";
    }

}

void VcprojGenerator::initCompilerTool()
{
    QString placement = project->first("OBJECTS_DIR");
    if(placement.isEmpty())
        placement = ".\\";

    VCConfiguration &conf = vcProject.Configuration;
    conf.compiler.AssemblerListingLocation = placement ;
    conf.compiler.ProgramDataBaseFileName = ".\\" ;
    conf.compiler.ObjectFile = placement ;
    // PCH
    if (usePCH) {
        conf.compiler.UsePrecompiledHeader     = pchUseUsingSpecific;
        conf.compiler.PrecompiledHeaderFile    = "$(IntDir)\\" + precompPch;
        conf.compiler.PrecompiledHeaderThrough = precompHFilename;
        conf.compiler.ForcedIncludeFiles       = precompHFilename;
        // Minimal build option triggers an Internal Compiler Error
        // when used in conjunction with /FI and /Yu, so remove it
        project->variables()["QMAKE_CFLAGS_DEBUG"].removeAll("-Gm");
        project->variables()["QMAKE_CFLAGS_DEBUG"].removeAll("/Gm");
        project->variables()["QMAKE_CXXFLAGS_DEBUG"].removeAll("-Gm");
        project->variables()["QMAKE_CXXFLAGS_DEBUG"].removeAll("/Gm");
    }

    conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS"]);
    if(project->isActiveConfig("debug")){
        // Debug version
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS"]);
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_DEBUG"]);
        if((projectTarget == Application) || (projectTarget == StaticLib))
            conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_MT_DBG"]);
        else
            conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_MT_DLLDBG"]);
    } else {
        // Release version
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS"]);
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_RELEASE"]);
        conf.compiler.PreprocessorDefinitions += "QT_NO_DEBUG";
        conf.compiler.PreprocessorDefinitions += "NDEBUG";
        if((projectTarget == Application) || (projectTarget == StaticLib))
            conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_MT"]);
        else
            conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_MT_DLL"]);
    }

    // Common for both release and debug
    if(project->isActiveConfig("warn_off"))
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_WARN_OFF"]);
    else if(project->isActiveConfig("warn_on"))
        conf.compiler.parseOptions(project->variables()["QMAKE_CXXFLAGS_WARN_ON"]);
    if(project->isActiveConfig("windows"))
        conf.compiler.PreprocessorDefinitions += project->variables()["MSVCPROJ_WINCONDEF"];

    // Can this be set for ALL configs?
    // If so, use qmake.conf!
    if(projectTarget == SharedLib)
        conf.compiler.PreprocessorDefinitions += "_WINDOWS";

    conf.compiler.PreprocessorDefinitions += project->variables()["DEFINES"];
    conf.compiler.PreprocessorDefinitions += project->variables()["PRL_EXPORT_DEFINES"];
    conf.compiler.parseOptions(project->variables()["MSVCPROJ_INCPATH"]);
}

void VcprojGenerator::initLibrarianTool()
{
    VCConfiguration &conf = vcProject.Configuration;
    conf.librarian.OutputFile = project->first("DESTDIR");
    if(conf.librarian.OutputFile.isEmpty())
        conf.librarian.OutputFile = ".\\";

    if(!conf.librarian.OutputFile.endsWith("\\"))
        conf.librarian.OutputFile += '\\';

    conf.librarian.OutputFile += project->first("MSVCPROJ_TARGET");
}

void VcprojGenerator::initLinkerTool()
{
    findLibraries(); // Need to add the highest version of the libs
    VCConfiguration &conf = vcProject.Configuration;
    conf.linker.parseOptions(project->variables()["MSVCPROJ_LFLAGS"]);
    conf.linker.AdditionalDependencies += project->variables()["MSVCPROJ_LIBS"];

    switch (projectTarget) {
    case Application:
        conf.linker.OutputFile = project->first("DESTDIR");
        break;
    case SharedLib:
        conf.linker.parseOptions(project->variables()["MSVCPROJ_LIBOPTIONS"]);
        conf.linker.OutputFile = project->first("DESTDIR");
        break;
    case StaticLib: //unhandled - added to remove warnings..
        break;
    }

    if(conf.linker.OutputFile.isEmpty())
        conf.linker.OutputFile = ".\\";

    if(!conf.linker.OutputFile.endsWith("\\"))
        conf.linker.OutputFile += '\\';

    conf.linker.OutputFile += project->first("MSVCPROJ_TARGET");

    if(project->isActiveConfig("debug")){
        conf.linker.parseOptions(project->variables()["QMAKE_LFLAGS_DEBUG"]);
    } else {
        conf.linker.parseOptions(project->variables()["QMAKE_LFLAGS_RELEASE"]);
    }

    if(project->isActiveConfig("dll")){
        conf.linker.parseOptions(project->variables()["QMAKE_LFLAGS_QT_DLL"]);
    }

    if(project->isActiveConfig("console")){
        conf.linker.parseOptions(project->variables()["QMAKE_LFLAGS_CONSOLE"]);
    } else {
        conf.linker.parseOptions(project->variables()["QMAKE_LFLAGS_WINDOWS"]);
    }

}

void VcprojGenerator::initIDLTool()
{
}

void VcprojGenerator::initCustomBuildTool()
{
}

void VcprojGenerator::initPreBuildEventTools()
{
}

void VcprojGenerator::initPostBuildEventTools()
{
    VCConfiguration &conf = vcProject.Configuration;
    if(!project->variables()["QMAKE_POST_LINK"].isEmpty()) {
        conf.postBuild.Description = var("QMAKE_POST_LINK");
        conf.postBuild.CommandLine = var("QMAKE_POST_LINK");
    }
    if(!project->variables()["MSVCPROJ_COPY_DLL"].isEmpty()) {
        if(!conf.postBuild.CommandLine.isEmpty())
            conf.postBuild.CommandLine += " && ";
        conf.postBuild.Description += var("MSVCPROJ_COPY_DLL_DESC");
        conf.postBuild.CommandLine += var("MSVCPROJ_COPY_DLL");
    }
}

void VcprojGenerator::initPreLinkEventTools()
{
}

void VcprojGenerator::addMocArguments(VCFilter &filter)
{
    QString &args = filter.customMocArguments;
    args.clear();
    // Add Defines
    args += varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") +
            varGlue("DEFINES"," -D"," -D","") +
            varGlue("QMAKE_COMPILER_DEFINES"," -D"," -D","");
    // Add Includes
    args += " -I" + specdir();
    args += varGlue("INCLUDEPATH"," -I", " -I", "");
    if(!project->isActiveConfig("no_include_pwd")) {
        QString pwd = fileFixify(QDir::currentPath());
        if(pwd.isEmpty())
            pwd = ".";
        args += " -I\"" + pwd + "\"";
    }
}

void VcprojGenerator::initSourceFiles()
{
    vcProject.SourceFiles.Name = "Source Files";
    vcProject.SourceFiles.Filter = "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat";

    vcProject.SourceFiles.addFiles(project->variables()["SOURCES"]);

    vcProject.SourceFiles.Project = this;
    vcProject.SourceFiles.Config = &(vcProject.Configuration);
    vcProject.SourceFiles.CustomBuild = none;
}

void VcprojGenerator::initHeaderFiles()
{
    QStringList list;
    vcProject.HeaderFiles.Name = "Header Files";
    vcProject.HeaderFiles.Filter = "h;hpp;hxx;hm;inl";
    list += project->variables()["HEADERS"];
    if (usePCH) // Generated PCH cpp file
        vcProject.HeaderFiles.addFile(precompH);

    for(int index = 0; index < list.count(); ++index)
        vcProject.HeaderFiles.addFile(VCFilterFile(list.at(index), mocFile(list.at(index))));

    vcProject.HeaderFiles.Project = this;
    vcProject.HeaderFiles.Config = &(vcProject.Configuration);
    vcProject.HeaderFiles.CustomBuild = mocHdr;
    addMocArguments(vcProject.HeaderFiles);
}

void VcprojGenerator::initMOCFiles()
{
    vcProject.MOCFiles.Name = "Generated MOC Files";
    vcProject.MOCFiles.Filter = "cpp;c;cxx;moc";

    // Create a list of the files being moc'ed
    QStringList &objl = project->variables()["OBJMOC"],
                &srcl = project->variables()["SRCMOC"];
    int index = 0;
    for(; index < objl.count() && index < srcl.count(); ++index) {
        vcProject.MOCFiles.addFile(VCFilterFile(srcl.at(index), mocSource(srcl.at(index))));
    }
    // Exclude the rest, except .moc's
    for(; index < srcl.count(); ++index)
        vcProject.MOCFiles.addFile(VCFilterFile(
                              srcl.at(index),
                              mocSource(srcl.at(index)),
                              srcl.at(index).endsWith(Option::cpp_moc_ext)
                              ? false : true));

    vcProject.MOCFiles.Project = this;
    vcProject.MOCFiles.Config = &(vcProject.Configuration);
    vcProject.MOCFiles.CustomBuild = mocSrc;
    addMocArguments(vcProject.MOCFiles);
}

void VcprojGenerator::initFormsFiles()
{
    vcProject.FormFiles.Name = "Forms";
    vcProject.FormFiles.ParseFiles = _False;
    vcProject.FormFiles.Filter = "ui";

    vcProject.FormFiles.addFiles(project->variables()["FORMS"]);

    vcProject.FormFiles.Project = this;
    vcProject.FormFiles.Config = &(vcProject.Configuration);
    vcProject.FormFiles.CustomBuild = none;
}

void VcprojGenerator::initTranslationFiles()
{
    vcProject.TranslationFiles.Name = "Translations Files";
    vcProject.TranslationFiles.ParseFiles = _False;
    vcProject.TranslationFiles.Filter = "ts";

    vcProject.TranslationFiles.addFiles(project->variables()["TRANSLATIONS"]);

    vcProject.TranslationFiles.Project = this;
    vcProject.TranslationFiles.Config = &(vcProject.Configuration);
    vcProject.TranslationFiles.CustomBuild = none;
}

void VcprojGenerator::initLexYaccFiles()
{
    vcProject.LexYaccFiles.Name = "Lex / Yacc Files";
    vcProject.LexYaccFiles.ParseFiles = _False;
    vcProject.LexYaccFiles.Filter = "l;y";

    vcProject.LexYaccFiles.addFiles(project->variables()["LEXSOURCES"]);
    vcProject.LexYaccFiles.addFiles(project->variables()["YACCSOURCES"]);

    vcProject.LexYaccFiles.Project = this;
    vcProject.LexYaccFiles.Config = &(vcProject.Configuration);
    vcProject.LexYaccFiles.CustomBuild = lexyacc;
}

void VcprojGenerator::initResourceFiles()
{
    vcProject.ResourceFiles.Name = "Resources";
    vcProject.ResourceFiles.ParseFiles = _False;
    vcProject.ResourceFiles.Filter = "cpp;ico;png;jpg;jpeg;gif;xpm;bmp;rc;ts";

    vcProject.ResourceFiles.addFiles(project->variables()["RC_FILE"]);
    vcProject.ResourceFiles.addFiles(project->variables()["QMAKE_IMAGE_COLLECTION"]);
    vcProject.ResourceFiles.addFiles(project->variables()["IMAGES"]);
    vcProject.ResourceFiles.addFiles(project->variables()["IDLSOURCES"]);

    vcProject.ResourceFiles.Project = this;
    vcProject.ResourceFiles.Config = &(vcProject.Configuration);
    vcProject.ResourceFiles.CustomBuild = none;
}

void VcprojGenerator::initExtraCompilerOutputs()
{
    const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    int count = 0;
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it, ++count) {
        // Create an extra compiler filter and add the files
        VCFilter extraCompile;
        extraCompile.Name = (*it);
        extraCompile.ParseFiles = _False;
        extraCompile.Filter = "";

        // If the extra compiler has a variable_out set the output file
        // is added to an other file list, and does not need its own..
        QString tmp_other_out = project->first((*it) + ".variable_out");
        if (!tmp_other_out.isEmpty())
            continue;

        QString tmp_out = project->first((*it) + ".output");
        if (project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1) {
            // Combined output, only one file result
            extraCompile.addFile(
                Option::fixPathToTargetOS(replaceExtraCompilerVariables(tmp_out, QString::null, QString::null), false));
        } else {
            // One output file per input
            QStringList tmp_in = project->variables()[project->first((*it) + ".input")];
            for (int i = 0; i < tmp_in.count(); ++i)
                extraCompile.addFile(
                    Option::fixPathToTargetOS(replaceExtraCompilerVariables(tmp_out, tmp_in.at(i), QString::null), false));
        }
        extraCompile.Project = this;
        extraCompile.Config = &(vcProject.Configuration);
        extraCompile.CustomBuild = none;

        vcProject.ExtraCompilersFiles.append(extraCompile);
    }
}

/* \internal
    Sets up all needed variables from the environment and all the different caches and .conf files
*/

void VcprojGenerator::initOld()
{
    if(init_flag)
        return;

    init_flag = true;
    QStringList::Iterator it;

    // this should probably not be here, but I'm using it to wrap the .t files
    if(project->first("TEMPLATE") == "vcapp")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");
    if(project->variables()["QMAKESPEC"].isEmpty())
        project->variables()["QMAKESPEC"].append(getenv("QMAKESPEC"));

    processDllConfig();

    // Decode version, and add it to $$MSVCPROJ_VERSION --------------
    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"][0];
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(QRegExp("\\."), "");
        project->variables()["MSVCPROJ_VERSION"].append("/VERSION:" + major + "." + minor);
    }

    // QT ------------------------------------------------------------
    if(project->isActiveConfig("qt")) {
        project->variables()["CONFIG"].append("moc");
        project->variables()["INCLUDEPATH"] +=        project->variables()["QMAKE_INCDIR_QT"];
        project->variables()["QMAKE_LIBDIR"] += project->variables()["QMAKE_LIBDIR_QT"];

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

            if(!project->isActiveConfig("dll") && !project->isActiveConfig("plugin"))
                project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
        }
    }

    fixTargetExt();
    processMocConfig();

    // /VERSION:x.yz -------------------------------------------------
    if(!project->variables()["VERSION"].isEmpty()) {
        QString version = project->variables()["VERSION"][0];
        int firstDot = version.indexOf(".");
        QString major = version.left(firstDot);
        QString minor = version.right(version.length() - firstDot - 1);
        minor.replace(".", "");
        project->variables()["QMAKE_LFLAGS"].append("/VERSION:" + major + "." + minor);
    }

    processLibsVar();
    processFileTagsVar();

     // Get filename w/o extention -----------------------------------
    QString msvcproj_project = "";
    QString targetfilename = "";
    if(project->variables()["TARGET"].count()) {
        msvcproj_project = project->variables()["TARGET"].first();
        targetfilename = msvcproj_project;
    }

    // Save filename w/o extention in $$QMAKE_ORIG_TARGET ------------
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    // TARGET (add extention to $$TARGET)
    //project->variables()["MSVCPROJ_DEFINES"].append(varGlue(".first() += project->first("TARGET_EXT");

    // Init base class too -------------------------------------------
    MakefileGenerator::init();


    if(msvcproj_project.isEmpty())
        msvcproj_project = Option::output.fileName();

    msvcproj_project = msvcproj_project.right(msvcproj_project.length() - msvcproj_project.lastIndexOf("\\") - 1);
    msvcproj_project = msvcproj_project.left(msvcproj_project.lastIndexOf("."));
    msvcproj_project.replace(QRegExp("-"), "");

    project->variables()["MSVCPROJ_PROJECT"].append(msvcproj_project);
    QStringList &proj = project->variables()["MSVCPROJ_PROJECT"];

    for(it = proj.begin(); it != proj.end(); ++it)
        (*it).replace(QRegExp("\\.[a-zA-Z0-9_]*$"), "");

    // SUBSYSTEM -----------------------------------------------------
    if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
            project->variables()["MSVCPROJ_TEMPLATE"].append("win32app" + project->first("VCPROJ_EXTENSION"));
            if(project->isActiveConfig("console")) {
                project->variables()["MSVCPROJ_CONSOLE"].append("CONSOLE");
                project->variables()["MSVCPROJ_WINCONDEF"].append("_CONSOLE");
                project->variables()["MSVCPROJ_VCPROJTYPE"].append("0x0103");
                project->variables()["MSVCPROJ_SUBSYSTEM"].append("CONSOLE");
            } else {
                project->variables()["MSVCPROJ_CONSOLE"].clear();
                project->variables()["MSVCPROJ_WINCONDEF"].append("_WINDOWS");
                project->variables()["MSVCPROJ_VCPROJTYPE"].append("0x0101");
                project->variables()["MSVCPROJ_SUBSYSTEM"].append("WINDOWS");
            }
    } else {
        if(project->isActiveConfig("dll")) {
            project->variables()["MSVCPROJ_TEMPLATE"].append("win32dll" + project->first("VCPROJ_EXTENSION"));
        } else {
            project->variables()["MSVCPROJ_TEMPLATE"].append("win32lib" + project->first("VCPROJ_EXTENSION"));
        }
    }

    // $$QMAKE.. -> $$MSVCPROJ.. -------------------------------------
    project->variables()["MSVCPROJ_LIBS"] += project->variables()["QMAKE_LIBS"];
    project->variables()["MSVCPROJ_LFLAGS"] += project->variables()["QMAKE_LFLAGS"];
    if(!project->variables()["QMAKE_LIBDIR"].isEmpty()) {
        QStringList strl = project->variables()["QMAKE_LIBDIR"];
        QStringList::iterator stri;
        for(stri = strl.begin(); stri != strl.end(); ++stri) {
            if(!(*stri).startsWith("/LIBPATH:"))
                (*stri).prepend("/LIBPATH:");
        }
        project->variables()["MSVCPROJ_LFLAGS"] += strl;
    }
    project->variables()["MSVCPROJ_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS"];
    // We don't use this... Direct manipulation of compiler object
    //project->variables()["MSVCPROJ_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    //project->variables()["MSVCPROJ_DEFINES"].append(varGlue("PRL_EXPORT_DEFINES","/D ","" " /D ",""));
    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
        QString inc = (*incit);
        inc.replace(QRegExp("\""), "");
        project->variables()["MSVCPROJ_INCPATH"].append("/I" + inc);
    }
    project->variables()["MSVCPROJ_INCPATH"].append("/I" + specdir());

    QString dest;
    project->variables()["MSVCPROJ_TARGET"] = project->first("TARGET");
    Option::fixPathToTargetOS(project->first("TARGET"));
    dest = project->first("TARGET") + project->first("TARGET_EXT");
    if(project->first("TARGET").startsWith("$(QTDIR)"))
        dest.replace(QRegExp("\\$\\(QTDIR\\)"), getenv("QTDIR"));
    project->variables()["MSVCPROJ_TARGET"] = dest;

    // DLL COPY ------------------------------------------------------
    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
        QStringList dlldirs = project->variables()["DLLDESTDIR"];
        QString copydll("");
        QStringList::Iterator dlldir;
        for(dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir) {
            if(!copydll.isEmpty())
                copydll += " && ";
            copydll += "copy  \"$(TargetPath)\" \"" + *dlldir + "\"";
        }

        QString deststr("Copy " + dest + " to ");
        for(dlldir = dlldirs.begin(); dlldir != dlldirs.end();) {
            deststr += *dlldir;
            ++dlldir;
            if(dlldir != dlldirs.end())
                deststr += ", ";
        }

        project->variables()["MSVCPROJ_COPY_DLL"].append(copydll);
        project->variables()["MSVCPROJ_COPY_DLL_DESC"].append(deststr);
    }

    if (!project->variables()["DEF_FILE"].isEmpty())
        project->variables()["MSVCPROJ_LFLAGS"].append("/DEF:"+project->first("DEF_FILE"));

    // FORMS ---------------------------------------------------------
    QStringList &list = project->variables()["FORMS"];
    for(it = list.begin(); it != list.end(); ++it) {
        if(QFile::exists(*it + ".h"))
            project->variables()["SOURCES"].append(*it + ".h");
    }

    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "MSVCPROJ_LFLAGS" << "MSVCPROJ_LIBS";

    // Verbose output if "-d -d"...
    outputVariables();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

QString VcprojGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out)
{
    QString ret = MakefileGenerator::replaceExtraCompilerVariables(var, in, out);
    ret.replace("$(DEFINES)",  varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") +
                varGlue("DEFINES"," -D"," -D",""));
    ret.replace("$(INCPATH)",  this->var("MSVCPROJ_INCPATH"));
    return ret;
}



bool VcprojGenerator::openOutput(QFile &file, const QString &build) const
{
    QString outdir;
    if(!file.fileName().isEmpty()) {
        QFileInfo fi(file);
        if(fi.isDir())
            outdir = file.fileName() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.fileName().isEmpty()) {
        QString ext = project->first("VCPROJ_EXTENSION");
        if(project->first("TEMPLATE") == "vcsubdirs")
            ext = project->first("VCSOLUTION_EXTENSION");
        file.setFileName(outdir + project->first("QMAKE_ORIG_TARGET") + ext);
    }
    if(QDir::isRelativePath(file.fileName()))
        file.setFileName(Option::fixPathToLocalOS(QDir::currentPath() + Option::dir_sep + fixFilename(file.fileName())));
    return Win32MakefileGenerator::openOutput(file, build);
}

QString VcprojGenerator::fixFilename(QString ofile) const
{
    ofile = Option::fixPathToLocalOS(ofile);
    int slashfind = ofile.lastIndexOf(Option::dir_sep);
    if(slashfind == -1) {
        ofile = ofile.replace('-', '_');
    } else {
        int hypenfind = ofile.indexOf('-', slashfind);
        while (hypenfind != -1 && slashfind < hypenfind) {
            ofile = ofile.replace(hypenfind, 1, '_');
            hypenfind = ofile.indexOf('-', hypenfind + 1);
        }
    }
    return ofile;
}

QString VcprojGenerator::findTemplate(QString file)
{
    QString ret;
    if(!QFile::exists((ret = file)) &&
       !QFile::exists((ret = QString(Option::mkfile::qmakespec + "/" + file))) &&
       !QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/win32-msvc.net/" + file)) &&
       !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))))
        return "";
    debug_msg(1, "Generator: MSVC.NET: Found template \'%s\'", ret.latin1());
    return ret;
}


void VcprojGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_DEFINES") {
        QStringList &out = project->variables()["MSVCPROJ_DEFINES"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(out.indexOf((*it)) == -1)
                out.append((" /D " + *it));
        }
    } else {
        MakefileGenerator::processPrlVariable(var, l);
    }
}

void VcprojGenerator::outputVariables()
{
#if 0
    qDebug("Generator: MSVC.NET: List of current variables:");
    for(QMap<QString, QStringList>::ConstIterator it = project->variables().begin(); it != project->variables().end(); ++it)
        qDebug("Generator: MSVC.NET: %s => %s", it.key().latin1(), it.data().join(" | ").latin1());
#endif
}
