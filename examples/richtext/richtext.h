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

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <qvbox.h>

class QTextEdit;
class QPushButton;

class MyRichText : public QVBox
{
    Q_OBJECT

public:
    MyRichText(QWidget *parent = 0);

protected:
    QTextEdit *view;
    QPushButton *bClose, *bNext, *bPrev;
    int num;

protected slots:
    void prev();
    void next();

};

#endif
