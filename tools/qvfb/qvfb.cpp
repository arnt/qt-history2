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

#include "qvfb.h"
#include "qvfbview.h"
#include "qvfbratedlg.h"
#include "config.h"
#include "skin.h"

#include <qmenu.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qcheckbox.h>
#include <qscrollarea.h>

QVFb::QVFb( int display_id, int w, int h, int d, const QString &skinName, QWidget *parent,
            const char *name, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    setObjectName(name);
    QPixmap pix(":/res/images/logo.png");
    setWindowIcon( pix );

    rateDlg = 0;
    view = 0;
    scroller = 0;
    skin = 0;
    init( display_id, w, h, d, skinName );
    createActions();
    createMenuBar();
}

QVFb::~QVFb()
{
}

void QVFb::init( int display_id, int w, int h, int d, const QString &skin_name )
{
    setWindowTitle( QString("Virtual framebuffer %1x%2 %3bpp Display :%4")
                    .arg(w).arg(h).arg(d).arg(display_id) );
    delete view;
    delete scroller;
    delete skin;

    if ( !skin_name.isEmpty() && QFile::exists(skin_name) ) {
        bool vis = isVisible();
        if ( vis ) hide();
        menuBar()->hide();
        scroller = 0;
        skin = new Skin( this, skin_name, w, h );
        view = new QVFbView( display_id, w, h, d, skin );
        skin->setView( view );
        view->setFixedSize( w, h );
        setCentralWidget( skin );
        view->show();
        if ( vis ) show();
    } else {
        if ( !currentSkin.isEmpty() ) {
            clearMask();
            setParent(0, 0);
            show();
        }
        menuBar()->show();
        skin = 0;
        scroller = new QScrollArea(this);
        view = new QVFbView( display_id, w, h, d, scroller );
        scroller->setWidget(view);
        setCentralWidget(scroller);
        scroller->show();
    }
    // Resize QVFb to the new size
    QSize newSize = view->sizeHint() + (frameSize() - size());
    if (frameSize() == size()) // First time (no window border yet)
        newSize += QSize(10, 30);
    resize(newSize);

    currentSkin = skin_name;
}

QAction *QVFb::newAction(const char *menuName, const char *shortkey, const char *slot)
{
    QAction *result = new QAction(menuName, this);
    result->setShortcut(QKeySequence(shortkey));
    connect(result, SIGNAL(triggered()), slot);
    return result;
}

void QVFb::createActions()
{
    actions[ConfigAct] = newAction("&Configure", "Ctrl+Alt+C", SLOT(configure()));
    actions[QuitAct] = new QAction("&Quit", this);
    connect(actions[QuitAct], SIGNAL(triggered()), qApp, SLOT(quit()));
    actions[AboutAct] = newAction("&About", "", SLOT(about()));
    actions[AboutQtAct] = newAction("About &Qt", "", SLOT(aboutQt()));

    actions[SaveAct] = newAction("&Save image...", "Ctrl+Alt+S", SLOT(saveImage()));

    actions[AnimationAct] = newAction("&Animation...", "Ctrl+Alt+A", SLOT(toggleAnimation()));
    actions[CursorAct] = newAction("Show &Cursor", "Ctrl+Alt+U", SLOT(toggleCursor()));
    actions[CursorAct]->setCheckable(true);
    enableCursor(true);

    actions[RefreshAct] = newAction("&Refresh rate...", "Ctrl+Alt+R", SLOT(changeRate()));
    actions[Zoom05Act] = newAction("Zoom scale &0.5", "Ctrl+Alt+0", SLOT(setZoomHalf()));
    actions[Zoom1Act] = newAction("Zoom scale &1", "Ctrl+Alt+1", SLOT(setZoom1()));
    actions[Zoom2Act] = newAction("Zoom scale &2", "Ctrl+Alt+2", SLOT(setZoom2()));
    actions[Zoom3Act] = newAction("Zoom scale &3", "Ctrl+Alt+3", SLOT(setZoom3()));
    actions[Zoom4Act] = newAction("Zoom scale &4", "Ctrl+Alt+4", SLOT(setZoom4()));
}

QMenu *QVFb::createPopupMenu()
{
    QMenu *file = new QMenu("&File");
    file->addAction(actions[ConfigAct]);
    file->addSeparator();
    file->addAction(actions[SaveAct]);
    file->addAction(actions[AnimationAct]);
    file->addSeparator();
    file->addAction(actions[QuitAct]);

    QMenu *view = new QMenu("&View");
    view->addAction(actions[CursorAct]);
    view->addAction(actions[RefreshAct]);
    view->addSeparator();
    view->addAction(actions[Zoom1Act]);
    view->addAction(actions[Zoom2Act]);
    view->addAction(actions[Zoom3Act]);
    view->addAction(actions[Zoom4Act]);
    view->addAction(actions[Zoom05Act]);

    QMenu *help = new QMenu("Help");
    help->addAction(actions[AboutAct]);
    help->addAction(actions[AboutQtAct]);

    QMenu *menu = new QMenu(this);
    menu->addAction(file->menuAction());
    menu->addAction(view->menuAction());
    menu->addSeparator();
    menu->addAction(help->menuAction());
    return menu;
}

void QVFb::createMenuBar()
{
    QMenu *file = new QMenu("&File");
    file->addAction(actions[ConfigAct]);
    file->addSeparator();
    file->addAction(actions[SaveAct]);
    file->addAction(actions[AnimationAct]);
    file->addSeparator();
    file->addAction(actions[QuitAct]);

    QMenu *view = new QMenu("&View");
    view->addAction(actions[CursorAct]);
    view->addAction(actions[RefreshAct]);
    view->addSeparator();
    view->addAction(actions[Zoom1Act]);
    view->addAction(actions[Zoom2Act]);
    view->addAction(actions[Zoom3Act]);
    view->addAction(actions[Zoom4Act]);
    view->addAction(actions[Zoom05Act]);

    QMenu *help = new QMenu("&Help");
    help->addAction(actions[AboutAct]);
    help->addAction(actions[AboutQtAct]);

    QMenuBar *menu = menuBar();
    menu->addAction(file->menuAction());
    menu->addAction(view->menuAction());
    menu->addSeparator();
    menu->addAction(help->menuAction());
}

void QVFb::enableCursor( bool e )
{
    view->setCursor( e ? Qt::ArrowCursor : Qt::BlankCursor );
    actions[CursorAct]->setChecked(e);
}

void QVFb::setZoom(double z)
{
    view->setZoom(z);
}

void QVFb::setZoomHalf()
{
    setZoom(0.5);
}

void QVFb::setZoom1()
{
    setZoom(1);
}

void QVFb::setZoom2()
{
    setZoom(2);
}

void QVFb::setZoom3()
{
    setZoom(3);
}

void QVFb::setZoom4()
{
    setZoom(4);
}

void QVFb::saveImage()
{
    QImage img = view->image();
    QString filename = QFileDialog::getSaveFileName(this, "Save image", "snapshot.png", "Portable Network Graphics (*.png)");
    if (!filename.isEmpty())
        img.save(filename,"PNG");
}

void QVFb::toggleAnimation()
{
    if ( view->animating() ) {
        view->stopAnimation();
    } else {
#if 0
        QString filename = QFileDialog::getSaveFileName(this, "Save animation", "animation.mng", "*.mng");
        if ( !filename ) {
            view->stopAnimation();
        } else {
            view->startAnimation(filename);
        }
#endif
    }
}

void QVFb::toggleCursor()
{
    enableCursor(actions[CursorAct]->isChecked());
}

void QVFb::changeRate()
{
    if ( !rateDlg ) {
        rateDlg = new QVFbRateDialog( view->rate(), this );
        connect( rateDlg, SIGNAL(updateRate(int)), view, SLOT(setRate(int)) );
    }

    rateDlg->show();
}

void QVFb::about()
{
#if defined(Q_WS_MAC)
    QString platform("Mac OS X");
    QString qt("Mac");
#elif defined( Q_WS_WIN )
    QString platform("Windows");
    QString qt("Windows");
#else
    QString platform("X11");
    QString qt("X11");
#endif
    QMessageBox::about(this, "About QVFB",
        "<p><b><font size=+2>Qt/Embedded Virtual " + platform + " Framebuffer</font></b></p>"
        "<p></p>"
        "<p>Version 1.0</p>"
        "<p>Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.</p>"
        "<p></p>"
        "<p>This program is licensed to you under the terms of the GNU General "
        "Public License Version 2 as published by the Free Software Foundation. This "
        "gives you legal permission to copy, distribute and/or modify this software "
        "under certain conditions. For details, see the file 'LICENSE.GPL' that came with "
        "this software distribution. If you did not get the file, send email to "
        "info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY "
        "OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
        "FOR A PARTICULAR PURPOSE.</p>"
    );
}

void QVFb::aboutQt()
{
    QMessageBox::aboutQt( this, tr("QVFB") );
}

void QVFb::configure()
{
    config = new Config(this, Qt::WShowModal);

    int w = view->displayWidth();
    int h = view->displayHeight();
    QString skin;
    config->size_width->setValue(w);
    config->size_height->setValue(h);
    config->size_custom->setChecked(true); // unless changed by settings below
    config->size_240_320->setChecked(w==240&&h==320);
    config->size_320_240->setChecked(w==320&&h==240);
    config->size_640_480->setChecked(w==640&&h==480);
    config->skin->setEditable(true);
    if (!currentSkin.isNull()) {
        config->size_skin->setChecked(true);
        config->skin->setEditText(currentSkin);
    }
    config->touchScreen->setChecked(view->touchScreenEmulation());
    config->depth_1->setChecked(view->displayDepth()==1);
    config->depth_4gray->setChecked(view->displayDepth()==4);
    config->depth_8->setChecked(view->displayDepth()==8);
    config->depth_12->setChecked(view->displayDepth()==12);
    config->depth_16->setChecked(view->displayDepth()==16);
    config->depth_24->setChecked(view->displayDepth()==24);
    config->depth_32->setChecked(view->displayDepth()==32);
    if ( view->gammaRed() == view->gammaGreen() && view->gammaGreen() == view->gammaBlue() ) {
        config->gammaslider->setValue(int(view->gammaRed()*400));
        config->rslider->setValue(100);
        config->gslider->setValue(100);
        config->bslider->setValue(100);
    } else {
        config->gammaslider->setValue(100);
        config->rslider->setValue(int(view->gammaRed()*400));
        config->gslider->setValue(int(view->gammaGreen()*400));
        config->bslider->setValue(int(view->gammaBlue()*400));
    }
    connect(config->gammaslider, SIGNAL(valueChanged(int)), this, SLOT(setGamma400(int)));
    connect(config->rslider, SIGNAL(valueChanged(int)), this, SLOT(setR400(int)));
    connect(config->gslider, SIGNAL(valueChanged(int)), this, SLOT(setG400(int)));
    connect(config->bslider, SIGNAL(valueChanged(int)), this, SLOT(setB400(int)));
    connect(config->resetgamma, SIGNAL(clicked()), this, SLOT(resetGamma()));
    updateGammaLabels();

    connect(config->buttonOk, SIGNAL(clicked()), config, SLOT(accept()));
    connect(config->buttonCancel, SIGNAL(clicked()), config, SLOT(reject()));

    double ogr=view->gammaRed(), ogg=view->gammaGreen(), ogb=view->gammaBlue();


    if ( config->exec() ) {
        int id = view->displayId(); // not settable yet
        if ( config->size_240_320->isChecked() ) {
            w=240; h=320;
        } else if ( config->size_320_240->isChecked() ) {
            w=320; h=240;
        } else if ( config->size_640_480->isChecked() ) {
            w=640; h=480;
        } else if ( config->size_skin->isChecked() ) {
            skin = config->skin->currentText();
        } else {
            w=config->size_width->value();
            h=config->size_height->value();
        }
        int d;
        if ( config->depth_1->isChecked() )
            d=1;
        else if ( config->depth_4gray->isChecked() )
            d=4;
        else if ( config->depth_8->isChecked() )
            d=8;
        else if ( config->depth_12->isChecked() )
            d=12;
        else if ( config->depth_16->isChecked() )
            d=16;
        else if ( config->depth_24->isChecked() )
            d=24;
        else
            d=32;

        if ( w != view->displayWidth() || h != view->displayHeight()
                || d != view->displayDepth() || skin != currentSkin )
            init( id, w, h, d, skin );
        view->setTouchscreenEmulation( config->touchScreen->isChecked() );
    } else {
        view->setGamma(ogr, ogg, ogb);
    }

    delete config;
    config=0;
}

void QVFb::resetGamma()
{
    config->gammaslider->setValue(100);
    config->rslider->setValue(100);
    config->gslider->setValue(100);
    config->bslider->setValue(100);
    updateGammaLabels();
    view->setGamma(1.0, 1.0, 1.0);
}
void QVFb::setGamma400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   config->gslider->value()/100.0*g,
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::setR400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   view->gammaGreen(),
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setG400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   config->gslider->value()/100.0*g,
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setB400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   view->gammaGreen(),
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::updateGammaLabels()
{
    config->rlabel->setText(QString::number(view->gammaRed(),'g',2));
    config->glabel->setText(QString::number(view->gammaGreen(),'g',2));
    config->blabel->setText(QString::number(view->gammaBlue(),'g',2));
}
