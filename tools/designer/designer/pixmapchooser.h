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

#ifndef PIXMAPCHOOSER_H
#define PIXMAPCHOOSER_H

#include <qfiledialog.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qurl.h>

class FormWindow;

class PixmapView : public QScrollView,
		   public QFilePreview
{
    Q_OBJECT

public:
    PixmapView( QWidget *parent );
    void setPixmap( const QPixmap &pix );
    void drawContents( QPainter *p, int, int, int, int );
    void previewUrl( const QUrl &u );

private:
    QPixmap pixmap;

};

class ImageIconProvider : public QFileIconProvider
{
    Q_OBJECT

public:
    ImageIconProvider( QWidget *parent = 0, const char *name = 0 );
    ~ImageIconProvider();

    const QPixmap *pixmap( const QFileInfo &fi );

private:
    QList<QByteArray> fmts;
    QPixmap imagepm;

};

QPixmap qChoosePixmap( QWidget *parent, FormWindow *fw = 0, const QPixmap &old = QPixmap(),  QString *fn = 0 );
QStringList qChoosePixmaps( QWidget *parent );

#endif
