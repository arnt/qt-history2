/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Patternist_ExternalEvironment_H
#define Patternist_ExternalEvironment_H

#include <QString>
#include <QUrl>

#include "Primitives.h"
#include "QName.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Contains static values and functions related to retrieving
     * information about the implementation.
     *
     * This is notably used for supporting XSL-T's <tt>fn:system-property()</tt> function.
     *
     * The values are stored in native formats, and their string representation,
     * as defined in the specification, can be accessed via retrieveProperty().
     *
     * @see <a href="http://www.w3.org/TR/xslt20/#system-property">XSL Transformations
     * (XSLT) Version 2.0, 16.6.5 system-property</a>
     * @author Frans Englich <fenglich@trolltech.com>
      */
    class ExternalEnvironment
    {
    public:
        /**
         * The supported XSL-T version.
         *
         * This value is currently not 2.0 even though Patternist is intended to
         * be an XSL-T 2.0 processor. This is because Patternist is not comformant.
         *
         * @see <a href="http://www.w3.org/TR/xslt20/#system-property">The Note paragraph
         * at the very end of XSL Transformations (XSLT) Version 2.0,
         * 16.6.5 system-property</a>
         */
        static const xsDecimal XSLVersion;

        static const QString Vendor;
        static const QUrl VendorURL;
        static const QString ProductName;
        static const QString ProductVersion;
        static const bool IsSchemaAware;
        static const bool SupportsSerialization;
        static const bool SupportsBackwardsCompatibility;

        /**
         * Returns a string representation for @p property as defined
         * for the system properties in "XSL Transformations (XSLT)
         * Version 2.0, 16.6.5 system-property". Hence, this function
         * handles only the properties specified in the XSL namespace, and returns
         * an empty string if an unrecognized property is asked for.
         */
        static QString retrieveProperty(const QName name);

    private:
        static inline QString toString(const bool);
        /**
         * This class is not supposed to be instantiated. No implementation
         * is provided for this constructor.
         */
        ExternalEnvironment();
        Q_DISABLE_COPY(ExternalEnvironment)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
