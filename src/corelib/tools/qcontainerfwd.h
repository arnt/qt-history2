/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCONTAINERFWD_H
#define QCONTAINERFWD_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

template <class Key, class T> class QCache;
template <class Key, class T> class QHash;
template <class T> class QLinkedList;
template <class T> class QList;
template <class Key, class T> class QMap;
template <class Key, class T> class QMultiHash;
template <class Key, class T> class QMultiMap;
template <class T1, class T2> class QPair;
template <class T> class QQueue;
template <class T> class QSet;
template <class T> class QStack;
template<class T, int Prealloc = 256> class QVarLengthArray;
template <class T> class QVector;

QT_END_HEADER

#endif // QCONTAINERFWD_H
