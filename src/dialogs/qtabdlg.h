/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdlg.h#4 $
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

    void addTab( QWidget *, const char * );

    void setDefaultButton( bool enable );
    bool hasDefaultButton() { return db != 0; }

    void setCancelButton( bool enable );
    bool hasCancelButton() { return cb != 0; }

    bool eventFilter( QObject *, QEvent * );

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

signals:
    void apply();
    void collect();
    void defaultButtonPressed();
    void cancelButtonPressed();
    void okButtonPressed();

private:
    void showTab();
    void setSizes();

    QTab * tabs;
    QTab * currentTab;
    QPushButton * ok;
    QPushButton * cb;
    QPushButton * db;
    QRect childRect;
    int bh;

    friend class QTab;
};



#endif
