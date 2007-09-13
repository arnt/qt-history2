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

#ifndef PARSER_H
#define PARSER_H

#include <QString>

QT_BEGIN_NAMESPACE

class Parser
{
public:
    static QString cleanArgs( const QString &func );
};

QT_END_NAMESPACE

#endif // PARSER_H
