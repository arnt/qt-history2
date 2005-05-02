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

#ifndef WIDGETDATABASE_H
#define WIDGETDATABASE_H

#include "shared_global.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetdatabase.h>

#include <QtGui/QIcon>
#include <QtCore/QString>
#include <QtCore/QVariant>

class QObject;
class QDesignerCustomWidgetInterface;

class QT_SHARED_EXPORT WidgetDataBaseItem: public QDesignerWidgetDataBaseItemInterface
{
public:
    WidgetDataBaseItem(const QString &name = QString(),
                       const QString &group = QString());

    QString name() const;
    void setName(const QString &name);

    QString group() const;
    void setGroup(const QString &group);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString whatsThis() const;
    void setWhatsThis(const QString &whatsThis);

    QString includeFile() const;
    void setIncludeFile(const QString &includeFile);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    bool isCompat() const;
    void setCompat(bool compat);

    bool isContainer() const;
    void setContainer(bool b);

    bool isCustom() const;
    void setCustom(bool b);

    QString pluginPath() const;
    void setPluginPath(const QString &path);

    bool isPromoted() const;
    void setPromoted(bool b);

    QString extends() const;
    void setExtends(const QString &s);

    void setDefaultPropertyValues(const QList<QVariant> &list);
    QList<QVariant> defaultPropertyValues() const;

private:
    QString m_name;
    QString m_group;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_includeFile;
    QString m_pluginPath;
    QString m_extends;
    QIcon m_icon;
    uint m_compat: 1;
    uint m_container: 1;
    uint m_form: 1;
    uint m_custom: 1;
    uint m_promoted: 1;
    QList<QVariant> m_defaultPropertyValues;
};

class QT_SHARED_EXPORT WidgetDataBase: public QDesignerWidgetDataBaseInterface
{
    Q_OBJECT
public:
    WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~WidgetDataBase();

    virtual QDesignerFormEditorInterface *core() const;

    virtual QDesignerWidgetDataBaseItemInterface *item(int index) const;
    int indexOfObject(QObject *o, bool resolveName = true) const;

    void grabDefaultPropertyValues();

public slots:
    void loadPlugins();

private:
    QList<QVariant> defaultPropertyValues(const QString &name);
    WidgetDataBaseItem *createCustomWidgetItem(QDesignerCustomWidgetInterface *customWidget) const;

    QDesignerFormEditorInterface *m_core;
};

#endif // WIDGETDATABASE_H
