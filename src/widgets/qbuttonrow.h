/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttonrow.h#3 $
**
** Definition of button row layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUTTONROW_H
#define QBUTTONROW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QGManager;
class QChain;

class QButtonRow : public QWidget
{
    Q_OBJECT
public:
    QButtonRow( QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
    void show();
protected:
    virtual void childEvent( QChildEvent * );
    virtual void layoutEvent( QEvent * );
private:
    void recalc();
    QGManager *gm;
    QChain *par;
    QChain *ser;
    QSize prefSize;
    bool first;
};
#endif //QBUTTONROW_H
