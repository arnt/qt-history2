/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/glpixmap/globjwin.h#1 $
**
** Definition of GLObjectWindow widget class
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

    void		makePixmap();
    void		makePixmapManually();
    void		makePixmapHidden();
    void		makePixmapHiddenManually();
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
