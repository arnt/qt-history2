/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.h#2 $
**
** Definition of QDialog class
**
** Author  : Haavard Nord
** Created : 950502
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#include "qview.h"


class QDialog : public QView			// modal dialog widget
{
    Q_OBJECT
public:
    QDialog( QWidget *parent=0, const char *name=0, WFlags f=WType_Modal );
   ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		exec();

    int		result() const		{ return rescode; }

slots:
    virtual void done( int );
    void	accept();
    void	reject();

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );

private:
    int		rescode;
};


#endif // QDIALOG_H
