/****************************************************************************
** $Id$
**
** Global type declarations and definitions
**
** Created : 920529
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN

#if defined(Q_DEFINED_QASCIIDICT) && defined(Q_DEFINED_QCONNECTION_LIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QAsciiDict<QConnectionList>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QAsciiDictIterator<QConnectionList>;
#endif

#if defined(Q_DEFINED_QDICT) && defined(Q_DEFINED_QSTYLESHEET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QDict<QStyleSheetItem>;
#endif

#if defined(Q_DEFINED_QDICT) && defined(Q_DEFINED_QLIBRARY)
Q_TEMPLATE_EXTERN template class Q_EXPORT QDict<QLibrary>; // for Qtopia
#endif

#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QOBJECT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QObject>;
#endif

// needed for Qtopia
#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QWIDGET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QWidget>;
#endif

#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QACCESSIBLE_OBJECT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QAccessibleObject>;
#endif

#if defined(Q_DEFINED_QINTDICT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDict<int>;
#endif

#if defined(Q_DEFINED_QINTDICT) && defined(Q_DEFINED_QWIDGET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDict<QWidget>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDictIterator<QWidget>;
#endif

#if defined(Q_DEFINED_QMAP)
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, int>; // for Qtopia
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, bool>; // for Qtopia
#endif

#if defined(Q_DEFINED_QMAP) && defined(Q_DEFINED_QSTRING)
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<QString, QString>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<QString, int>; // for Qtopia
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, QString>; // for Qtopia
#endif

#if defined(Q_DEFINED_QMEMARRAY)
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<int>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<bool>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<char>;
#endif

#if defined(Q_DEFINED_QMEMARRAY) && defined(Q_DEFINED_QPOINT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<QPoint>;
#endif

#if defined(Q_DEFINED_QPTRLIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<char>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<char>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QWIDGET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QWidget>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QWidget>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QCONNECTION)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QConnection>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QConnection>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QOBJECT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QObject>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QObject>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QDOCKWINDOW)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QDockWindow>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<int>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QSTYLESHEET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QStyleSheetItem>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QWIDGET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QWidget>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QCONNECTION_LIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QConnectionList>;
#endif

#if defined(Q_DEFINED_QVALUELIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<bool>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<int>;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QRECT)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QRect>;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QSTRING)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QString>;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QSTYLESHEET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList< QPtrVector< QStyleSheetItem> >;
#endif
#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QSTYLESHEET)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QStyleSheetItem::ListStyle>;
#endif

// qcanvas module
#if defined( QT_MODULE_CANVAS ) && !defined( QT_LICENSE_PROFESSIONAL ) && !defined( QT_INTERNAL_CANVAS ) && defined(Q_DEFINED_QCANVAS)
#if defined(Q_DEFINED_QPTRLIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList< QCanvasItem >;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList< QCanvasView >;
#endif

#if defined(Q_DEFINED_QVALUELIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList< QCanvasItem* >;
#endif
#endif

// qtable module
#if defined( QT_MODULE_TABLE ) && !defined( QT_LICENSE_PROFESSIONAL ) && !defined( QT_INTERNAL_TABLE )
#if defined (Q_DEFINED_QTABLE_SELECTION) && defined(Q_DEFINED_QPTRLIST)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QTableSelection>;
#endif

#if defined(Q_DEFINED_QTABLE_ITEM) && defined(Q_DEFINED_QPTRVECTOR)
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QTableItem>;
#endif

#if defined(Q_DEFINED_QTABLE) && defined(Q_DEFINED_QPTRVECTOR)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QTable>;
#endif
#endif

// MOC_SKIP_END
#endif // template defined
