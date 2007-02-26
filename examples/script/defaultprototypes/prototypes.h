/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <QtCore/QObject>
#include <QtScript/QScriptable>

class ListWidgetItemPrototype : public QObject, public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
public:
    ListWidgetItemPrototype(QObject *parent = 0);

    QString text() const;
    void setText(const QString &text);

public Q_SLOTS:
    QString toString() const;
};

class ListWidgetPrototype : public QObject, public QScriptable
{
    Q_OBJECT
public:
    ListWidgetPrototype(QObject *parent = 0);

public Q_SLOTS:
    void addItem(const QString &text);
    void addItems(const QStringList &texts);
    void setBackgroundColor(const QString &colorName);
};

#endif
