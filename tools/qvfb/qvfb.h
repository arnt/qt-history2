/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmainwindow.h>

class QVFbView;
class QVFbRateDialog;
class QPopupMenu;
class QMenuData;
class QFileDialog;
class Config;

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    QVFb( int display_id, int w, int h, int d, const QString &skin, QWidget *parent = 0,
		const char *name = 0, WFlags = 0 );
    ~QVFb();

    void enableCursor( bool e );
    void popupMenu();

protected slots:
    void saveImage();
    void toggleAnimation();
    void toggleCursor();
    void changeRate();
    void about();
    void aboutQt();

    void configure();

    void setZoom(double);
    void setZoom1();
    void setZoom2();
    void setZoom3();
    void setZoom4();
    void setZoomHalf();

protected:
    void createMenu(QMenuData *menu);

private:
    void init( int display_id, int w, int h, int d, const QString &skin );
    QVFbView *view;
    QVFbRateDialog *rateDlg;
    QFileDialog* imagesave;
    QPopupMenu *viewMenu;
    int cursorId;
    Config* config;
    QString currentSkin;

private slots:
    void setGamma400(int n);
    void setR400(int n);
    void setG400(int n);
    void setB400(int n);
    void updateGammaLabels();
};

