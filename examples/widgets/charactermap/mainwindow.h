/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QClipboard;
class QComboBox;
class QLineEdit;
class QScrollArea;
class CharacterWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void findStyles();
    void insertCharacter(const QString &character);
    void updateClipboard();

private:
    void findFonts();

    CharacterWidget *characterWidget;
    QClipboard *clipboard;
    QComboBox *fontCombo;
    QComboBox *styleCombo;
    QLineEdit *lineEdit;
    QScrollArea *scrollArea;
};

#endif
