/****************************************************************************
** $Id: //depot/qt/main/examples/kiosk/kioskwidget.h#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#ifndef KIOSKWIDGET_H
#define KIOSKWIDGET_H

#include <qwidget.h>
class QStringList;
class MovieScreen;
class QListBoxItem;
class QListBox;
class QTextBrowser;
class KioskWidget : public QWidget
{
    Q_OBJECT;
public:
    KioskWidget( QStringList files, QWidget *parent=0, const char *name=0 );
    ~KioskWidget();
private slots:
    void handleClick(int,QListBoxItem*);
    void play(QListBoxItem*);
    void browse();
    void play( const QString& );
private:
    MovieScreen *screen;
    QListBox *lb;
    QTextBrowser *browser;
};

#endif
