/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qmainwindow.h>

class QStyle;
class QListBox;
class QListBoxItem;
class QWidgetStack;

class Frame : public QMainWindow {
    Q_OBJECT
public:
    Frame( QWidget *parent=0, const char *name=0 );

    void addCategory( QWidget *w, const QPixmap &p, const QString &n );
    void addCategory( QWidget *w, const QPixmap &p1, const QPixmap &p2, const QString &n );

private slots:
    void setStyle( const QString& );
    void clickedCategory( QListBoxItem * );

private:
    QWidget *createCategory( const QString& );
    int idEnglish;

    QString title;
    QListBox *categories;
    QWidgetStack *stack;
};
