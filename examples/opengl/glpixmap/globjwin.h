/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** The GLObjectWindow contains a GLBox and three sliders connected to
** the GLBox's rotation slots.
**
****************************************************************************/

#ifndef GLOBJWIN_H
#define GLOBJWIN_H

#include <qwidget.h>

class GLBox;
class QLabel;
class QPopupMenu;

class GLObjectWindow : public QWidget
{
    Q_OBJECT
public:
    GLObjectWindow( QWidget* parent = 0, const char* name = 0 );

protected slots:

    void		grabFrameBuffer();
    void		makePixmap();
    void		makePixmapHidden();
    void		makePixmapForMenu();
    void		useFixedPixmapSize();

private:
    void		drawOnPixmap( QPixmap* pm );
    GLBox* c1;
    QLabel* lb;
    int fixMenuItemId;
    int insertMenuItemId;
    QSize pmSz;
    QPopupMenu* file;
};


#endif // GLOBJWIN_H
