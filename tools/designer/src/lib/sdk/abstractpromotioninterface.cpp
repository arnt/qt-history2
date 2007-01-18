/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "abstractpromotioninterface.h"

QDesignerPromotionInterface::~QDesignerPromotionInterface()
{
}

/*!
    \class QDesignerPromotionInterface

    \brief The QDesignerPromotionInterface provides functions for modifying
           the promoted classes in Designer.
    \inmodule QtDesigner
    \internal
*/

/*!
    \class QDesignerPromotionInterface::PromotedClass
    A pair of database items containing the base class and the promoted class.

    \typedef QDesignerPromotionInterface::PromotedClasses
    A list of PromotedClass items.

    virtual QDesignerPromotionInterface::PromotedClasses promotedClasses()  const = 0;

    Returns a list of promoted classes along with their base classes in alphabetical order.
    It can be used to populate tree models for editing promoted widgets.

*/

/*!  \fn virtual QSet<QString> QDesignerPromotionInterface::referencedPromotedClassNames()  const = 0;
     Returns a set of promoted classed that are referenced by the currently opened forms.
*/

/*! \fn virtual bool QDesignerPromotionInterface::addPromotedClass(const QString &baseClass, const QString &className, const QString &includeFile, QString *errorMessage)= 0;

    Add a promoted class.
*/

/*! \fn  virtual bool QDesignerPromotionInterface::removePromotedClass(const QString &className, QString *errorMessage)= 0;

    Remove a promoted class unless it is referenced by a form.
*/

/*! \fn  virtual bool QDesignerPromotionInterface::changePromotedClassName(const QString &oldClassName, const QString &newClassName,  QString *errorMessage)= 0;

    Change the class name of a promoted class.
*/

/*! \fn  virtual bool QDesignerPromotionInterface::setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage)= 0
   Change the include file of a promoted class.
*/

/*! \fn virtual QList<QDesignerWidgetDataBaseItemInterface *> QDesignerPromotionInterface::promotionBaseClasses() const = 0;

     Return a list of base classes that are suitable for promotion.
*/
