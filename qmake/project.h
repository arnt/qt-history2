/****************************************************************************
**
** Definition of QMakeProject class.
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

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <qstringlist.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qstack.h>
#include <qmap.h>

class QMakeProperty;

class QMakeProject
{
    struct ScopeIterator;
    friend struct ScopeIterator;
    struct ScopeBlock;
    friend struct ScopeBlock;

    struct ScopeBlock
    {
        enum TestStatus { TestNone, TestFound, TestSeek };
        ScopeBlock() : iterate(0), ignore(false), else_status(TestNone) { }
        ScopeBlock(bool i) : iterate(0), ignore(i), else_status(TestNone) { }
        ~ScopeBlock();
        ScopeIterator *iterate;
        uint ignore : 1, else_status : 2;
    };

    QStack<ScopeBlock> scope_blocks;
    ScopeIterator *iterator;

    QString pfile, cfile;
    QMakeProperty *prop;
    void reset();
    QMap<QString, QStringList> vars, base_vars, cache;
    bool parse(const QString &text, QMap<QString, QStringList> &place);

    enum IncludeStatus {
        IncludeSuccess,
        IncludeFeatureAlreadyLoaded,
        IncludeFailure,
        IncludeNoExist,
        IncludeParseFailure
    };
    IncludeStatus doProjectInclude(QString file, bool feature, QMap<QString, QStringList> &place,
                          const QString &seek_var=QString::null);
    bool doProjectTest(const QString &func, const QString &params, QMap<QString, QStringList> &place);
    bool doProjectTest(const QString &func, QStringList args, QMap<QString, QStringList> &place);
    bool doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place);
    QString doVariableReplace(QString &str, const QMap<QString, QStringList> &place);
    void init(QMakeProperty *);

public:
    QMakeProject() { init(0); }
    QMakeProject(QMakeProperty *p) { init(p); }

    enum { ReadCache=0x01, ReadConf=0x02, ReadCmdLine=0x04, ReadProFile=0x08,
           ReadPostFiles=0x10, ReadFeatures=0x20, ReadAll=0xFF };
    bool read(const QString &project, const QString &pwd, uchar cmd=ReadAll);
    bool read(uchar cmd=ReadAll);

    QString projectFile();
    QString configFile();

    bool isEmpty(const QString &v);
    QStringList &values(const QString &v);
    QString first(const QString &v);
    QMap<QString, QStringList> &variables();
    bool isActiveConfig(const QString &x, bool regex=false, QMap<QString, QStringList> *place=NULL);

protected:
    friend class MakefileGenerator;
    bool read(const QString &file, QMap<QString, QStringList> &place);
    bool read(QTextStream &file, QMap<QString, QStringList> &place);

};

inline QString QMakeProject::projectFile()
{
    if (pfile == "-")
        return QString("(stdin)");
    return pfile;
}

inline QString QMakeProject::configFile()
{ return cfile; }

inline bool QMakeProject::isEmpty(const QString &v)
{ return !vars.contains(v) || vars[v].isEmpty(); }

inline QStringList &QMakeProject::values(const QString &v)
{ return vars[v]; }

inline QString QMakeProject::first(const QString &v)
{
    if (isEmpty(v))
        return QString("");
    return vars[v].first();
}

inline QMap<QString, QStringList> &QMakeProject::variables()
{ return vars; }

#endif /* __PROJECT_H__ */
