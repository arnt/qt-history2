/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#6 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"
#include "qpalette.h"

#include <qimage.h>
#include <qapplication.h>

class GradientBackground : public QIconViewBackground
{
public:
    GradientBackground() { }
    virtual ~GradientBackground() {}

    void paint( QPainter *p, const QRect &r, int xOffset, int yOffset, const QSize &s ) {
        if ( sz != s ) {
            makeGradient( pix, Qt::blue, Qt::yellow, s.width(), s.height() );
            sz = s;
        }
        bitBlt(  p->device(), r.x(), r.y(), 
                 &pix, r.x(), r.y(), r.width(), r.height() );
    }

protected:
    void makeGradient( QPixmap &pmCrop, const QColor &_color1, const QColor &_color2, int _xSize, int _ySize ) {
        QColor cRow;
        int rca, gca, bca;
        int rDiff, gDiff, bDiff;
        float rat;
        unsigned int *p;
        unsigned int rgbRow;
        
        pmCrop.resize( _xSize, _ySize );
        QImage image( _xSize, _ySize, 32 );

        rca = _color1.red();
        gca = _color1.green();
        bca = _color1.blue();
        rDiff = _color2.red() - _color1.red();
        gDiff = _color2.green() - _color1.green();
        bDiff = _color2.blue() - _color1.blue();

        for ( int y = _ySize; y > 0; y-- ) {
                p = ( unsigned int* )image.scanLine( _ySize - y );
                rat = 1.0 * y / _ySize;

                cRow.setRgb( rca + (int)( rDiff * rat ),
                             gca + (int)( gDiff * rat ),
                             bca + (int)( bDiff * rat ) );

                rgbRow = cRow.rgb();

                for( int x = 0; x < _xSize; x++ ) {
                        *p = rgbRow;
                        p++;
                    }
            }

        pmCrop.convertFromImage( image );
    }
    
    QPixmap pix;
    QSize sz;
    
};

class ColorBackground : public QIconViewBackground
{
public:
    ColorBackground() {}
    virtual ~ColorBackground() {}

    void paint( QPainter *p, const QRect &rect, int, int, const QSize & ) {
        p->fillRect( rect, QColor( 0, 139, 139 ) );
    }

};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( argc == 2 && QString( "-desktop" ) == argv[1] ) {
	QtFileIconView fiv( QString::null );
	a.setMainWidget( &fiv );
	QPalette pal = fiv.palette();
	fiv.setBackground( new GradientBackground );
        fiv.setFrameStyle( QFrame::NoFrame );
	fiv.setCaption( "desktop" );
	fiv.showMaximized();
        fiv.setSelectionMode( QIconView::StrictMulti );
	fiv.setViewMode( QIconSet::Large );
	fiv.setDirectory( "/" );
	return a.exec();
    } else {
	FileMainWindow mw;
	mw.resize( 680, 480 );
	a.setMainWidget( &mw );
	mw.show();
	mw.fileView()->setDirectory( "/" );
	return a.exec();
    }
}
