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

#ifndef QVFB_H
#define QVFB_H

#include <QMainWindow>
#include <QStringList>

class QVFbView;
class QVFbRateDialog;
class QPopupMenu;
class QMenuData;
class Q3FileDialog;
class QAction;
class QActionGroup;
class Config;
class Skin;
class QVFb;
class QLabel;
class Q3PopupMenu;
class QScrollArea;

class Zoomer : public QWidget {
    Q_OBJECT
public:
    Zoomer(QVFb* target);

private slots:
    void zoom(int);

private:
    QVFb *qvfb;
    QLabel *label;
};

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    QVFb(QWidget *parent = 0, const char *name = 0, uint wflags = 0);
    ~QVFb();

    void enableCursor( bool e );
    void popupMenu();

    QSize sizeHint() const;

protected slots:
    void saveImage();
    void toggleAnimation();
    void toggleCursor();
    void changeRate();
    void setRate(int);
    void about();

    void configure();
    void skinConfigChosen(int i);
    void chooseSize(const QSize& sz);

    void setZoom1();
    void setZoom2();
    void setZoom3();
    void setZoom4();
    void setZoomHalf();
    void setZoom075();

    void setZoom();

public slots:
    void setZoom(double);

protected:
    void createMenu(QMenuBar *menu);
    void createMenu(Q3PopupMenu *menu);
    QMenu* createFileMenu();
    QMenu* createViewMenu();
    QMenu* createHelpMenu();

private:
    void findSkins(const QString &currentSkin);
    void init();
    Skin *skin;
    double skinscaleH,skinscaleV;
    QVFbView *view;
    QVFbRateDialog *rateDlg;

    QMenu *viewMenu;
    QAction *cursorAction;
    QActionGroup *zoomActions;

    Config* config;
    QStringList skinnames;
    QStringList skinfiles;
    int currentSkinIndex;
    Zoomer *zoomer;
    QScrollArea* scroller;

    int refreshRate;

private slots:
    void setGamma400(int n);
    void setR400(int n);
    void setG400(int n);
    void setB400(int n);
    void updateGammaLabels();
};

#endif
