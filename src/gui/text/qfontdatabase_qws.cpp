/****************************************************************************
**
** Implementation of platform specific QFontDatabase.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfontmanager_qws.h"
#include "qdir.h"
#include "qgfx_qws.h" //for qt_screen so we can check for rotation

extern QString qws_topdir();

/*!
    \internal
*/
void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf )
{
    if ( !db )
	return;

    QString familyname = qdf->name;
    QString foundryname = "truetype";
    if ( qdf->factory->name() == "BDF" )
	foundryname = "bdf";
    int weight = qdf->weight;
    bool italic = qdf->italic;
    QtFontStyle::Key styleKey;
    styleKey.italic = italic;
    styleKey.oblique = false;
    styleKey.weight = weight;

    QtFontFamily *f = db->family( familyname, true );
    QtFontFoundry *foundry = f->foundry( foundryname, true );
    QtFontStyle *style = foundry->style( styleKey,  TRUE );
    style->smoothScalable = (qdf->factory->name() == "FT" || qdf->size == 0);
    if ( !style->smoothScalable )
	style->pixelSize( qdf->size/10, TRUE );
}

static void initializeDb()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;
    qfontdatabase_cleanup.set(&db);

    if(!qt_fontmanager)
        qt_fontmanager=new QFontManager();

    QDiskFont *qdf;

    for (int i = 0; i < qt_fontmanager->diskfonts.size(); ++i) {
        qdf = qt_fontmanager->diskfonts.at(i);
	QFontDatabase::qwsAddDiskFont( qdf );
    }

#ifndef QT_NO_DIR

    QDir dir(qws_topdir()+"/lib/fonts/","*.qpf");
    for (int i=0; i<(int)dir.count(); i++) {
	int u0 = dir[i].find('_');
	int u1 = dir[i].find('_',u0+1);
	int u2 = dir[i].find('_',u1+1);
	int u3 = dir[i].find('.',u1+1);
	if ( u2 < 0 ) u2 = u3;

#if 1
	/*
	  Skip fonts for other screen orientations. Such fonts may be
	  installed even on a production device. Different orientations
	  could have different fonts.
	*/
	//### This code could be prettier
	QString rotation = u2 == u3 ? QString::null : dir[i].mid(u2+1,u3-u2-1);

	QString screenr;
	if ( qt_screen->isTransformed() ) {
	    screenr = "t";
	    QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
	    QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
	    screenr += QString::number( a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()) );
	}

	if ( rotation != screenr )
	    continue;
#endif

	QString familyname = dir[i].left(u0);
	int pointSize = dir[i].mid(u0+1,u1-u0-1).toInt()/10;
	bool italic = dir[i].mid(u2-1,1) == "i";
	int weight = dir[i].mid(u1+1,u2-u1-1-(italic?1:0)).toInt();
	QtFontFamily *f = db->family( familyname, true );
	f->scripts[QFont::Unicode] = QtFontFamily::Supported;
	QtFontFoundry *foundry = f->foundry( "qt", true );
	QtFontStyle::Key styleKey;
	styleKey.italic = italic;
	styleKey.oblique = false;
	styleKey.weight = weight;
	QtFontStyle *style = foundry->style( styleKey,  TRUE );
	style->smoothScalable = FALSE;
	style->pixelSize( pointSize, TRUE );
    }
#endif
}

static inline void load(const QString & = QString::null, int = -1 )
{
}
