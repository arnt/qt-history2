/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ARGHINTWIDGET_H
#define ARGHINTWIDGET_H

#include <qframe.h>
#include <qmap.h>

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
