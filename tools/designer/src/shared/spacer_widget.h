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

#ifndef SPACER_H
#define SPACER_H

#include "shared_global.h"

#include <QWidget>

class AbstractFormWindow;

class QT_SHARED_EXPORT Spacer: public QWidget
{
    Q_OBJECT

    Q_ENUMS(SizeType)

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(SizeType sizeType READ sizeType WRITE setSizeType)
    Q_PROPERTY(QSize sizeHint READ sizeHint WRITE setSizeHint DESIGNABLE true STORED true)

private:
    enum { HSize = 6, HMask = 0x3f, VMask = HMask << HSize,
           MayGrow = 1, ExpMask = 2, MayShrink = 4 };

public:
    enum SizeType { Fixed = 0,
                    Minimum = MayGrow,
                    Maximum = MayShrink,
                    Preferred = MayGrow | MayShrink,
                    MinimumExpanding = MayGrow | ExpMask,
                    Expanding = MayGrow | MayShrink | ExpMask,
                    Ignored = ExpMask /* magic value */ };

    Spacer(QWidget *parent = 0);

    QSize minimumSize() const;

    QSize sizeHint() const;
    void setSizeHint(const QSize &s);

    Spacer::SizeType sizeType() const;
    void setSizeType(Spacer::SizeType t);

    Qt::Alignment alignment() const;
    Qt::Orientation orientation() const;

    void setOrientation(Qt::Orientation o);
    void setInteraciveMode(bool b) { interactive = b; };

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent* e);
    void updateMask();

private:
    AbstractFormWindow *m_formWindow;
    Qt::Orientation orient;
    bool interactive;
    QSize sh;
};

#endif // SPACER_H
