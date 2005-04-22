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

#ifndef __MSVC_DSP_H__
#define __MSVC_DSP_H__

#include "winmakefile.h"

class DspMakefileGenerator : public Win32MakefileGenerator
{
    QString currentGroup;
    void beginGroupForFile(QString file, QTextStream &, const QString& filter="");
    void endGroups(QTextStream &);

    bool init_flag;
    bool writeDspHeader(QTextStream &);
    bool writeDspParts(QTextStream &);
    bool writeFileGroup(QTextStream &t, const QStringList &listNames, const QString &group, const QString &filter);
    bool writeBuildstepForFile(QTextStream &t, const QString &file, const QString &listName);
    static bool writeDspConfig(QTextStream &t, DspMakefileGenerator *config);
    static QString writeBuildstepForFileForConfig(const QString &file, const QString &listName, DspMakefileGenerator *config);
    QString configName(DspMakefileGenerator * config);

    bool writeMakefile(QTextStream &);
    bool writeProjectMakefile();
    void init();

public:
    DspMakefileGenerator();
    ~DspMakefileGenerator();

    bool openOutput(QFile &file, const QString &build) const;
    bool hasBuiltinCompiler(const QString &filename) const;

protected:
    virtual bool doDepends() const { return false; } //never necesary
    virtual void processSources() { filterIncludedFiles("SOURCES"); }
    virtual QString replaceExtraCompilerVariables(const QString &, const QString &, const QString &);
    virtual bool supportsMetaBuild() { return true; }
    virtual bool supportsMergedBuilds() { return true; }
    virtual bool mergeBuildProject(MakefileGenerator *other);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();

    bool usePCH;
    QString precompH, namePCH,
            precompObj, precompPch;

    QString platform;

    struct BuildStep {
        BuildStep() {}
        BuildStep &operator<<(const BuildStep &other) {
            deps << other.deps;
            buildStep += other.buildStep;
            buildName += other.buildName;
            buildOutputs += other.buildOutputs;
            return *this;
        }

        QStringList deps;
        QString buildStep;
        QString buildName;
        QStringList buildOutputs;
    };
    QMap<QString, BuildStep> swappedBuildSteps;



    // Holds all configurations for glue (merged) project
    QList<DspMakefileGenerator*> mergedProjects;
};

inline DspMakefileGenerator::~DspMakefileGenerator()
{ }

inline bool DspMakefileGenerator::findLibraries()
{ return Win32MakefileGenerator::findLibraries("MSVCDSP_LIBS"); }

#endif /* __MSVC_DSP_H__ */
