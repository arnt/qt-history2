/****************************************************************************
** $Id: $
**
** Definition of the QTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#ifndef QT_H
#include "qtextedit.h"
#endif // QT_H

#ifndef QT_NO_TEXTVIEW

class Q_EXPORT QTextView : public QTextEdit
{
    Q_OBJECT
    Q_OVERRIDE( int undoDepth DESIGNABLE false SCRIPTABLE false )
    Q_OVERRIDE( bool overwriteMode DESIGNABLE false SCRIPTABLE false )

public:
#if defined (QT_STRICT_NAMES)
    // unfortunately no default value possible for 'context'
    QTextView( const QString& text, const QString& context,
	       QWidget* Q_PARENT, const char* Q_NAME);
#else
    QTextView( const QString& text, const QString& context = QString::null,
	       QWidget* Q_PARENT, const char* Q_NAME);
#endif
    QTextView( QWidget* Q_PARENT, const char* Q_NAME );

    virtual ~QTextView();

};

#endif //QT_NO_TEXTVIEW
#endif //QTEXTVIEW_H
