/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgroupbox.h#24 $
**
** Definition of QGroupBox widget class
**
** Created : 950203
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class QAccel;
class QGroupBoxPrivate;
class QVBoxLayout;
class QGridLayout;


class Q_EXPORT QGroupBox : public QFrame
{
    Q_OBJECT
public:
    QGroupBox( QWidget *parent=0, const char *name=0 );
    QGroupBox( const QString &title, QWidget *parent=0, const char* name=0 );
    QGroupBox( int columns, Orientation o, QWidget *parent=0, const char *name=0 );
    QGroupBox( int columns, Orientation o, const QString &title, QWidget *parent=0, const char* name=0 );

    QString title() const { return str; }
    virtual void setTitle( const QString &);

    int alignment() const { return align; }
    virtual void setAlignment( int );

protected:
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void updateMask();

private slots:
    void fixFocus();

private:
    void skip();
    void init();
    void setColumnLayout(int columns, Orientation o);

    QString str;
    int align;
    QAccel * accel;
    QGroupBoxPrivate * d;

    QVBoxLayout *vbox;
    QGridLayout *grid;
    int row;
    int col;
    int nRows, nCols;
    Orientation dir;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGroupBox( const QGroupBox & );
    QGroupBox &operator=( const QGroupBox & );
#endif
};


#endif // QGROUPBOX_H
