/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.h#45 $
**
** Definition of QTabDialog class
**
** Created : 960825
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTABDIALOG_H
#define QTABDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#include "qiconset.h"
#endif // QT_H


class  QTabBar;
struct QTabPrivate;
class  QTab;


class Q_EXPORT QTabDialog : public QDialog
{
    Q_OBJECT
public:
    QTabDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		WFlags f=0 );
   ~QTabDialog();

    void show();
    void setFont( const QFont & font );

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
    bool isTabEnabled( const char* ) const; // compatibility
    void setTabEnabled( const char*, bool ); // compatibility

    void showPage( QWidget * );
    void removePage( QWidget * );
    QString tabLabel( QWidget * );

    QWidget * currentPage() const;

    void setDefaultButton( const QString &text );
    void setDefaultButton();
    bool hasDefaultButton() const;

    void setHelpButton( const QString &text );
    void setHelpButton();
    bool hasHelpButton() const;

    void setCancelButton( const QString &text );
    void setCancelButton();
    bool hasCancelButton() const;

    void setApplyButton( const QString &text );
    void setApplyButton();
    bool hasApplyButton() const;

#if 1 // OBSOLETE
    void setOKButton( const QString &text = "OK" );
#endif
    void setOkButton( const QString &text );
    void setOkButton();
    bool hasOkButton() const;

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    void styleChange( GUIStyle );
    void setTabBar( QTabBar* );
    QTabBar* tabBar() const;

signals:
    void aboutToShow();

    void applyButtonPressed();
    void cancelButtonPressed();
    void defaultButtonPressed();
    void helpButtonPressed();

    void selected( const QString& );

private slots:
    void showTab( int i );

private:
    void setSizes();
    void setUpLayout();

    QTabPrivate *d;
};


#endif // QTABDIALOG_H
