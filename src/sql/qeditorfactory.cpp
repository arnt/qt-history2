/****************************************************************************
**
** Implementation of QEditorFactory class
**
** Created : 2000-11-17
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcleanuphandler.h"
#include "qwidget.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qcombobox.h"
#include "qlayout.h"

#include "qeditorfactory.h"
#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

/*!
    \class QEditorFactory qeditorfactory.h
    \brief This class is used to create editor factories for QVariants.

    Each editor factory provides the createEditor() function which given
    a QVariant will create and return a QWidget that can edit that
    QVariant. For example if you have a a QVariant::String a QLineEdit
    would be the default editor, whereas a QVariant::Int's default
    editor would be a QSpinBox.

  If you want to create different editors for fields with the same
  data type, subclass QEditorFactory and reimplement the createEditor()
  function.

*/

/*!
  Constructs an editor factory with parent \a parent and name \a name.
*/

QEditorFactory::QEditorFactory ( QObject * parent, const char * name )
    : QObject( parent, name )
{

}

/*!
  Destroys the object and frees any allocated resources.
*/

QEditorFactory::~QEditorFactory()
{

}

static QEditorFactory * defaultfactory = 0;
static QCleanupHandler< QEditorFactory > q_cleanup_editor_factory;

/*!
  Returns an instance of a default editor factory.
*/

QEditorFactory * QEditorFactory::defaultFactory()
{
    if( defaultfactory == 0 ){
	defaultfactory = new QEditorFactory();
	q_cleanup_editor_factory.add( defaultfactory );
    }

    return defaultfactory;
}

/*!

  Replaces the default editor factory with \a factory.
  <em>QEditorFactory takes ownership of factory, and destroys it when
  it is no longer needed. </em>
*/

void QEditorFactory::installDefaultFactory( QEditorFactory * factory )
{
    if( factory == 0 ) return;

    if( defaultfactory != 0 ){
	q_cleanup_editor_factory.remove( defaultfactory );
	delete defaultfactory;
    }
    defaultfactory = factory;
    q_cleanup_editor_factory.add( defaultfactory );
}

/*!
  Creates and returns the appropriate editor for the QVariant \a v.
  If the QVariant is invalid, 0 is returned.
*/

QWidget * QEditorFactory::createEditor( QWidget * parent, const QVariant & v )
{
    QWidget * w = 0;
    switch( v.type() ){
	case QVariant::Invalid:
	    w = 0;
	    break;
	case QVariant::Bool:
	    w = new QComboBox( parent );
	    ((QComboBox *) w)->insertItem( "False" );
	    ((QComboBox *) w)->insertItem( "True" );
	    break;
	case QVariant::UInt:
	case QVariant::Int:
	    w = new QSpinBox( -999999, 999999, 1, parent );
	    break;
	case QVariant::String:
	case QVariant::CString:
	case QVariant::Double:
	    w = new QLineEdit( parent );
	    ((QLineEdit*)w)->setFrame( FALSE );
	    break;
	case QVariant::Date:
	    w = new QDateEdit( parent );
	    ((QDateEdit*)w)->setFrame( FALSE );
	    break;
	case QVariant::Time:
	    w = new QTimeEdit( parent );
	    ((QTimeEdit*)w)->setFrame( FALSE );
	    break;
	case QVariant::DateTime:
	    w = new QDateTimeEdit( parent );
	    ((QDateTimeEdit*)w)->setFrameShape( QFrame::NoFrame );
	    break;
	case QVariant::Pixmap:
	    w = new QLabel( parent );
	    break;
	case QVariant::Palette:
	case QVariant::ColorGroup:
	case QVariant::Color:
	case QVariant::Font:
	case QVariant::Brush:
	case QVariant::Bitmap:
	case QVariant::Cursor:
	case QVariant::Map:
	case QVariant::StringList:
	case QVariant::Rect:
	case QVariant::Size:
	case QVariant::IconSet:
	case QVariant::Point:
	case QVariant::PointArray:
	case QVariant::Region:
	case QVariant::SizePolicy:
	case QVariant::ByteArray:
	default:
	    w = new QWidget( parent );
	    break;
    }
    return w;
}
#endif // QT_NO_SQL
