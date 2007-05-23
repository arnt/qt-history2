/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

struct Preferences;

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parentWidget);
    ~PreferencesDialog();

    bool showDialog(Preferences &p);

    static QString chooseTemplatePath(QWidget *parent);

private slots:
    void addTemplatePath();
    void removeTemplatePath();
    void templatePathSelectionChanged();
    void editAppStyleSheet();

private:
    void setPreferences(const Preferences &p);
    void getPreferences(Preferences &p) const;

    Ui::PreferencesDialog *m_ui;
};

#endif // PREFERENCESDIALOG_H
