/****************************************************************************
**
** Definition of DspMakefileGenerator class.
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

#ifndef __MSVC_DSP_H__
#define __MSVC_DSP_H__

#include "winmakefile.h"
#include <qvaluestack.h>

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
    DspMakefileGenerator(QMakeProject *p);
    ~DspMakefileGenerator();

    bool openOutput(QFile &file) const;

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
