/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef PIXMAPCHOOSER_H
#define PIXMAPCHOOSER_H

#include <qfiledialog.h>
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
    QStrList fmts;
    QPixmap imagepm;

};

QPixmap qChoosePixmap( QWidget *parent, FormWindow *fw = 0, const QPixmap &old = QPixmap() );

class PixmapChooser
{
public:
    enum Size { Mini, Small, Large, NoSize, Disabled };

    PixmapChooser();
    QString pixmapPath( Size size ) const;

    static QPixmap loadPixmap( const QString &name, Size size = Small );

private:
    QString smallPixDir;
    QString largePixDir;
    QString miniPixDir;
    QString noSizePixDir;

};


#endif
