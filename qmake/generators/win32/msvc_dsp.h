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

    bool writeMakefile(QTextStream &);
    QString findTemplate(const QString &file);
    void init();

public:
    DspMakefileGenerator();
    ~DspMakefileGenerator();

    bool supportsMetaBuild() { return false; }
    bool openOutput(QFile &file, const QString &build) const;

protected:
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();

    QString precompH,
            precompObj, precompPch;
    bool usePCH;
};

inline DspMakefileGenerator::~DspMakefileGenerator()
{ }

inline bool DspMakefileGenerator::findLibraries()
{ return Win32MakefileGenerator::findLibraries("MSVCDSP_LIBS"); }

#endif /* __MSVC_DSP_H__ */
