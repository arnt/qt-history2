/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.h#35 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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
#endif // QT_H


class  QTabBar;
struct QTabPrivate;
struct QTab;


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
    void addTab( QWidget *, QTab* );
    bool isTabEnabled( const QString &) const;
    void setTabEnabled( const QString &, bool );

    void showPage( QWidget * );
    QString tabLabel( QWidget * );

    void setDefaultButton( const QString &text = "Defaults" );
    bool hasDefaultButton() const;

    void setCancelButton( const QString &text = "Cancel" );
    bool hasCancelButton() const;

    void setApplyButton( const QString &text = "Apply" );
    bool hasApplyButton() const;

#if 1 // OBSOLETE
    void setOKButton( const QString &text = "OK" );
#endif
    void setOkButton( const QString &text = "OK" );
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

    void selected( const QString& );

private slots:
    void showTab( int i );

private:
    void setSizes();
    void setUpLayout();
    QRect childRect() const;

    QTabPrivate *d;
};


#endif // QTABDIALOG_H
