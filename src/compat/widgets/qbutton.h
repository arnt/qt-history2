/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QBUTTON_H
#define QBUTTON_H

#ifndef QT_H
#include "qabstractbutton.h"
#endif // QT_H

class QButton : public QAbstractButton
{
    Q_OBJECT
public:
    QButton( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~QButton();

protected:
    virtual void drawButton( QPainter * );
    virtual void drawButtonLabel( QPainter * );
    void	paintEvent( QPaintEvent * );

};

#endif
