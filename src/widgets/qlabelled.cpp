/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabelled.cpp#3 $
**
** Implementation of QLabelled widget class
**
** Created : 980220
**
** Copyright (C) 1995-1998 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabelled.h"
#include "qlayout.h"
#include "qlabel.h"

class QLabelledPrivate {
public:
    QLabelledPrivate(QLabelled* parent)
    {
	QLabel *l = new QLabel(parent, "label");
	l->setMargin(2);

	label = l;
	child = 0;
	align = AlignTop;
	grid = 0;
    }

    QWidget* label;
    QWidget* child;
    int align;
    QGridLayout* grid;
};

QLabelled::QLabelled( QWidget *parent, const char *name ) :
    QFrame(parent, name)
{
    init();
}

QLabelled::QLabelled( const char *label, QWidget *parent, const char *name ) :
    QFrame(parent, name)
{
    init();
    setLabel(label);
}

void QLabelled::init()
{
    setFrameStyle( QFrame::Box | QFrame::Sunken );
    setMargin(9);
    d = new QLabelledPrivate(this);
    layout();
    resetFrameRect();
}

QLabelled::~QLabelled()
{
    delete d;
}

const char* QLabelled::labelText() const
{
    if (d->label->inherits("QLabel"))
	return ((QLabel*)d->label)->text();
    else
	return 0;
}

QWidget* QLabelled::label() const
{
    return d->label;
}

void QLabelled::setLabel( const char *text )
{
    if (d->label->inherits("QLabel"))
	((QLabel*)d->label)->setText(text);
}

void QLabelled::setLabel( QWidget* label )
{
    bool v = d->label->isVisible();
    delete d->label;
    d->label = label;
    recreate( this, 0, QPoint(0,0), v );
    layout();
}

int QLabelled::alignment() const
{
    return d->align;
}

void QLabelled::setAlignment( int align )
{
    if ( d->align != align ) {
	ASSERT( align == AlignTop || align == AlignLeft );
	d->align = align;
	layout();
	resetFrameRect();
	update();
    }
}

int QLabelled::labelMargin() const
{
    if (d->label->inherits("QLabel"))
	return ((QLabel*)d->label)->margin();
    else
	return 0;
}

void QLabelled::layout()
{
    delete d->grid;

    QSize sh = d->label->sizeHint();
    if (sh.isValid() && !sh.isEmpty())
	d->label->setFixedSize(sh);

    if ( d->align == AlignTop ) {
	d->grid = new QGridLayout(this,4,4);
	d->grid->addMultiCellWidget( d->label, 0, 0, 1, 2, AlignLeft );
	if (d->child) {
	    d->grid->addWidget( d->child, 2, 2, AlignLeft );
	    d->grid->setRowStretch( 2, 1 );
	    d->grid->setColStretch( 2, 1 );
	}
	d->grid->addRowSpacing( 1, margin()/2 );
	d->grid->addColSpacing( 0, frameWidth()-labelMargin() );
	d->grid->addColSpacing( 1, labelMargin() );
	d->grid->addRowSpacing( 3, frameWidth() );
	d->grid->addColSpacing( 3, frameWidth() );
    } else {
	d->grid = new QGridLayout(this,4,4);
	d->grid->addMultiCellWidget( d->label, 1, 2, 0, 0, AlignTop );
	if (d->child) {
	    d->grid->addWidget( d->child, 2, 2, AlignTop );
	    d->grid->setRowStretch( 2, 1 );
	    d->grid->setColStretch( 2, 1 );
	}
	d->grid->addColSpacing( 1, margin()/2 );
	d->grid->addRowSpacing( 0, frameWidth()-labelMargin() );
	d->grid->addRowSpacing( 1, labelMargin() );
	d->grid->addRowSpacing( 3, frameWidth() );
	d->grid->addColSpacing( 3, frameWidth() );
    }

    d->grid->activate();
}

void QLabelled::resetFrameRect()
{
    QRect f = rect();

    if ( d->align == AlignTop ) {
	f.setTop( d->label->height()/2 );
	setFrameRect(f);
    } else {
	f.setLeft( d->label->width()/2 );
	setFrameRect(f);
    }
}

void QLabelled::resizeEvent( QResizeEvent * )
{
    resetFrameRect();
}

void QLabelled::childEvent( QChildEvent *e )
{
    if (d->label == e->child())
	return; // Yes, we know all about it.

    if (d->child && e->inserted())
	warning("QLabelled should only have one child inserted.");

    if (e->inserted())
	d->child = e->child();
    else
	d->child = 0;

    layout();
}

/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QLabelled::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
        childEvent( (QChildEvent*)e );
        return TRUE;
    default:
        return QWidget::event( e );
    }
}

