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

#ifndef ABSTRACTWIDGETDATABASE_H
#define ABSTRACTWIDGETDATABASE_H

#include "sdk_global.h"

#include <QObject>
#include <QList>

class QIcon;
class QString;
class AbstractFormEditor;

class AbstractWidgetDataBaseItem
{
public:
    virtual ~AbstractWidgetDataBaseItem() {}

    virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;

    virtual QString group() const = 0;
    virtual void setGroup(const QString &group) = 0;

    virtual QString toolTip() const = 0;
    virtual void setToolTip(const QString &toolTip) = 0;

    virtual QString whatsThis() const = 0;
    virtual void setWhatsThis(const QString &whatsThis) = 0;

    virtual QString includeFile() const = 0;
    virtual void setIncludeFile(const QString &includeFile) = 0;

    virtual QIcon icon() const = 0;
    virtual void setIcon(const QIcon &icon) = 0;

    virtual bool isCompat() const = 0;
    virtual void setCompat(bool compat) = 0;

    virtual bool isContainer() const = 0;
    virtual void setContainer(bool container) = 0;

    virtual bool isForm() const = 0;
    virtual void setForm(bool form) = 0;

    virtual bool isCustom() const = 0;
    virtual void setCustom(bool custom) = 0;

    virtual QString pluginPath() const = 0;
    virtual void setPluginPath(const QString &path) = 0;

    virtual bool isPromoted() const = 0;
    virtual void setPromoted(bool b) = 0;

    virtual QString extends() const = 0;
    virtual void setExtends(const QString &s) = 0;
};

class QT_SDK_EXPORT AbstractWidgetDataBase: public QObject
{
    Q_OBJECT
public:
    AbstractWidgetDataBase(QObject *parent = 0);
    virtual ~AbstractWidgetDataBase();

    virtual int count() const;
    virtual AbstractWidgetDataBaseItem *item(int index) const;

    virtual int indexOf(AbstractWidgetDataBaseItem *item) const;
    virtual void insert(int index, AbstractWidgetDataBaseItem *item);
    virtual void append(AbstractWidgetDataBaseItem *item);

    virtual int indexOfObject(QObject *object, bool resolveName = true) const;
    virtual int indexOfClassName(const QString &className, bool resolveName = true) const;

    virtual AbstractFormEditor *core() const;

    bool isContainer(QObject *object, bool resolveName = true) const;
    bool isForm(QObject *object, bool resolveName = true) const;
    bool isCustom(QObject *object, bool resolveName = true) const;

signals:
    void changed();

protected:
    QList<AbstractWidgetDataBaseItem *> m_items;
};

#endif // ABSTRACTWIDGETDATABASE_H
