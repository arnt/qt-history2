/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QWidget>
#include <QImage>
#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

class QPaintEvent;
class QSvgRenderer;
class QWheelEvent;

class SvgRasterView : public QWidget
{
    Q_OBJECT

public:
    SvgRasterView(const QString &file, QWidget *parent=0);

    virtual QSize sizeHint() const;

protected slots:
    void poluteImage();
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    QSvgRenderer *doc;
    QImage buffer;
    bool m_dirty;
};

class SvgNativeView : public QWidget
{
    Q_OBJECT

public:
    SvgNativeView(const QString &file, QWidget *parent=0);

    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    QSvgRenderer *doc;
};

#ifndef QT_NO_OPENGL
class SvgGLView : public QGLWidget
{
    Q_OBJECT

public:
    SvgGLView(const QString &file, QWidget *parent=0);

    void setFastAntialiasing(bool fastAntialiasing);

    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    QSvgRenderer *doc;

    bool fastAntialiasing;
};
#endif

#endif
