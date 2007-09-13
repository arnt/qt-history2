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

#ifndef QTBRUSHPATTERNEDITOR_H
#define QTBRUSHPATTERNEDITOR_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QtBrushPatternEditor : public QWidget
{
    Q_OBJECT
public:
    QtBrushPatternEditor(QWidget *parent = 0);
    ~QtBrushPatternEditor();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

private:
    class QtBrushPatternEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushPatternEditor)
    Q_DISABLE_COPY(QtBrushPatternEditor)
    Q_PRIVATE_SLOT(d_func(), void slotHsvClicked())
    Q_PRIVATE_SLOT(d_func(), void slotRgbClicked())
    Q_PRIVATE_SLOT(d_func(), void slotPatternChanged(int pattern))
    Q_PRIVATE_SLOT(d_func(), void slotChangeColor(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(int color))
};

}

QT_END_NAMESPACE

#endif
