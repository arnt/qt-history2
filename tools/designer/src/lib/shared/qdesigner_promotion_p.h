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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNERPROMOTION_H
#define QDESIGNERPROMOTION_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerPromotionInterface>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

    class  QDESIGNER_SHARED_EXPORT  QDesignerPromotion : public QDesignerPromotionInterface
    {
    public:
        QDesignerPromotion(QDesignerFormEditorInterface *core);

        virtual PromotedClasses promotedClasses() const;

        virtual QSet<QString> referencedPromotedClassNames() const;

        virtual bool addPromotedClass(const QString &baseClass,
                                      const QString &className,
                                      const QString &includeFile,
                                      QString *errorMessage);

        virtual bool removePromotedClass(const QString &className, QString *errorMessage);

        virtual bool changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage);

        virtual bool setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage);

        virtual QList<QDesignerWidgetDataBaseItemInterface *> promotionBaseClasses() const;

    private:
        bool canBePromoted(const QDesignerWidgetDataBaseItemInterface *) const;
        void refreshObjectInspector();

        QDesignerFormEditorInterface *m_core;
    };
}

#endif // QDESIGNERPROMOTION_H
