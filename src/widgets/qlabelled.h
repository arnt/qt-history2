/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabelled.h#3 $
**
** Definition of QLabelled widget class
**
** Created : 980220
**
** Copyright (C) 1995-1998 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLABELLED_H
#define QLABELLED_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QLabelledPrivate;

class QLabelled : public QFrame
{
    Q_OBJECT
public:
    QLabelled( QWidget *parent=0, const char *name=0 );
    QLabelled( const char *, QWidget *parent=0, const char *name=0 );
    ~QLabelled();

    virtual const char* labelText() const;
    QWidget* label() const;
    virtual void setLabel( const char * );
    void setLabel( QWidget* );

    int alignment() const;
    virtual void setAlignment( int );

    bool event( QEvent * );

protected:
    virtual void childEvent( QChildEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual int labelMargin() const;

private:
    QLabelledPrivate* d;
    void init();
    void layout();
    void resetFrameRect();

private:	// Disabled copy constructor and operator=
    QLabelled( const QLabelled & );
    QLabelled &operator=( const QLabelled & );
};


#endif // QLABELLED_H
