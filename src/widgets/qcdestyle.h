/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcdestyle.h#3 $
**
** Definition of the CDE-like style class
**
** Created : 990513
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/


#ifndef QCDESTYLE_H
#define QCDESTYLE_H

#ifndef QT_H
#include "qmotifstyle.h"
#endif // QT_H

#ifdef QT_FEATURE_STYLE_CDE

class Q_EXPORT QCDEStyle : public QMotifStyle
{
    Q_OBJECT
public:
    QCDEStyle( bool useHighlightCols = FALSE );
    virtual ~QCDEStyle();
    int defaultFrameWidth() const;

    void drawArrow( QPainter *p, ArrowType type, bool down,
		    int x, int y, int w, int h,
		    const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    void drawIndicator( QPainter* p, int x, int y, int w, int h,  const QColorGroup &g,
		       int state, bool down = FALSE, bool enabled = TRUE );

    void drawExclusiveIndicator( QPainter* p,  int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down = FALSE, bool enabled = TRUE );
    
};

#endif // QT_FEATURE_STYLE_CDE

#endif // QCDESTYLE_H
