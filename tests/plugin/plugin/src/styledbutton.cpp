/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#include "styledbutton.h"
#include <qcolordialog.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qimage.h>

StyledButton::StyledButton( QWidget* parent, const char* name, WFlags f )
    : QButton( parent, name, f ), pix( 0 ), spix( 0 ), s( 0 ), formWindow( 0 )
{
    setMinimumSize( minimumSizeHint() );

    connect( this, SIGNAL(clicked()), SLOT(onEditor()));

    setEditor( ColorEditor );
}

StyledButton::StyledButton( const QBrush& b, QWidget* parent, const char* name, WFlags f )
    : QButton( parent, name, f ), spix( 0 ), s( 0 ), formWindow( 0 )
{
    col = b.color();
    pix = b.pixmap();
    setMinimumSize( minimumSizeHint() );
}

StyledButton::~StyledButton()
{
}

void StyledButton::setEditor( EditorType e )
{
    edit = e;
}

StyledButton::EditorType StyledButton::editor() const
{
    return edit;
}

void StyledButton::setColor( const QColor& c )
{
    col = c;
    update();
}

void StyledButton::setPixmap( const QPixmap & pm )
{
    if ( !pm.isNull() ) {
	delete pix;
	pix = new QPixmap( pm );
    } else {
	delete pix;
	pix = 0;
    }
    scalePixmap();
}

QColor StyledButton::color() const
{
    return col;
}

QPixmap* StyledButton::pixmap() const
{
    return pix;
}

bool StyledButton::scale() const
{
    return s;
}

void StyledButton::setScale( bool on )
{
    if ( s == on )
	return;

    s = on;
    scalePixmap();
}

QSize StyledButton::sizeHint() const
{
    return QSize( 50, 25 );
}

QSize StyledButton::minimumSizeHint() const
{
    return QSize( 50, 25 );
}

void StyledButton::scalePixmap()
{
    delete spix;

    if ( pix ) {
	spix = new QPixmap( width()/2, height()/2 );
	QImage img = pix->convertToImage();

	spix->convertFromImage( s? img.smoothScale( width()/2, height()/2 ) : img );
    } else {
	spix = 0;
    }

    update();
}

void StyledButton::resizeEvent( QResizeEvent* e )
{
    scalePixmap();
    QButton::resizeEvent( e );
}

void StyledButton::drawButton( QPainter *paint )
{
    style().drawBevelButton( paint, 0, 0, width(), height(), colorGroup(), isDown() );
    drawButtonLabel( paint );
    if ( hasFocus() ) {
 	style().drawFocusRect( paint, style().bevelButtonRect( 0, 0, width(), height()),
			       colorGroup(), &colorGroup().button() );
    }
}

void StyledButton::drawButtonLabel( QPainter *paint )
{
    QColor pen = isEnabled() ?
		 hasFocus() ? palette().active().buttonText() : palette().inactive().buttonText()
		 : palette().disabled().buttonText();
    paint->setPen( pen );
    if ( spix ) {
	paint->setBrush( QBrush( col, *spix ) );
	paint->setBrushOrigin( width()/4, height()/4 );
    } else
	paint->setBrush( QBrush( col ) );

    paint->drawRect( width()/4, height()/4, width()/2, height()/2 );
}

void StyledButton::onEditor()
{
    switch (edit) {
    case ColorEditor: {
	QColor col = QColorDialog::getColor( palette().active().background(), this );
	if ( col.isValid() ) {
	    setColor( col );
	    emit changed();
	}
    } break;
    case PixmapEditor:
	break;
    default:
	break;
    }
}
