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

#ifndef Q3TEXTVIEW_H
#define Q3TEXTVIEW_H

#include "Qt3Support/q3textedit.h"

#ifndef QT_NO_TEXTVIEW

class Q_COMPAT_EXPORT Q3TextView : public Q3TextEdit
{
    Q_OBJECT

public:
    Q3TextView(const QString& text, const QString& context = QString(),
               QWidget* parent=0, const char* name=0);
    Q3TextView(QWidget* parent=0, const char* name=0);

    virtual ~Q3TextView();

private:
    Q_DISABLE_COPY(Q3TextView)
};

#endif //QT_NO_TEXTVIEW
#endif //QTEXTVIEW_H
