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

#include "formeditor_global.h"

#include <abstractformeditor.h>
#include <abstractwidgetdatabase.h>

#include <QString>
#include <QIcon>

class QObject;

struct WidgetDataBaseItem: public AbstractWidgetDataBaseItem
{
    WidgetDataBaseItem(const QString &name = QString::null,
                       const QString &group = QString::null);

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

    bool isForm() const;
    void setForm(bool b);

    bool isCustom() const;
    void setCustom(bool b);

    QString pluginPath() const;
    void setPluginPath(const QString &path);

    bool isPromoted() const;
    void setPromoted(bool b);
    
    QString extends() const;
    void setExtends(const QString &s);

private:
    QString m_name;
    QString m_group;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_includeFile;
    QString m_pluginPath;
    QIcon m_icon;
    uint m_compat: 1;
    uint m_container: 1;
    uint m_form: 1;
    uint m_custom: 1;
    uint m_promoted: 1;
    QString m_extends;
};

class QT_FORMEDITOR_EXPORT WidgetDataBase: public AbstractWidgetDataBase
{
    Q_OBJECT
public:
    WidgetDataBase(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~WidgetDataBase();
    
    virtual AbstractFormEditor *core() const;    
    
    virtual AbstractWidgetDataBaseItem *item(int index) const;
    int indexOfObject(QObject *o, bool resolveName = true) const;
    
public slots:
    void loadPlugins();

private:
    AbstractFormEditor *m_core;
};

#endif // WIDGETDATABASE_H
