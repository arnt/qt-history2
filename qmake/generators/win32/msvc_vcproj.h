
/****************************************************************************
**
** Definition of VcprojGenerator class.
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
    virtual void writeSubDirs(QTextStream &t);
    QString findTemplate(QString file);
    void init();

public:
    VcprojGenerator(QMakeProject *p);
    ~VcprojGenerator();

    QString defaultMakefile() const;
    virtual bool doDepends() const { return false; } //never necesary
    QString precompH, precompHFilename,
            precompObj, precompPch;
    bool usePCH;
    QString mocFile(const QString &file) { return Win32MakefileGenerator::mocFile(file); }

protected:
    virtual bool supportsMetaBuild() { return false; }
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
    void initUICFiles();
    void initFormsFiles();
    void initTranslationFiles();
    void initLexYaccFiles();
    void initResourceFiles();
    void addMocArguments(VCFilter &filter);

    VCProject vcProject;
    target projectTarget;

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
