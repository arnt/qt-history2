/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef ARGHINTWIDGET_H
#define ARGHINTWIDGET_H

#include <qframe.h>

class QLabel;
class ArrowButton;

class ArgHintWidget : public QFrame
{
    Q_OBJECT

public:
    ArgHintWidget( QWidget *parent, const char*name );

    void setFunctionText( int func, const QString &text );
    void setNumFunctions( int num );

public slots:
    void relayout();
    void gotoPrev();
    void gotoNext();

private:
    void updateState();

private:
    int curFunc;
    int numFuncs;
    QMap<int, QString> funcs;
    QLabel *funcLabel;
    ArrowButton *prev, *next;

};

#endif
