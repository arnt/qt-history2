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

#ifndef __MSVC_VCPROJ_H__
#define __MSVC_VCPROJ_H__

#include "winmakefile.h"
#include "msvc_objectmodel.h"

enum target {
    Application,
    SharedLib,
    StaticLib
};

struct QUuid;
class VcprojGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    bool writeVcprojParts(QTextStream &);

    bool writeMakefile(QTextStream &);
    bool writeProjectMakefile();
    void writeSubDirs(QTextStream &t);

    QString findTemplate(QString file);
    void init();

public:
    VcprojGenerator();
    ~VcprojGenerator();

    QString defaultMakefile() const;
    virtual bool doDepends() const { return false; } //never necesary
    QString precompH, precompHFilename,
            precompObj, precompPch;
    QString mocFile(const QString &file) { return Win32MakefileGenerator::mocFile(file); }
    static bool hasBuiltinCompiler(const QString &file);

    QMap<QString, QStringList> extraCompilerSources;
    bool usePCH;

protected:
    virtual QString replaceExtraCompilerVariables(const QString &, const QString &, const QString &);
    virtual bool supportsMetaBuild() { return true; }
    virtual bool supportsMergedBuilds() { return true; }
    virtual bool mergeBuildProject(MakefileGenerator *other);

    virtual bool openOutput(QFile &file, const QString &build) const;
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();
    virtual void outputVariables();
    QString fixFilename(QString ofile) const;

    void initOld();
    void initProject();
    void initConfiguration();
    void initCompilerTool();
    void initLinkerTool();
    void initLibrarianTool();
    void initIDLTool();
    void initCustomBuildTool();
    void initPreBuildEventTools();
    void initPostBuildEventTools();
    void initPreLinkEventTools();
    void initSourceFiles();
    void initHeaderFiles();
    void initMOCFiles();
    void initTranslationFiles();
    void initResourceFiles();
    void initLexYaccFiles();
    void initExtraCompilerOutputs();

    void addMocArguments(VCFilter &filter);

    target projectTarget;

    // Used for single project
    VCProjectSingleConfig vcProject;

    // Holds all configurations for glue (merged) project
    QList<VcprojGenerator*> mergedProjects;

private:
    QUuid getProjectUUID(const QString &filename=QString::null);
    QUuid increaseUUID(const QUuid &id);
    friend class VCFilter;
};

inline VcprojGenerator::~VcprojGenerator()
{ }

inline QString VcprojGenerator::defaultMakefile() const
{
    return project->first("TARGET") + project->first("VCPROJ_EXTENSION");
}

inline bool VcprojGenerator::findLibraries()
{
    return Win32MakefileGenerator::findLibraries("MSVCPROJ_LIBS");
}

#endif /* __MSVC_VCPROJ_H__ */
