/****************************************************************************
**
** Definition of QFrame widget class.
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

#ifndef QFRAME_H
#define QFRAME_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QFramePrivate;

class Q_GUI_EXPORT QFrame : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFrame)

    Q_ENUMS(Shape Shadow)
    Q_PROPERTY(Shape frameShape READ frameShape WRITE setFrameShape)
    Q_PROPERTY(Shadow frameShadow READ frameShadow WRITE setFrameShadow)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(int midLineWidth READ midLineWidth WRITE setMidLineWidth)
    QDOC_PROPERTY(int frameWidth READ frameWidth)
    QDOC_PROPERTY(QRect frameRect READ frameRect WRITE setFrameRect)

public:
    QFrame(QWidget* parent = 0, Qt::WFlags f = 0);
    ~QFrame();

    int frameStyle()    const;
    void setFrameStyle(int);

    int frameWidth()    const;

    QSize sizeHint() const;

    enum Shape {
        NoFrame  = 0, // no frame
        Box = 0x0001, // rectangular box
        Panel = 0x0002, // rectangular panel
        WinPanel = 0x0003, // rectangular panel (Windows)
        HLine = 0x0004, // horizontal line
        VLine = 0x0005, // vertical line
        StyledPanel = 0x0006, // rectangular panel depending on the GUI style

        PopupPanel = 0x0007, // rectangular panel depending on the GUI style
        MenuBarPanel = 0x0008,
        ToolBarPanel = 0x0009,
        LineEditPanel = 0x000a,
        TabWidgetPanel = 0x000b,
        GroupBoxPanel = 0x000c,

        MShape = 0x000f // mask for the shape
    };
    enum Shadow {
        Plain = 0x0010, // plain line
        Raised = 0x0020, // raised shadow effect
        Sunken = 0x0030, // sunken shadow effect
        MShadow = 0x00f0 // mask for the shadow
    };

    Shape frameShape()    const;
    void setFrameShape(Shape);
    Shadow frameShadow()   const;
    void setFrameShadow(Shadow);

    int lineWidth()     const;
    void setLineWidth(int);

    int midLineWidth()  const;
    void setMidLineWidth(int);

    QRect frameRect()     const;
    void setFrameRect(const QRect &);

protected:
    QFrame(QFramePrivate &, QWidget* parent, Qt::WFlags f = 0);
    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);

    void drawFrame(QPainter *);

private:
#if defined(Q_DISABLE_COPY)
    QFrame(const QFrame &);
    QFrame &operator=(const QFrame &);
#endif

#ifdef QT_COMPAT
public:
    QFrame(QWidget* parent, const char* name, Qt::WFlags f = 0);
#endif
};

#endif // QFRAME_H
