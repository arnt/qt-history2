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

#ifndef DESIGNERPREFERENCES_H
#define DESIGNERPREFERENCES_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include "preferenceinterface.h"

class DesignerPreferences : public PreferenceInterface
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
    void updateUI(int uimode);

private:
    bool m_dirty;
    bool m_showNewDialog;
    int m_uiMode;
    QPointer<QWidget> m_prefWidget;
};

#endif
