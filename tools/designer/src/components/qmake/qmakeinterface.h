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

#ifndef QMAKEINTERFACE_H
#define QMAKEINTERFACE_H

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class QMakeProject;
class MakefileGenerator;

#ifdef QT_BUILD_QMAKE_LIBRARY
# define QM_QMAKE_EXPORT Q_DECL_EXPORT
#else
# define QM_QMAKE_EXPORT Q_DECL_IMPORT
#endif

class QM_QMAKE_EXPORT QMakeInterface
{
    QString profile;
    mutable MakefileGenerator *mkfile;
    mutable QMap<QString, MakefileGenerator *> build_mkfile;
    QMakeProject *getBuildStyle(const QString &buildStyle) const;

public:
    QMakeInterface(bool findFile = true, int argc=0, char **argv=0);
    QMakeInterface(const QString &file, int argc=0, char **argv=0);
    ~QMakeInterface();

    QStringList buildStyles() const;

    QStringList compileFlags(const QString &buildStyle = QString(), bool cplusplus=true) const;
    QStringList defines(const QString &buildStyle = QString()) const;
    QStringList linkFlags(const QString &buildStyle = QString()) const;
    QStringList libraries(const QString &buildStyle = QString()) const;

    bool addVariable(const QString &);
    inline bool addVariable(const QString &var, const QString &val) 
        { return addVariable(var + " += " + val); }
    inline bool setVariable(const QString &var, const QString &val) 
        { return addVariable(var + " = " + val); }
    inline bool setConfig(const QString &config) 
        { return addVariable("CONFIG", config); }
    inline bool setQtConfig(const QString &qt) 
        { return addVariable("QT", qt); }
};

#endif // QMAKEINTERFACE_H
