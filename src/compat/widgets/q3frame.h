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

#ifndef Q3FRAME_H
#define Q3FRAME_H

#ifndef QT_H
#include "qframe.h"
#endif

class Q_COMPAT_EXPORT Q3Frame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int margin READ margin WRITE setMargin)

public:
    Q3Frame(QWidget* parent, const char* name = 0, Qt::WFlags f = 0);
    ~Q3Frame();
#ifndef qdoc
    bool        lineShapesOk()  const { return true; }
#endif

    int margin() const { return marg; }
    void setMargin(int);

    QRect contentsRect() const;
    int frameWidth() const;

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    virtual void frameChanged();
    virtual void drawFrame(QPainter *);
    virtual void drawContents(QPainter *);

private:
    int marg;
#if defined(Q_DISABLE_COPY)
    Q3Frame(const QFrame &);
    Q3Frame &operator=(const QFrame &);
#endif
};


#endif // Q3FRAME_H
