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

#ifndef PLUGINPREFERENCES_H
#define PLUGINPREFERENCES_H

#include <QtCore/QCoreVariant>
#include <QtCore/QMap>
#include <QtCore/QString>

#include <pluginmanager.h>
#include "preferenceinterface.h"

class PluginPreferences : public PreferenceInterface
{
    Q_OBJECT
public:
    PluginPreferences(PluginManager *plugin_manager, QObject *parent = 0);
    ~PluginPreferences();

    //PreferenceInterface
    QWidget *createPreferenceWidget(QWidget *parent);
    QString preferenceName() const;
    QIcon preferenceIcon() const;
    bool settingsChanged() const;
    bool saveSettings();
    bool readSettings();

signals:
    void updateWidget();    
    
private:
    bool m_dirty;
    PluginManager *m_plugin_manager;
    QStringList m_plugin_paths, m_disabled_plugins;
    
    friend class PluginPreferenceWidget;
};

#endif
