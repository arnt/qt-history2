/****************************************************************************
**

 ** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>

class ViewManager;
class Q3TextParagraph;

class MarkerWidget : public QWidget
{
    Q_OBJECT

public:
    MarkerWidget( ViewManager *parent, const char*name );

signals:
    void markersChanged();
    void expandFunction( Q3TextParagraph *p );
    void collapseFunction( Q3TextParagraph *p );
    void collapse( bool all /*else only functions*/ );
    void expand( bool all /*else only functions*/ );
    void editBreakPoints();
    void isBreakpointPossible( bool &possible, const QString &code, int line );
    void showMessage( const QString &msg );

public slots:
    void doRepaint() { repaint( FALSE ); }

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void contextMenuEvent( QContextMenuEvent *e );

private:
    QPixmap buffer;
    ViewManager *viewManager;

};

#endif
