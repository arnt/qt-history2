/****************************************************************************
**
** Definition of QToolBarExtension widget class.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTOOLBAREXTENSION_P_H
#define QTOOLBAREXTENSION_P_H

#include <qtoolbutton.h>

class QToolBarExtension : public QToolButton
{
    Qt::Orientation orientation;

public:
    QToolBarExtension(QWidget *parent);

    void setOrientation(Qt::Orientation o);

    QSize sizeHint() const;
};

#endif // QTOOLBAREXTENSION_P_H
