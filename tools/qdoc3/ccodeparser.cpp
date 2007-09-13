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
  ccodeparser.cpp
*/

#include "ccodeparser.h"

QT_BEGIN_NAMESPACE

CCodeParser::CCodeParser()
{
}

CCodeParser::~CCodeParser()
{
}

QString CCodeParser::language()
{
    return "C";
}

QString CCodeParser::headerFileNameFilter()
{
    return "*.ch *.h";
}

QString CCodeParser::sourceFileNameFilter()
{
    return "*.c";
}

QT_END_NAMESPACE
