/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QWidget>

class QPushButton;
class QTextEdit;
class QLineEdit;

class TextFinder : public QWidget
{
    Q_OBJECT

public:
    TextFinder(QWidget *parent = 0);

private slots:
    void on_findButton_clicked();
    
private:
    QWidget* loadUiFile();
    void loadTextFile();

    QPushButton *ui_findButton;
    QTextEdit *ui_textEdit;
    QLineEdit *ui_lineEdit;
    bool isFirstTime;
};

#endif
