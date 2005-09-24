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

#include "qsvgwidget.h"

#include "qsvgrenderer.h"

#include "qpainter.h"
#include "private/qwidget_p.h"

class QSvgWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QSvgWidget)
public:
    QSvgWidgetPrivate()
        : QWidgetPrivate()
    {
        Q_Q(QSvgWidget);
        renderer = new QSvgRenderer(q);
    }
    QSvgWidgetPrivate(const QString &file)
        : QWidgetPrivate()
    {
        Q_Q(QSvgWidget);
        renderer = new QSvgRenderer(file, q);
    }
    QSvgRenderer *renderer;
};

QSvgWidget::QSvgWidget(QWidget *parent)
    : QWidget(*new QSvgWidgetPrivate, parent, 0)
{
}

QSvgWidget::QSvgWidget(const QString &file, QWidget *parent)
    : QWidget(*new QSvgWidgetPrivate(file), parent, 0)
{
}

QSvgWidget::~QSvgWidget()
{

}

QSvgRenderer * QSvgWidget::renderer() const
{
    Q_D(const QSvgWidget);
    return d->renderer;
}


QSize QSvgWidget::sizeHint() const
{
    Q_D(const QSvgWidget);
    if (d->renderer->isValid())
        return d->renderer->defaultSize();
    else
        return QSize(128, 64);
}


void QSvgWidget::paintEvent(QPaintEvent *)
{
    Q_D(QSvgWidget);
    QPainter p(this);
    d->renderer->render(&p);
}

void QSvgWidget::load(const QString &file)
{
    Q_D(const QSvgWidget);
    d->renderer->load(file);
}

void QSvgWidget::load(const QByteArray &contents)
{
    Q_D(const QSvgWidget);
    d->renderer->load(contents);
}
