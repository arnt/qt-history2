/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWSINPUTCONTEXT_P_H
#define QWSINPUTCONTEXT_P_H

#include "qinputcontext.h"

#ifndef QT_NO_QWS_IM

class QWSIMEvent;

class QWSInputContext : public QInputContext
{
    Q_OBJECT
public:
    explicit QWSInputContext(QObject* parent = 0);
    ~QWSInputContext() {}


    QString identifierName() { return QString(); }
    QString language() { return QString(); }

    void reset();
    void update();
    void mouseHandler( int x, QMouseEvent *event);

    void setFocusWidget( QWidget *w );
    void widgetDestroyed(QWidget *w);

    bool isComposing() const { return false; } // ### should be removed

    static QWidget *activeWidget();
    static bool translateIMEvent(QWidget *w, const QWSIMEvent *e);
};
#endif // QT_NO_QWS_IM
#endif
