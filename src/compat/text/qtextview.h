/****************************************************************************
**
** Definition of the QTextView class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#ifndef QT_H
#include "q3textedit.h"
#endif // QT_H

#ifndef QT_NO_TEXTVIEW

class Q_GUI_EXPORT QTextView : public Q3TextEdit
{
    Q_OBJECT
    Q_OVERRIDE(int undoDepth DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool overwriteMode DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool modified SCRIPTABLE false)
    Q_OVERRIDE(bool readOnly DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false)

public:
    QTextView(const QString& text, const QString& context = QString::null,
               QWidget* parent=0, const char* name=0);
    QTextView(QWidget* parent=0, const char* name=0);

    virtual ~QTextView();

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QTextView(const QTextView &);
    QTextView &operator=(const QTextView &);
#endif
};

#endif //QT_NO_TEXTVIEW
#endif //QTEXTVIEW_H
