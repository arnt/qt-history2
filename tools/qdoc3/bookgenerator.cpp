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

/*
  bookgenerator.cpp
*/

#include "bookgenerator.h"

QT_BEGIN_NAMESPACE

BookGenerator::BookGenerator()
{
}

BookGenerator::~BookGenerator()
{
}

void BookGenerator::generateTree( const Tree *tree, CodeMarker *marker )
{
    Q_UNUSED( tree )
    Q_UNUSED( marker )
}

QT_END_NAMESPACE
