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

#include "prototypes.h"
#include <QtGui/QListWidgetItem>
#include <QtGui/QListWidget>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

Q_DECLARE_METATYPE(QListWidgetItem*)
Q_DECLARE_METATYPE(QListWidget*)

ListWidgetItemPrototype::ListWidgetItemPrototype(QObject *parent)
    : QObject(parent)
{
}

QString ListWidgetItemPrototype::text() const
{
    QListWidgetItem *item = qscriptvalue_cast<QListWidgetItem*>(thisObject());
    if (item)
        return item->text();
    return QString();
}

void ListWidgetItemPrototype::setText(const QString &text)
{
    QListWidgetItem *item = qscriptvalue_cast<QListWidgetItem*>(thisObject());
    if (item)
        item->setText(text);
}

QString ListWidgetItemPrototype::toString() const
{
    return QString("ListWidgetItem(text = %0)").arg(text());
}



ListWidgetPrototype::ListWidgetPrototype(QObject *parent)
    : QObject(parent)
{
}

void ListWidgetPrototype::addItem(const QString &text)
{
    QListWidget *widget = qscriptvalue_cast<QListWidget*>(thisObject());
    if (widget)
        widget->addItem(text);
}

void ListWidgetPrototype::addItems(const QStringList &texts)
{
    QListWidget *widget = qscriptvalue_cast<QListWidget*>(thisObject());
    if (widget)
        widget->addItems(texts);
}

void ListWidgetPrototype::setBackgroundColor(const QString &colorName)
{
    QListWidget *widget = qscriptvalue_cast<QListWidget*>(thisObject());
    if (widget) {
        QPalette palette = widget->palette();
        QColor color(colorName);
        palette.setBrush(QPalette::Base, color);
        widget->setPalette(palette);
    }
}
