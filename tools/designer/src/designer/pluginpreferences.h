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

#include "../shared/pluginmanager.h"
#include "preferenceinterface.h"

class PluginPreferences : public QObject, public PreferenceInterface
{
    Q_OBJECT
public:
    PluginPreferences(QObject *parent = 0);
    ~PluginPreferences();

    //PreferenceInterface
    QWidget *createPreferenceWidget(QWidget *parent);
    QString preferenceName() const;
    QIcon preferenceIcon() const;
    bool settingsChanged() const;
    bool saveSettings();
    bool readSettings();

signals:
    void setupPlugins(const QStringList &sl);

private slots:
    // All of these are going to work on strings, I don't think it is a big deal as
    // I doubt there could ever be more than 20 plugin paths.
    void addPath(const QString &path);
    void removePath(const QString &path);
    void setPluginPathEnabled(const QString &path, bool enable);

private:
    bool m_dirty;
    PluginManager m_pluginManager;
};

#endif
