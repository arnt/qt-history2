/****************************************************************************
** $Id: //depot/qt/2.2/src/kernel/qpainter_p.h#1 $
**
** Definition of some Qt private functions.
**
** Created : 000909
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

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

void qt_format_text( const QFontMetrics& fm, int x, int y, int w, int h,
		     int tf, const QString& str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     char **internal, QPainter* painter );


#endif
