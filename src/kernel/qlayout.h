/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayout.h#15 $
**
** Definition of layout classes
**
** Created : 960416
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QLAYOUT_H
#define QLAYOUT_H

#include "qgmanagr.h"
#include "qlist.h"

class QMenuBar;

struct QLayoutData;


class QLayout : public QObject
{
    Q_OBJECT
public:
    virtual ~QLayout();
    int defaultBorder() const { return defBorder; }

    enum { unlimited = QCOORD_MAX };

    virtual bool activate();
    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

    void  setMenuBar( QMenuBar *w );

    QWidget *mainWidget();

protected:
    QLayout( QWidget *parent,  int border,
	     int autoBorder, const char *name );
    QLayout( int autoBorder = -1, const char *name=0 );

    QGManager *basicManager() { return bm; }
    virtual QChain *mainVerticalChain() = 0;
    virtual QChain *mainHorizontalChain() = 0;

    virtual void initGM() = 0;
    void addChildLayout( QLayout *);

    static QChain *verChain( QLayout *l ) { return l->mainVerticalChain(); }
    static QChain *horChain( QLayout *l ) { return l->mainHorizontalChain(); }

private:
    QGManager * bm;
    int defBorder;
    bool    topLevel;

    QLayoutData *extraData;
private:	// Disabled copy constructor and operator=
    QLayout( const QLayout & );
    QLayout &operator=( const QLayout & );

};


class QBoxLayout : public QLayout
{
    Q_OBJECT
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
		     Down = TopToBottom, Up = BottomToTop };

    QBoxLayout( QWidget *parent, Direction, int border=0,
		int autoBorder = -1, const char *name=0 );

    QBoxLayout(	Direction, int autoBorder = -1,
		const char *name=0 );

    ~QBoxLayout();

    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, int alignment = AlignCenter );
    void addLayout( QLayout *layout, int stretch = 0 );
    Direction direction() const { return (Direction)dir; }

    void addStrut( int );
protected:
    QChain *mainVerticalChain();
    QChain *mainHorizontalChain();
    void initGM();

private:
    void addB( QLayout *, int stretch );

    QGManager::Direction dir;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;

private:	// Disabled copy constructor and operator=
    QBoxLayout( const QBoxLayout & );
    QBoxLayout &operator=( const QBoxLayout & );

};


class QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout( QWidget *parent, int border=0,
		int autoBorder = -1, const char *name=0 );

    QHBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QHBoxLayout();
};



class QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout( QWidget *parent, int border=0,
		int autoBorder = -1, const char *name=0 );

    QVBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QVBoxLayout();
};



class QGridLayout : public QLayout
{
    Q_OBJECT
public:
    QGridLayout( QWidget *parent, int nRows, int nCols, int border=0,
		 int autoBorder = -1, const char *name=0 );
    QGridLayout( int nRows, int nCols, int autoBorder = -1,
		 const char *name=0 );
    ~QGridLayout();
    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow,
			       int fromCol, int toCol, int align = 0 );
    void addLayout( QLayout *layout, int row, int col);

    void setRowStretch( int row, int stretch );
    void setColStretch( int col, int stretch );
    void addRowSpacing( int row, int minsize );
    void addColSpacing( int col, int minsize );

protected:
    QChain *mainVerticalChain() { return verChain; }
    QChain *mainHorizontalChain() { return horChain; }
    void initGM();

private:
    QArray<QChain*> *rows;
    QArray<QChain*> *cols;

    QChain *horChain;
    QChain *verChain;
    void init ( int r, int c );

    int rr;
    int cc;

private:	// Disabled copy constructor and operator=
    QGridLayout( const QGridLayout & );
    QGridLayout &operator=( const QGridLayout & );
};


#endif // QLAYOUT_H
