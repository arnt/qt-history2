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

#include "qdesigner_preferences.h"

#include <QtCore/QSettings>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

DesignerPreferences::DesignerPreferences(QObject *parent)
    : QObject(parent), m_dirty(false), m_showNewDialog(true)
{
}

DesignerPreferences::~DesignerPreferences()
{
}

QWidget *DesignerPreferences::createPreferenceWidget(QWidget *parent)
{
    if (!m_prefWidget) {
        m_prefWidget = new QWidget(parent);
        QVBoxLayout *layout = new QVBoxLayout(m_prefWidget);
        QLabel *lbl = new QLabel(tr("Designer Preferences"), m_prefWidget);
        optShowNewDialog = new QCheckBox(tr("&Show \"New Form\" dialog on startup"), m_prefWidget);
        optShowNewDialog->setChecked(m_showNewDialog);
        layout->addWidget(lbl);
        layout->addStretch();
        layout->addWidget(optShowNewDialog);
        layout->addStretch();
        connect(optShowNewDialog, SIGNAL(toggled(bool)), this, SLOT(setShowDialog(bool)));
        connect(optShowNewDialog, SIGNAL(toggled(bool)), this, SLOT(setSettingsDirty()));
    }
    return m_prefWidget;
}

bool DesignerPreferences::settingsChanged() const
{
    return m_dirty;
}

bool DesignerPreferences::readSettings()
{
    QSettings settings;
    m_showNewDialog = settings.value("newFormDialog/ShowOnStartup", true).toBool();
    if (settings.status() == QSettings::NoError && optShowNewDialog)
        optShowNewDialog->setChecked(m_showNewDialog);
    return settings.status() == QSettings::NoError;
}

bool DesignerPreferences::saveSettings()
{
    QSettings settings;
    settings.setValue("newFormDialog/ShowOnStartup", m_showNewDialog);
    return settings.status() == QSettings::NoError;
}

void DesignerPreferences::setSettingsDirty()
{
    m_dirty = true;
}

void DesignerPreferences::setShowDialog(bool enable)
{
    m_showNewDialog = enable;
}

QString DesignerPreferences::preferenceName() const
{
    return tr("Qt Designer Preferences");
}

QIcon DesignerPreferences::preferenceIcon() const
{
    return QIcon();
}
