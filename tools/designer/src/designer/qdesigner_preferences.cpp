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

#include "qdesigner_workbench.h"
#include "qdesigner_preferences.h"
#include "qdesigner_settings.h"

#include "ui_designer_preferences.h"

class DesignerPreferencesWidget : public QWidget
{
    Q_OBJECT
public:
    DesignerPreferencesWidget(QWidget *parent);
    bool setupPreferences();

private slots:
    void on_optShowDialog_toggled(bool toggled);
    void on_optSDI_toggled(bool toggled);

signals:
    void settingsChanged();
    void uiChanged(int); // true for SDI, false for MDI
    void newFormChanged(bool);

private:
    Ui::DesignerPreferences ui;
};

DesignerPreferencesWidget::DesignerPreferencesWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.sdiButton->setIcon(QIcon(":/trolltech/designer/images/sdi.png"));
    ui.mdiButton->setIcon(QIcon(":/trolltech/designer/images/mdi.png"));
}

bool DesignerPreferencesWidget::setupPreferences()
{
    disconnect(this, SIGNAL(uiChanged(int)), this, SIGNAL(settingsChanged()));
    disconnect(this, SIGNAL(newFormChanged(bool)), this, SIGNAL(settingsChanged()));
    QDesignerSettings settings;
    ui.optShowDialog->setChecked(settings.showNewFormOnStartup());
    if (settings.uiMode() == QDesignerWorkbench::TopLevelMode)
        ui.sdiButton->setChecked(true);
    else
        ui.mdiButton->setChecked(true);
    connect(this, SIGNAL(uiChanged(int)), this, SIGNAL(settingsChanged()));
    connect(this, SIGNAL(newFormChanged(bool)), this, SIGNAL(settingsChanged()));
    return settings.status() != QSettings::NoError;
}

void DesignerPreferencesWidget::on_optShowDialog_toggled(bool toggled)
{
    emit newFormChanged(toggled);
}

void DesignerPreferencesWidget::on_optSDI_toggled(bool toggled)
{
    emit uiChanged(toggled ? QDesignerWorkbench::TopLevelMode : QDesignerWorkbench::WorkspaceMode);
}

DesignerPreferences::DesignerPreferences(QObject *parent)
    : PreferenceInterface(parent), m_dirty(false), m_showNewDialog(true), m_uiMode(QDesignerWorkbench::TopLevelMode)
{
}

DesignerPreferences::~DesignerPreferences()
{
}

QWidget *DesignerPreferences::createPreferenceWidget(QWidget *parent)
{
    if (!m_prefWidget) {
        m_prefWidget = new DesignerPreferencesWidget(parent);
        connect(m_prefWidget, SIGNAL(settingsChanged()), this, SLOT(setSettingsDirty()));
        connect(m_prefWidget, SIGNAL(uiChanged(int)), this, SLOT(updateUI(int)));
        connect(m_prefWidget, SIGNAL(newFormChanged(bool)), this, SLOT(setShowDialog(bool)));
    }
    return m_prefWidget;
}

bool DesignerPreferences::settingsChanged() const
{
    return m_dirty;
}

bool DesignerPreferences::readSettings()
{
    bool ret = false;
    if (m_prefWidget)
        ret = static_cast<DesignerPreferencesWidget *>(static_cast<QWidget *>(m_prefWidget))->setupPreferences();
    m_dirty = false;
    return ret;
}

bool DesignerPreferences::saveSettings()
{
    QDesignerSettings settings;
    settings.setShowNewFormOnStartup(m_showNewDialog);
    settings.setUIMode(m_uiMode);
    settings.sync();
    return settings.status() == QSettings::NoError;
}

void DesignerPreferences::setSettingsDirty()
{
    m_dirty = true;
    emit changed();
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

void DesignerPreferences::updateUI(int uimode)
{
    m_uiMode = uimode;
}

#include "qdesigner_preferences.moc"
