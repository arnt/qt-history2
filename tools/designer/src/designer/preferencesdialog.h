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
class FontPanel;
class QListWidget;

class QDialogButtonBox;
class QComboBox;
class QPushButton;

namespace qdesigner_internal {
    class GridPanel;
}

class PreferencesDialog: public QDialog
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget *parentWidget);

    bool showDialog(Preferences &p);

    static QString chooseTemplatePath(QWidget *parent);

private slots:
    void addTemplatePath();
    void removeTemplatePath();
    void templatePathSelectionChanged();

private:
    QWidget *initUIModeControls();
    QWidget *initTemplatePathControls();

    void setPreferences(const Preferences &p);
    void getPreferences(Preferences &p) const;

    QComboBox *m_uiModeCombo;
    FontPanel *m_fontPanel;
    qdesigner_internal::GridPanel *m_gridPanel;
    QListWidget *m_templatePathListWidget;
    QPushButton *m_removeTemplatePathButton;
    QDialogButtonBox *m_dialogButtonBox;
};

#endif // PREFERENCESDIALOG_H
