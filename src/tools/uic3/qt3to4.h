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

#ifndef QT3TO4_H
#define QT3TO4_H

#include <QString>
#include <qlist.h>
#include <qpair.h>

class Porting
{
public:
    typedef QPair<QString, QString> Rule;
    typedef QList<Rule> RuleList;

public:
    Porting();

    inline RuleList renamedHeaders() const
    { return m_renamedHeaders; }

    inline RuleList renamedClasses() const
    { return m_renamedClasses; }

    QString renameHeader(const QString &headerName) const;
    QString renameClass(const QString &className) const;
    QString renameEnumerator(const QString &enumName) const;

protected:
    static void readXML(RuleList *renamedHeaders, RuleList *renamedClasses, RuleList *renamedEnums);
    static int findRule(const RuleList &rules, const QString &q3);

private:
    RuleList m_renamedHeaders;
    RuleList m_renamedClasses;
    RuleList m_renamedEnums;
};

#endif // QT3TO4_H
