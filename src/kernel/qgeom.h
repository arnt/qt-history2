/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgeom.h#14 $
**
**  Geometry Management
**
**  Created:  960416
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QGEOM_H
#define QGEOM_H

#include "qbasic.h"

class QLayout : public QObject
{
public:
    int defaultBorder() const { return defBorder; }

    bool doIt() { return bm->doIt(); }
    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

protected:
    QLayout( QWidget *parent,  int border,
	     int autoBorder, const char *name );
    QLayout( QLayout *parent, int autoBorder = -1, const char *name=0 );

    QBasicManager *basicManager() { return bm; }
    virtual QChain *mainVerticalChain() = 0;
    virtual QChain *mainHorizontalChain() = 0;

    static QChain *verChain( QLayout *l ) { return l->mainVerticalChain(); }
    static QChain *horChain( QLayout *l ) { return l->mainHorizontalChain(); }

private:
    QBasicManager * bm;
    int defBorder;
    bool    topLevel;

private:	// Disabled copy constructor and operator=
    QLayout( const QLayout & ) {}
    QLayout &operator=( const QLayout & ) { return *this; }

};


class QBoxLayout : public QLayout
{
public:
    QBoxLayout(	 QWidget *parent, QBasicManager::Direction, int border=0,
		 int autoBorder = -1, const char *name=0 );

    QBoxLayout(	 QLayout *parent, QBasicManager::Direction, int autoBorder = -1,
		 const char *name=0 );

    enum alignment { alignCenter, alignTop, alignLeft,
		     alignBottom, alignRight };

    //QBoxLayout *addNewBox( QBasicManager::Direction, int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, alignment a = alignCenter );
    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addLayout( QLayout *layout, int stretch = 0 );

    QBasicManager::Direction direction() const { return dir; }

    void addStrut( int );
protected:
    QChain *mainVerticalChain();
    QChain *mainHorizontalChain();

private:
    void addB( QLayout *, int stretch );

    QBasicManager::Direction dir;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;

private:	// Disabled copy constructor and operator=
    QBoxLayout( const QBoxLayout & ) : QLayout(0) {}
    QBoxLayout &operator=( const QBoxLayout & ) { return *this; }

};



class QGridLayout : public QLayout
{
public:
    QGridLayout( QWidget *parent, int nRows, int nCols, int border=0,
		 int autoBorder = -1, const char *name=0 );
    QGridLayout( QLayout *parent, int nRows, int nCols, int autoBorder = -1,
		 const char *name=0 );

    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow, 
			       int fromCol, int toCol, int align = 0 );
    void addLayout( QLayout *layout, int row, int col);

    void setRowStretch( int row, int stretch );
    void setColStretch( int col, int stretch );
    //void addStrut( int size, int col);

protected:
    QChain *mainVerticalChain() { return verChain; }
    QChain *mainHorizontalChain() { return horChain; }

private:
    QArray<QChain*> *rows;
    QArray<QChain*> *cols;

    QChain *horChain;
    QChain *verChain;
    void init ( int r, int c );

private:	// Disabled copy constructor and operator=
    QGridLayout( const QGridLayout & ) :QLayout(0) {}
    QGridLayout &operator=( const QGridLayout & ) { return *this; }
};

#endif
