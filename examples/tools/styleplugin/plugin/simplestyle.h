/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SIMPLESTYLE_H
#define SIMPLESTYLE_H

#include <QWindowsStyle>

QT_DECLARE_CLASS(QPalette)

class SimpleStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    SimpleStyle() {};

    void polish(QPalette &palette);
};

#endif
