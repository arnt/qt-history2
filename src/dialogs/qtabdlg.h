/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdlg.h#9 $
**
** Definition of tab dialog
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTABDLG_H
#define QTABDLG_H

#include "qdialog.h"

class QTab;

class QTabDialog : public QDialog
{
    Q_OBJECT

public:
    QTabDialog( QWidget *parent=0, const char *name=0, WFlags f=0 );
    ~QTabDialog();

    void show();
    void setFont( const QFont & font );

    void addTab( QWidget *, const char * );

    void setDefaultButton( const char *text = "Defaults" );
    bool hasDefaultButton() const { return db != 0; }

    void setCancelButton( const char *text = "Cancel" );
    bool hasCancelButton() const { return cb != 0; }

    void setApplyButton( const char *text = "Apply" );
    bool hasApplyButton() const { return ab != 0; }


    bool eventFilter( QObject *, QEvent * );

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

signals:
    void aboutToShow();

    void applyButtonPressed();
    void cancelButtonPressed();
    void defaultButtonPressed();

private:
    void showTab();
    void setSizes();
    QRect childRect();

    QTab * tabs;
    QTab * currentTab;
    QPushButton * ok;
    QPushButton * cb;
    QPushButton * db;
    QPushButton * ab;
    int bh;

    friend class QTab;
};

#endif
