/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.h#13 $
**
** Definition of tab dialog
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
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
    QTabDialog( QWidget *parent=0, const char *name=0, WFlags f=0 );
    ~QTabDialog();

    void show();
    void setFont( const QFont & font );

    void addTab( QWidget *, const char * );
    void setTabEnabled( const char *, bool );
    bool isTabEnabled( const char * );

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
    void showTab( unsigned int i );

private:
    void setSizes();
    QRect childRect();

    QTabPrivate * d;
};

#endif
