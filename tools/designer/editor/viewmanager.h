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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qwidget.h>
#include "dlldefs.h"
#include <qvaluelist.h>

class QChildEvent;
class MarkerWidget;
class QHBoxLayout;
class QDockArea;
class QTextParag;

class EDITOR_EXPORT ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager( QWidget *parent, const char *name );

    void addView( QWidget *view );
    QWidget *currentView() const;
    void showMarkerWidget( bool );

    void setError( int line );
    void setStep( int line );
    void clearStep();
    void setBreakPoints( const QValueList<int> &l );
    QValueList<int> breakPoints() const;

signals:
    void markersChanged();
    void expandFunction( QTextParag *p );
    void collapseFunction( QTextParag *p );
    void collapse( bool all /*else only functions*/ );
    void expand( bool all /*else only functions*/ );

protected slots:
    void clearErrorMarker();

protected:
    void childEvent( QChildEvent *e );
    void resizeEvent( QResizeEvent *e );

private:
    QWidget *curView;
    MarkerWidget *markerWidget;
    QHBoxLayout *layout;
    QDockArea *dockArea;

};

#endif
