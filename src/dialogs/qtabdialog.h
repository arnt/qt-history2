/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.h#29 $
**
** Definition of QTabDialog class
**
** Created : 960825
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTABDLG_H
#define QTABDLG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H


class  QTabBar;
struct QTabPrivate;
struct QTab;


class QTabDialog : public QDialog
{
    Q_OBJECT
public:
    QTabDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		WFlags f=0 );
   ~QTabDialog();

    void show();
    void setFont( const QFont & font );

    void addTab( QWidget *, const char * );
    void addTab( QWidget *, QTab* );
    bool isTabEnabled( const char * ) const;
    void setTabEnabled( const char *, bool );

    void showPage( QWidget * );
    const char * tabLabel( QWidget * );

    void setDefaultButton( const char *text = "Defaults" );
    bool hasDefaultButton() const;

    void setCancelButton( const char *text = "Cancel" );
    bool hasCancelButton() const;

    void setApplyButton( const char *text = "Apply" );
    bool hasApplyButton() const;

#if 1 // OBSOLETE
    void setOKButton( const char * text = "OK" );
#endif
    void setOkButton( const char * text = "OK" );
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

    void selected( const char * );

private slots:
    void showTab( int i );

private:
    void setSizes();
    void setUpLayout();
    QRect childRect() const;

    QTabPrivate *d;
};


#endif // QTABDLG_H
