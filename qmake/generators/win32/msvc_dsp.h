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
    bool writeDspParts(QTextStream &);
    bool writeFileGroup(QTextStream &t, const QStringList &files, const QString &group, const QString &filter);
    bool writeBuildstepForFile(QTextStream &t, const QString &file);

    bool writeMakefile(QTextStream &);
    void init();

public:
    DspMakefileGenerator();
    ~DspMakefileGenerator();

    bool supportsMetaBuild() { return false; }
    bool openOutput(QFile &file, const QString &build) const;
    bool hasBuiltinCompiler(const QString &filename) const;

protected:
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();

    QString precompH, namePCH,
            precompObj, precompPch;

    QString platform;
    QStringList configurations;
    bool usePCH;

    struct BuildStep {
        BuildStep() {}
        BuildStep(int configurations) {
            configs = configurations;
            while (configurations--) {
                deps << QString();
                buildSteps << QString();
                buildNames << QString();
                buildOutputs << QStringList();
            }
        }

        BuildStep &operator<<(const BuildStep &other) {
            for (int i = 0; i < configs; ++i) {
                deps << other.deps;
                buildSteps[i] += other.buildSteps[i];
                buildNames[i] += other.buildNames[i];
                buildOutputs[i] += other.buildOutputs[i];
            }
            return *this;
        }

        QStringList deps;
        QStringList buildSteps;
        QStringList buildNames;
        QList<QStringList> buildOutputs;
        int configs;
    };
    QMap<QString, BuildStep> swappedBuildSteps;
};

inline DspMakefileGenerator::~DspMakefileGenerator()
{ }

inline bool DspMakefileGenerator::findLibraries()
{ return Win32MakefileGenerator::findLibraries("MSVCDSP_LIBS"); }

#endif /* __MSVC_DSP_H__ */
