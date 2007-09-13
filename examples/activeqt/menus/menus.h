/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MENUS_H
#define MENUS_H

#include <QMainWindow>

QT_DECLARE_CLASS(QTextEdit)

class QMenus : public QMainWindow
{
    Q_OBJECT

public:
    QMenus(QWidget *parent = 0);

public slots:
    void fileOpen();
    void fileSave();

    void editNormal();
    void editBold();
    void editUnderline();

    void editAdvancedFont();
    void editAdvancedStyle();

    void helpAbout();
    void helpAboutQt();

private:
    QTextEdit *editor;
};

#endif // MENUS_H
