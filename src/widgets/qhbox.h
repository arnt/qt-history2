/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.h#6 $
**
** Definition of hbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QHBOX_H
#define QHBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QBoxLayout;

class QHBox : public QWidget
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0, WFlags f=0 );
    bool event( QEvent * );
    void addStretch();
    void pack();

protected:
    QHBox( bool horizontal, QWidget *parent=0, const char *name=0, WFlags f=0 );
    virtual void childEvent( QChildEvent * );
private:
    void syncLayout();
    void doLayout();
    QBoxLayout *lay;
    bool packed;
};

#endif //QHBOX_H
