/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GAMMAVIEW_H
#define GAMMAVIEW_H

#include <QWidget>

QT_BEGIN_NAMESPACE

class GammaView: public QWidget
{
    Q_OBJECT
public:
    GammaView( QWidget *parent = 0, Qt::WindowFlags f = 0 )
        : QWidget(parent,f) { }
};

QT_END_NAMESPACE

#endif
