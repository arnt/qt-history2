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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <qstring.h>
#include <qtextstream.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qimage.h>

#include "metadatabase.h"

class QWidget;
class QObject;
class QLayout;
class QStyle;
class QPalette;
class FormWindow;
class MainWindow;
class QDomElement;
class QDesignerGridLayout;
class QListViewItem;

class Resource
{
public:
    Resource( QStyle* s = 0, QPalette* pal = 0 );
    Resource( MainWindow* mw, QStyle* s = 0, QPalette* pal = 0 );

    void setWidget( FormWindow *w );
    QWidget *widget() const;

    bool load( const QString& filename);
    bool load( QIODevice* );
    QString copy();

    bool save( const QString& filename);
    bool save( QIODevice* );
    void paste( const QString &cb, QWidget *parent );

    static void saveImageData( const QImage &img, QTextStream &ts, int indent );
    static void loadCustomWidgets( const QDomElement &e, Resource *r );

private:
    void saveObject( QObject *obj, QDesignerGridLayout* grid, QTextStream &ts, int indent );
    void saveChildrenOf( QObject* obj, QTextStream &ts, int indent );
    void saveObjectProperties( QObject *w, QTextStream &ts, int indent );
    void saveSetProperty( QObject *w, const QString &name, QVariant::Type t, QTextStream &ts, int indent );
    void saveEnumProperty( QObject *w, const QString &name, QVariant::Type t, QTextStream &ts, int indent );
    void saveProperty( QObject *w, const QString &name, const QVariant &value, QVariant::Type t, QTextStream &ts, int indent );
    void saveProperty( const QVariant &value, QTextStream &ts, int indent );
    void saveItems( QObject *obj, QTextStream &ts, int indent );
    void saveItem( const QStringList &text, const QList<QPixmap> &pixmaps, QTextStream &ts, int indent );
    void saveItem( QListViewItem *i, QTextStream &ts, int indent );
    void saveConnections( QTextStream &ts, int indent );
    void saveCustomWidgets( QTextStream &ts, int indent );
    void saveTabOrder( QTextStream &ts, int indent );
    void saveColorGroup( QTextStream &ts, int indent, const QColorGroup &cg );
    void saveColor( QTextStream &ts, int indent, const QColor &c );
    void saveMetaInfo( QTextStream &ts, int indent );
    void savePixmap( const QPixmap &p, QTextStream &ts, int indent );

    QObject *createObject( const QDomElement &e, QWidget *parent, QLayout* layout = 0 );
    QWidget *createSpacer( const QDomElement &e, QWidget *parent, QLayout *layout, Qt::Orientation o );
    void createItem( const QDomElement &e, QWidget *widget, QListViewItem *i = 0 );
    void createColumn( const QDomElement &e, QWidget *widget );
    void setObjectProperty( QObject* widget, const QString &prop, const QDomElement &e);
    QString saveInCollection( const QImage &img );
    QString saveInCollection( const QPixmap &pix ) { return saveInCollection( pix.convertToImage() ); }
    QImage loadFromCollection( const QString &name );
    void saveImageCollection( QTextStream &ts, int indent );
    void loadImageCollection( const QDomElement &e );
    void loadConnections( const QDomElement &e );
    void loadTabOrder( const QDomElement &e );
    void loadItem( const QDomElement &n, QPixmap &pix, QString &txt, bool &hasPixmap );
    QColorGroup loadColorGroup( const QDomElement &e );
    QPixmap loadPixmap( const QDomElement &e );

private:
    struct Image {
	QImage img;
	QString name;
	bool operator==(  const Image &i ) const {
	    return ( i.name == name &&
		     i.img == img );
	}
    };

private:
    MainWindow *mainwindow;
    FormWindow *formwindow;
    QWidget* toplevel;
    QValueList<Image> images;
    bool previewMode;
    bool copying, pasting;
    bool mainContainerSet;
    QStyle* style;
    QPalette* pal;
    QStringList knownNames;
    QStringList usedCustomWidgets;
    QListViewItem *lastItem;

    QValueList<MetaDataBase::Include> metaIncludes;
    MetaDataBase::MetaInfo metaInfo;

};

#endif
