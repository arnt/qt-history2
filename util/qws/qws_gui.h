/****************************************************************************
**
** Definition of Qt/FB dummy framebuffer debug GUI.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWS_GUI_H
#define QWS_GUI_H

#include <qimage.h>
#include <qmainwindow.h>

class QWSServer;
class QPopupMenu;

class DummyFramebuffer : public QWidget {
    QImage img;
    QImage oldimg;
    QWSServer *server;
    bool showregions;
    QWidget* zoombox;

public:
    DummyFramebuffer( QWidget* parent );

    void serve(int depth,int refresh_delay);

    const QImage& image() const { return img; }

    void setRegionDisplay(bool y);
    void setZoomBox(bool y);

protected:
    void timerEvent(QTimerEvent*);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void sendMouseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
    void keyEvent(QKeyEvent* e);
    void paintEvent(QPaintEvent* e);
    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;
};

class DebuggingGUI : public QMainWindow {
    Q_OBJECT

public:
    DebuggingGUI();

    void serve(int depth, int refresh_delay) { fb->serve(depth,refresh_delay); }

private slots:
    void screendump();
    void toggleRegions();
    void toggleZoomBox();

private:
    DummyFramebuffer* fb;
    QPopupMenu* view;
    int show_client_regions_id;
    int show_zoom_box_id;

    bool showRegions() const;
};

#endif
