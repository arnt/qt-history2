/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabwidget.h#12 $
**
** Definition of QTabWidget class
**
** Created : 990318
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#ifndef QT_H
#include "qwidget.h"
#include "qiconset.h"
#endif // QT_H

class QTabBar;
class QTabWidgetData;
class QTab;
class QWidgetStack;


class Q_EXPORT QTabWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS( TabPosition )
    Q_PROPERTY( TabPosition tabPosition READ tabPosition WRITE setTabPosition )
    Q_PROPERTY( int margin READ margin WRITE setMargin )

public:
    QTabWidget( QWidget *parent, const char *name, WFlags f);
    QTabWidget( QWidget *parent=0, const char *name=0);
   ~QTabWidget();

    void addTab( QWidget *, const QString &);
    void addTab( QWidget *child, const QIconSet& iconset, const QString &label);
    void addTab( QWidget *, QTab* );

    void insertTab( QWidget *, const QString &, int index = -1);
    void insertTab( QWidget *child, const QIconSet& iconset, const QString &label, int index = -1);
    void insertTab( QWidget *, QTab*, int index = -1 );

    void changeTab( QWidget *, const QString &);
    void changeTab( QWidget *child, const QIconSet& iconset, const QString &label);

    bool isTabEnabled(  QWidget * ) const;
    void setTabEnabled( QWidget *, bool );

    void showPage( QWidget * );
    void removePage( QWidget * );
    QString tabLabel( QWidget * );

    QWidget * currentPage() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

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
    void styleChange( QStyle& );
    void updateMask();
    bool eventFilter( QObject *, QEvent * );

signals:
    void selected( const QString& );

private slots:
    void showTab( int i );

private:
    QTabWidgetData *d;
    void setUpLayout(bool = FALSE);
    void init();

#if 1 //def TOTAL_LOSER_COMPILER
    friend class QTabDialog;
#else
    friend void QTabDialog::setTabBar( QTabBar* );
    friend void QTabBar* QTabDialog::tabBar() const;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTabWidget( const QTabWidget & );
    QTabWidget& operator=( const QTabWidget & );
#endif
};


#endif
