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

#ifndef ANIMATEDLABEL_H
#define ANIMATEDLABEL_H

#include <qwidget.h>
#include <qdatetime.h>
#include <qasyncimageio.h>

class QFile;

class AnimatedLabel : public QWidget, QImageConsumer {
    Q_OBJECT

public:
    AnimatedLabel(QWidget* parent=0, const char* name=0);
    AnimatedLabel(const QString& animfile, QWidget* parent=0, const char* name=0);
    ~AnimatedLabel();

    bool start(const QString& animfile);

    int status() const;

    bool playing() const;

    QSize sizeHint() const;

signals:
    void finished();

public slots:
    void pushUpdate();
    void restart();

protected:
    void timerEvent(QTimerEvent*);

private:
    void showNextFrame();

    QImageDecoder *decoder;
    QFile* input;
    int tid;
    QSize sh;
    int loops;
    int loop;
    QRect change;
    QImage type2buffer;
    QPtrList<QPixmap> frame;
    QPtrList<QPoint> offset;
    int gotframes;
    int playframe;
    QTime timer;
    int period;

    // QImageConsumer interface
    virtual void end();
    // Change transfer type 1.
    virtual void changed( const QRect& );
    virtual void frameDone();
    // Change transfer type 2.
    virtual void frameDone( const QPoint&, const QRect& );

    virtual void setLooping( int );
    virtual void setFramePeriod( int );
    virtual void setSize( int, int );
};

#endif
