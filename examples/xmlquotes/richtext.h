/****************************************************************************
** $Id: //depot/qt/main/examples/xmlquotes/richtext.h#5 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <qvbox.h>
#include <qstringlist.h>

class QTextView;
class QPushButton;

class MyRichText : public QVBox
{
    Q_OBJECT

public:
    MyRichText( const QStringList &s, QWidget *parent = 0, const char *name = 0 );

protected:
    QTextView *view;
    QPushButton *bClose, *bNext, *bPrev;
    QStringList sayings;
    int num;

protected slots:
    void prev();
    void next();

};

#endif
