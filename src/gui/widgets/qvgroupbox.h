/****************************************************************************
**
** Definition of QVGroupBox widget class.
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

#ifndef QVGROUPBOX_H
#define QVGROUPBOX_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_VGROUPBOX

class Q_GUI_EXPORT QVGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    QVGroupBox(QWidget* parent=0, const char* name=0);
    QVGroupBox(const QString &title, QWidget* parent=0, const char* name=0);

    ~QVGroupBox();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVGroupBox(const QVGroupBox &);
    QVGroupBox &operator=(const QVGroupBox &);
#endif
};

#endif // QT_NO_VGROUPBOX

#endif // QVGROUPBOX_H
