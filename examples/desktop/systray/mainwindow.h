/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QSystemTrayIcon>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void updateMenu();
    void toggleVisibility();
    void showMessage();
    void balloonClicked();
    void activated(int);
    void changeIcon(int);

private:
    QLineEdit *titleEdit;
    QTextEdit *msgEdit;
    QComboBox *typeCombo;
    QSpinBox *timeoutSpin;
    QSystemTrayIcon *trayIcon;
    QAction *toggleVisibilityAction;
    QMenu *menu;
    QTextEdit *info;
    QComboBox *iconPicker;
};

#endif // MAINWINDOW_H

