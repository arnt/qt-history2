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

#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcom.h>

class QAction;
class QObject;

// {BB206E09-84E5-4777-9FCE-706BABFAB931}
#ifndef IID_Action
#define IID_Action QUuid( 0xbb206e09, 0x84e5, 0x4777, 0x9f, 0xce, 0x70, 0x6b, 0xab, 0xfa, 0xb9, 0x31 )
#endif

/*! To add actions to the Qt Designer menubars and toolbars, implement
  this interface. You habe to implement the create(), group() and
  connectTo() functions.

  You also have to implement the function featureList() (\sa
  QFeatureListInterface) and return there all actions (names of it)
  which this interface provides.
*/

class ActionInterface : public QFeatureListInterface
{
public:
    /*! This functions is called to create the action with the name \a
      name. \a parent should be used as parent of the action.

      In the implementation return the QAction object for the action
      \a name.
    */
    virtual QAction* create( const QString &name, QObject* parent = 0 ) = 0;

    /*! In the implementation of the interface return the name of the
      group of the action \a name.
    */
    virtual QString group( const QString &name ) const = 0;

    /*! \internal */
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
};

#endif
