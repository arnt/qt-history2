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

#ifndef ABSTRACTMETADATABASE_H
#define ABSTRACTMETADATABASE_H

#include "sdk_global.h"

#include <QObject>
#include <QList>

class QCursor;
class QWidget;

class AbstractFormEditor;

struct AbstractMetaDataBaseItem
{
    virtual ~AbstractMetaDataBaseItem() {}

    virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;

    virtual QString author() const = 0;
    virtual void setAuthor(const QString &author) = 0;

    virtual QString comment() const = 0;
    virtual void setComment(const QString &comment) = 0;

    virtual QCursor cursor() const = 0;
    virtual void setCursor(const QCursor &cursor) = 0;

    virtual QList<QWidget*> tabOrder() const = 0;
    virtual void setTabOrder(const QList<QWidget*> &tabOrder) = 0;

    virtual int spacing() const = 0;
    virtual void setSpacing(int spacing) = 0;

    virtual int margin() const = 0;
    virtual void setMargin(int margin) = 0;
    
    virtual bool enabled() const = 0;
    virtual void setEnabled(bool b) = 0;
};

class QT_SDK_EXPORT AbstractMetaDataBase: public QObject
{
    Q_OBJECT
public:
    AbstractMetaDataBase(QObject *parent = 0);
    virtual ~AbstractMetaDataBase();

    virtual AbstractMetaDataBaseItem *item(QObject *object) const = 0;
    virtual void add(QObject *object) = 0;
    virtual void remove(QObject *object) = 0;

    virtual QList<QObject*> objects() const = 0;

    void setPropertyChanged(QObject *o, const QString &propertyName, bool changed = true);
    bool isPropertyChanged(QObject *o, const QString &propertyName) const;

    virtual AbstractFormEditor *core() const = 0;

signals:
    void changed();
};

#endif // ABSTRACTMETADATABASE_H
