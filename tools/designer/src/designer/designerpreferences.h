// General Preferences for Qt Designer

#ifndef DESIGNERPREFERENCES_H
#define DESIGNERPREFERENCES_H

#include <QtCore/QObject>
#include <QtGui/QCheckBox>
#include <QtCore/QPointer>
#include "preferenceinterface.h"

class DesignerPreferences : public QObject, public PreferenceInterface
{
    Q_OBJECT
public:
    DesignerPreferences(QObject *parent = 0);
    ~DesignerPreferences();

    //PreferenceInterface
    QWidget *createPreferenceWidget(QWidget *parent);
    QString preferenceName() const;
    QIcon preferenceIcon() const;
    bool settingsChanged() const;
    bool saveSettings();
    bool readSettings();

private slots:
    void setSettingsDirty();
    void setShowDialog(bool enable);

private:
    bool m_dirty;
    bool m_showNewDialog;
    QPointer<QCheckBox> optShowNewDialog;
    QPointer<QWidget> m_prefWidget;
};

#endif
