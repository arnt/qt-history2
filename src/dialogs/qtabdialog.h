/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.h#16 $
**
** Definition of QTabDialog class
**
** Created : 960825
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTABDLG_H
#define QTABDLG_H

#include "qdialog.h"


struct QTabPrivate;

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
    bool isTabEnabled( const char * ) const;
    void setTabEnabled( const char *, bool );

    void setDefaultButton( const char *text = "Defaults" );
    bool hasDefaultButton() const;

    void setCancelButton( const char *text = "Cancel" );
    bool hasCancelButton() const;

    void setApplyButton( const char *text = "Apply" );
    bool hasApplyButton() const;

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

signals:
    void aboutToShow();

    void applyButtonPressed();
    void cancelButtonPressed();
    void defaultButtonPressed();

private slots:
    void showTab( int i );

private:
    void setSizes();
    QRect childRect() const;

    QTabPrivate *d;
};


#endif // QTABDLG_H
