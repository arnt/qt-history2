/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.h#14 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/


#ifndef QHBOX_H
#define QHBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_WIDGETS

#include "qframe.h"

class QBoxLayout;

class Q_EXPORT QHBox : public QFrame
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE  );
    void setSpacing( int );
    bool setStretchFactor( QWidget*, int stretch );
    QSize sizeHint() const;

protected:
    QHBox( bool horizontal, QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE  );
    void frameChanged();

private:
    QBoxLayout *lay;
};

#endif // QT_NO_WIDGETS

#endif // QHBOX_H
