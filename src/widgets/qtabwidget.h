/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabwidget.h#2 $
**
** Definition of QTabWidget class
**
** Created : 990318
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

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class  QTabBar;
class QTabWidgetData;
struct QTab;
class QWidgetStack;


class Q_EXPORT QTabWidget : public QWidget
{
    Q_OBJECT
public:
    QTabWidget( QWidget *parent=0, const char *name=0);
   ~QTabWidget();

    void addTab( QWidget *, const QString &);
    void addTab( QWidget *, QTab* );
    bool isTabEnabled( const QString &) const;
    void setTabEnabled( const QString &, bool );

    void showPage( QWidget * );
    QString tabLabel( QWidget * );

    QWidget * currentPage() const;

    virtual QSize	sizeHint() const;

    enum TabPosition { Top, Bottom };
    TabPosition tabPosition() const;
    void setTabPosition( TabPosition );

    int margin() const;
    void setMargin( int );


protected:
    void showEvent( QShowEvent * );
    void resizeEvent( QResizeEvent * );
    void setTabBar( QTabBar* );
    QTabBar* tabBar() const;
    void styleChange( GUIStyle );
    
signals:
    void selected( const QString& );

private slots:
    void showTab( int i );

private:
    QTabWidgetData *d;
    void setUpLayout(bool = FALSE);
};


#endif
