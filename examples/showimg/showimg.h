/****************************************************************************
** $Id: //depot/qt/main/examples/showimg/showimg.h#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SHOWIMG_H
#define SHOWIMG_H

#include <qwidget.h>
#include <qimage.h>


class QLabel;
class QMenuBar;
class QPopupMenu;

class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    ImageViewer( QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~ImageViewer();
    bool	loadImage( const char *fileName );
protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	scale();
    int		conversion_flags;
    bool	smooth() const;
    bool	useColorContext() const;
    int		alloc_context;
    bool	convertEvent( QMouseEvent* e, int& x, int& y );
    const char* filename;
    QImage	image;			// the loaded image
    QPixmap	pm;			// the converted pixmap
    QPixmap	pmScaled;		// the scaled pixmap

    QMenuBar   *menubar;
    QPopupMenu  *file;
    QPopupMenu   *saveimage;
    QPopupMenu   *savepixmap;
    QPopupMenu  *edit;
    QPopupMenu  *options;

    QWidget    *helpmsg;
    QLabel     *status;
    int         si, sp, ac, co, mo, fd, bd, // Menu item ids
		td, ta, ba, fa, au, ad, dd,
		ss, cc, t1, t8, t32;
    void	updateStatus();
    void	setMenuItemFlags();
    bool 	reconvertImage();
    int		pickx, picky;
    int		clickx, clicky;
    bool	may_be_other;
    static ImageViewer* other;

private slots:
    void	to1Bit();
    void	to8Bit();
    void	to32Bit();
    void	toBitDepth(int);

    void	copy();
    void	paste();

    void	editText();

    void	newWindow();
    void	openFile();
    void	saveImage(int);
    void	savePixmap(int);
    void	giveHelp();
    void	doOption(int);
    void	copyFrom(ImageViewer*);
};


#endif // SHOWIMG_H
