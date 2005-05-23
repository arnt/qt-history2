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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QAction;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();

private:
    void createMenu();
    void createHorizontalGroupBox();
    void createGridGroupBox();

    enum { NumGridRows = 3, NumButtons = 4 };

    QMenuBar *menuBar;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QTextEdit *smallEditor;
    QTextEdit *bigEditor;
    QLabel *labels[NumGridRows];
    QLineEdit *lineEdits[NumGridRows];
    QPushButton *buttons[NumButtons];
    QPushButton *okButton;
    QPushButton *cancelButton;

    QMenu *fileMenu;
    QAction *exitAction;
};

#endif
