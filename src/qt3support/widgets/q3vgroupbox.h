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

#ifndef Q3VGROUPBOX_H
#define Q3VGROUPBOX_H

#include <q3groupbox.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3VGroupBox : public Q3GroupBox
{
    Q_OBJECT
public:
    Q3VGroupBox( QWidget* parent=0, const char* name=0 );
    Q3VGroupBox( const QString &title, QWidget* parent=0, const char* name=0 );
    ~Q3VGroupBox();

private:
    Q_DISABLE_COPY(Q3VGroupBox)
};

QT_END_HEADER

#endif // Q3VGROUPBOX_H