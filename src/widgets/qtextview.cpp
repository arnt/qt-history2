/****************************************************************************
**
** Implementation of the QTextView class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextview.h"

#ifndef QT_NO_TEXTVIEW

/*! \class QTextView
    \brief The QTextView class provides a rich-text viewer.

  \obsolete

  This class wraps a read-only \l QTextEdit.
  Use a \l QTextEdit instead, and call setReadOnly(TRUE)
  to disable editing.
*/

/*! \reimp */

QTextView::QTextView( const QString& text, const QString& context,
		      QWidget *parent, const char *name )
    : QTextEdit( text, context, parent, name )
{
    setReadOnly( TRUE );
}

/*! \reimp */

QTextView::QTextView( QWidget *parent, const char *name )
    : QTextEdit( parent, name )
{
    setReadOnly( TRUE );
}

/*! \reimp */

QTextView::~QTextView()
{
}

/*!
    \property QTextView::undoDepth
    \brief the number of undoable steps
*/

/*!
    \property QTextView::overwriteMode
    \brief whether new text overwrites or pushes aside existing text
*/

/*!
    \property QTextView::modified
    \brief Whether the text view's contents have been modified.
*/

/*!
    \property QTextView::readOnly
    \brief Whether the text view's contents are read only.
*/

/*!
    \property QTextView::undoRedoEnabled
    \brief Whether undo and redo are enabled.
*/

#endif
