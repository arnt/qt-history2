/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MENU_H
#define MENU_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qlabel.h>


class MenuExample : public QWidget
{
    Q_OBJECT
public:
    MenuExample( QWidget *parent=0 );

public slots:
    void open();
    void news();
    void save();
    void closeDoc();
    void undo();
    void redo();
    void normal();
    void bold();
    void underline();
    void about();
    void aboutQt();
    void printer();
    void file();
    void fax();
    void printerSetup();

protected:
    void    resizeEvent( QResizeEvent * );

signals:
    void    explain( const QString& );

private:
    void contextMenuEvent ( QContextMenuEvent * );


    QMenuBar *menu;
    QLabel   *label;
    bool isBold;
    bool isUnderline;
    QAction *boldAct, *underlineAct;
};


#endif // MENU_H
