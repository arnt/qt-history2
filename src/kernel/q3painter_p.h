/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTER_P_H
#define QPAINTER_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qpainter.cpp and qfont.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#endif // QT_H

extern void qt_format_text( const QFont& f, const QRect &r,
			    int tf, const QString& str, int len, QRect *brect,
			    int tabstops, int* tabarray, int tabarraylen, QPainter* painter );


#endif
