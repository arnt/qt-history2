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
