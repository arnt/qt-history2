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

#ifndef WRITEINITIALIZATION_H
#define WRITEINITIALIZATION_H

#include "treewalker.h"

#include <qpair.h>
#include <qhash.h>
#include <qstack.h>

class QTextStream;
class Driver;
class Uic;
struct Option;

struct WriteInitialization : public TreeWalker
{
    WriteInitialization(Uic *uic);

//
// widgets
//
    void accept(DomUI *node);
    void accept(DomWidget *node);
    void accept(DomLayout *node);
    void accept(DomSpacer *node);
    void accept(DomLayoutItem *node);

//
// actions
//
    void accept(DomActionGroup *node);
    void accept(DomAction *node);
    void accept(DomActionRef *node);

//
// tab stops
//
    void accept(DomTabStops *tabStops);

//
// custom widgets
//
    void accept(DomCustomWidgets *node);
    void accept(DomCustomWidget *node);

//
// layout defaults
//
    void accept(DomLayoutDefault *node);

private:
    static QString domColor2QString(DomColor *c);

    QString pixCall(DomResourcePixmap *pix) const;
    QString trCall(const QString &str, const QString &className) const;
    QString trCall(const DomString *str, const QString &className) const;

    void writeProperties(const QString &varName, const QString &className,
                         const QList<DomProperty*> &lst);
    void writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName);

    QString translate(const QString &text, const QString &className=QString::null) const;

//
// special initialization
//
    void initializeMenu(DomWidget *w, const QString &parentWidget);
    void initializeListBox(DomWidget *w);
    void initializeIconView(DomWidget *w);
    void initializeListView(DomWidget *w);
    void initializeListViewItems(const QString &className, const QString &varName, const QList<DomItem*> &items);
    void initializeTable(DomWidget *w);
    void initializeTableItems(const QString &className, const QString &varName, const QList<DomItem*> &items);

//
// Sql
//
    void initializeSqlDataTable(DomWidget *w);
    void initializeSqlDataBrowser(DomWidget *w);
    
private:
    Uic *uic;
    Driver *driver;
    QTextStream &output;
    const Option &option;
    bool m_stdsetdef;

    struct Buddy
    {
        Buddy(const QString &oN, const QString &b)
            : objName(oN), buddy(b) {}
        QString objName;
        QString buddy;
    };

    QStack<DomWidget*> m_widgetChain;
    QStack<DomLayout*> m_layoutChain;
    QStack<DomActionGroup*> m_actionGroupChain;
    QList<Buddy> m_buddies;

    QHash<QString, QString> m_buttonGroups;
    QHash<QString, DomWidget*> m_registeredWidgets;
    int m_defaultMargin;
    int m_defaultSpacing;

    QString m_generatedClass;

    QString m_delayedInitialization;
    QTextStream refreshOut;

    QString m_delayedActionInitialization;
    QTextStream actionOut;
};


#endif // WRITEINITIALIZATION_H
