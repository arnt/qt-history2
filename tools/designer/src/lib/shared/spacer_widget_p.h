/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef SPACER_WIDGET_H
#define SPACER_WIDGET_H

#include "shared_global_p.h"

#include <QtGui/QWidget>
#include <QtGui/QSizePolicy>

class QDesignerFormWindowInterface;

class QDESIGNER_SHARED_EXPORT Spacer: public QWidget
{
    Q_OBJECT

    Q_ENUMS(SizeType)
    // Special hack: Make name appear as "spacer name"
    Q_PROPERTY(QString spacerName  READ objectName WRITE setObjectName)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QSizePolicy::Policy sizeType READ sizeType WRITE setSizeType)
    Q_PROPERTY(QSize sizeHint READ sizeHint WRITE setSizeHint DESIGNABLE true STORED true)
public:

    Spacer(QWidget *parent = 0);

    QSize sizeHint() const;
    void setSizeHint(const QSize &s);

    QSizePolicy::Policy sizeType() const;
    void setSizeType(QSizePolicy::Policy t);

    Qt::Alignment alignment() const;
    Qt::Orientation orientation() const;

    void setOrientation(Qt::Orientation o);
    void setInteractiveMode(bool b) { m_interactive = b; };

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent* e);
    void updateMask();

private:
    QDesignerFormWindowInterface *m_formWindow;
    Qt::Orientation m_orientation;
    bool m_interactive;
    QSize m_sizeHint;
};

#endif // SPACER_WIDGET_H
