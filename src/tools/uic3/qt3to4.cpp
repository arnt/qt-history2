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


#include "qt3to4.h"

#include <iostream>

#include <qconfig.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QLibraryInfo>
#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>

class ContentHandler: public QXmlDefaultHandler
{
public:
    typedef QPair<QString, QString> Rule;
    typedef QList<Rule> RuleList;

public:
    ContentHandler() {}

    inline RuleList renamedHeaders() const
    { return rules(QString::fromLatin1("RenamedHeader")); }

    inline RuleList renamedClasses() const
    { return rules(QString::fromLatin1("RenamedClass")); }

    inline RuleList rules(const QString &kind) const
    { return m_rules.value(kind); }

    virtual bool startDocument()
    {
        m_rules.clear();
        m_state.current.clear();
        m_state.kind.clear();
        m_state.q3.clear();
        m_state.q4.clear();

        return true;
    }

    virtual bool startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &attrs)
    {
        if (qName == QLatin1String("item")) {
            m_state.kind = attrs.value("Type");

            m_state.current.clear();
            m_state.q3.clear();
            m_state.q4.clear();

            if (!m_rules.contains(m_state.kind))
                m_rules[m_state.kind].clear();

            return true;
        }

        return true; // ### only if it is a valid tag
    }


    virtual bool endElement(const QString &, const QString &, const QString &qName)
    {
        if (qName == QLatin1String("Qt3")) {
            m_state.q3 = m_state.current.trimmed();
        } else if (qName == QLatin1String("Qt4")) {
            m_state.q4 = m_state.current.trimmed();
        } else if (qName == QLatin1String("item")
                   && (m_state.kind == QLatin1String("RenamedHeader") || m_state.kind == QLatin1String("RenamedClass"))) {
            Rule r(m_state.q3, m_state.q4);
            m_rules[m_state.kind].append(r);
        }

        m_state.current.clear();
        return true;
    }

    virtual bool characters(const QString &ch)
    { m_state.current += ch; return true; }

    virtual bool error(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

    virtual bool fatalError(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

    virtual bool warning(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

private:
    QHash<QString, RuleList> m_rules;

    struct state
    {
        QString current;
        QString kind;
        QString q3;
        QString q4;
    } m_state;
};

void Porting::readXML(QString fileName, RuleList *renamedHeaders, RuleList *renamedClasses)
{
/*
    Rules for findng q3porting.xml
    1. qInstallPathLibs()/qt3to4/
    2. $QTDIR/tools/porting/src/
    3. applicationDirPath()../lib/qt3to4/src/
    4. applicationDirPath()../tools/porting/src/
*/
    QString rulesFileName = "q3porting.xml";

    if(fileName.isEmpty()) {
        fileName = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::DataPath) + "/qt3to4/" + fileName);
        QFile f(fileName);
        if (!f.exists())
            fileName=QString();
    }
    
    if(fileName.isEmpty()) {
        fileName= QDir::cleanPath(QFile::decodeName(qgetenv("QTDIR")) + "/tools/porting/src/" + rulesFileName);
        QFile f(fileName);
        if (!f.exists())
            fileName=QString();
    }
    
    QString applicationDirPath = QApplication::instance()->applicationDirPath();
    if(fileName.isEmpty()) {
        fileName = QDir::cleanPath(applicationDirPath + "/../lib/qt3to4/" + rulesFileName);
        QFile f(fileName);
        if (!f.exists())
            fileName=QString();
    }
    
    if(fileName.isEmpty()) {
        fileName = QDir::cleanPath(applicationDirPath + "/../tools/porting/src/" + rulesFileName);
        QFile f(fileName);
        if (!f.exists())
            fileName=QString();
    }
    if (fileName.isEmpty()) {
        std::cout << "Error: Could not find rules file: " << rulesFileName.latin1() << std::endl;
        std::cout << "Please try setting the QTDIR environment variable" << std::endl;
        Q_ASSERT(0);
    }
    
    ContentHandler handler;

    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QXmlInputSource source(file);
    bool ok = reader.parse(&source);
    Q_UNUSED(ok);
    Q_ASSERT(ok);

    if (renamedHeaders)
        *renamedHeaders = handler.renamedHeaders();
    if (renamedClasses)
        *renamedClasses = handler.renamedClasses();
}

Porting::Porting()
{
    readXML(QString(), &m_renamedHeaders, &m_renamedClasses);
}

int Porting::findRule(const RuleList &rules, const QString &q3)
{
    for (int i=0; i<rules.count(); ++i)
        if (rules.at(i).first == q3)
            return i;
    return -1;
}

QString Porting::renameHeader(const QString &headerName) const
{
    int index = findRule(m_renamedHeaders, headerName);
    return index == -1 ? headerName : m_renamedHeaders.at(index).second;
}

QString Porting::renameClass(const QString &className) const
{
    int index = findRule(m_renamedClasses, className);
    return index == -1 ? className : m_renamedClasses.at(index).second;
}
