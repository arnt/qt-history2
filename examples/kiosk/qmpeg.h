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
#ifndef QMPEG_H
#define QMPEG_H

#include <qframe.h>

class QMpegWidgetPrivate;
typedef struct vid_stream VidStream;
extern "C" void ExecuteDisplay(VidStream *vid_stream);

class QMpegWidget : public QFrame {
    Q_OBJECT;
public:
    QMpegWidget( QWidget* parent=0, const char* name=0 );

    void stop();
    void play(const QString& filename);
    bool isPlaying() const;
    
    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;
signals:
    void finished();
    void stopped();
protected:
    void timerEvent(QTimerEvent*);
    void resizeEvent( QResizeEvent*);
private:
    friend void ExecuteDisplay(VidStream *vid_stream);
    void showFrame(const QImage&, int ms);
    int timer;
    QMpegWidgetPrivate* d;
    QPoint offset;
};

#endif
