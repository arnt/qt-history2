/****************************************************************************
**
** Definition of something or other.
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

#ifndef ROT13_H
#define ROT13_H

#include <qwidget.h>

class QMultiLineEdit;

class Rot13: public QWidget {
    Q_OBJECT
public:
    Rot13();

    QString rot13( const QString & ) const;

private slots:
    void changeLeft();
    void changeRight();

private:
    QMultiLineEdit * left, * right;
};

#endif
