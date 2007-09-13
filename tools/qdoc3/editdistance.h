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
  editdistance.h
*/

#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <QSet>
#include <QString>

QT_BEGIN_NAMESPACE

int editDistance( const QString& s, const QString& t );
QString nearestName( const QString& actual, const QSet<QString>& candidates );

QT_END_NAMESPACE

#endif
