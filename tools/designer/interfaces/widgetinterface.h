 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef WIDGETINTERFACE_H
#define WIDGETINTERFACE_H

#include <qcom.h>
#include <qiconset.h>

class QWidget;

// {55184143-F18F-42c0-A8EB-71C01516019A}
#ifndef IID_Widget
#define IID_Widget QUuid( 0x55184143, 0xf18f, 0x42c0, 0xa8, 0xeb, 0x71, 0xc0, 0x15, 0x16, 0x1, 0x9a )
#endif

/*! To add custom widgets to the Qt Designer, implement that interface
  in your custom widget plugin.

  You also have to implement the function featureList() (\sa
  QFeatureListInterface) and return there all widgets (names of it)
  which this interface provides.
*/

struct WidgetInterface : public QFeatureListInterface
{
public:

    /*! In the implementation create and return the widget \a widget
      here, use \a parent and \a name when creating the widget */
    virtual QWidget* create( const QString &widget, QWidget* parent = 0, const char* name = 0 ) = 0;

    /*! In the implementation return the name of the group of the
      widget \a widget */
    virtual QString group( const QString &widget ) const = 0;

    /*! In the implementation return the iconset, which should be used
      in the Qt Designer menubar and toolbar to represent the widget
      \a widget */
    virtual QIconSet iconSet( const QString &widget ) const = 0;

    /*! In the implementation return the include file which is needed
      for the widget \a widget in the generated code which uic
      generates. */
    virtual QString includeFile( const QString &widget ) const = 0;

    /*! In the implementation return the text which should be
      displayed as tooltip for the widget \a widget */
    virtual QString toolTip( const QString &widget ) const = 0;

    /*! In the implementation return the text which should be used for
      what's this help for the widget \a widget. */
    virtual QString whatsThis( const QString &widget ) const = 0;

    /*! In the implementation return TRUE here, of the \a widget
      should be able to contain other widget in the Qt Designer, else
      FALSE. */
    virtual bool isContainer( const QString &widget ) const = 0;
};

#endif
