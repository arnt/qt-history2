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


/*!
    \class QSvgWidget
    \brief The QSvgWidget class provides a widget that is used to display the contents of
    Scalable Vector Graphics (SVG) files.

    \sa QSvgRenderer, {QtSvg Module}, QPicture
*/

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

/*!
    Constructs a new SVG display widget with the given \a parent.
*/
QSvgWidget::QSvgWidget(QWidget *parent)
    : QWidget(*new QSvgWidgetPrivate, parent, 0)
{
}

/*!
    Constructs a new SVG display widget with the given \a parent and loads the contents
    of the specified \a file.
*/
QSvgWidget::QSvgWidget(const QString &file, QWidget *parent)
    : QWidget(*new QSvgWidgetPrivate(file), parent, 0)
{
}

/*!
    Destroys the widget.
*/
QSvgWidget::~QSvgWidget()
{

}

/*!
    Returns the renderer used to display the contents of the widget.
*/
QSvgRenderer * QSvgWidget::renderer() const
{
    Q_D(const QSvgWidget);
    return d->renderer;
}


/*!
    \reimp
*/
QSize QSvgWidget::sizeHint() const
{
    Q_D(const QSvgWidget);
    if (d->renderer->isValid())
        return d->renderer->defaultSize();
    else
        return QSize(128, 64);
}


/*!
    \reimp
*/
void QSvgWidget::paintEvent(QPaintEvent *)
{
    Q_D(QSvgWidget);
    QPainter p(this);
    d->renderer->render(&p);
}

/*!
    Loads the contents of the SVG \a file and updates the widget.
*/
void QSvgWidget::load(const QString &file)
{
    Q_D(const QSvgWidget);
    d->renderer->load(file);
}

/*!
    Loads the specified SVG format \a contents and updates the widget.
*/
void QSvgWidget::load(const QByteArray &contents)
{
    Q_D(const QSvgWidget);
    d->renderer->load(contents);
}
