/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3textview.h"

#ifndef QT_NO_TEXTVIEW

/*! 
  \class Q3TextView
  \brief The Q3TextView class provides a rich text viewer.

  \compat

  This class wraps a read-only \l Q3TextEdit.
  Use a \l Q3TextEdit instead, and call setReadOnly(true)
  to disable editing.
*/

/*! \internal */

Q3TextView::Q3TextView(const QString& text, const QString& context,
                      QWidget *parent, const char *name)
    : Q3TextEdit(text, context, parent, name)
{
    setReadOnly(true);
}

/*! \internal */

Q3TextView::Q3TextView(QWidget *parent, const char *name)
    : Q3TextEdit(parent, name)
{
    setReadOnly(true);
}

/*! \internal */

Q3TextView::~Q3TextView()
{
}

/*!
    \property Q3TextView::undoDepth
    \brief the number of undoable steps
*/

/*!
    \property Q3TextView::overwriteMode
    \brief whether new text overwrites or pushes aside existing text
*/

/*!
    \property Q3TextView::modified
    \brief Whether the text view's contents have been modified.
*/

/*!
    \property Q3TextView::readOnly
    \brief Whether the text view's contents are read only.
*/

/*!
    \property Q3TextView::undoRedoEnabled
    \brief Whether undo and redo are enabled.
*/

#endif
