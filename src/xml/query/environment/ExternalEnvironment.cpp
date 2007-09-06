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
***************************************************************************
*/

#include "CommonNamespaces.h"
#include "Debug.h"
#include "NamePool.h"

#include "ExternalEnvironment.h"

using namespace Patternist;

/* The goal is to support serialization, backwards compatibility, import feature
 * and full axis featur, but not schema awareness. */

const xsDecimal ExternalEnvironment::XSLVersion                        (1.20);
const QString   ExternalEnvironment::Vendor                            (QLatin1String("The Patternist Team"));
const QUrl      ExternalEnvironment::VendorURL                         (QLatin1String("http://svg.kde.org/"));
const QString   ExternalEnvironment::ProductName                       (QLatin1String("Patternist"));
const QString   ExternalEnvironment::ProductVersion                    (QLatin1String("0.1"));
const bool      ExternalEnvironment::IsSchemaAware                     (false);
const bool      ExternalEnvironment::SupportsSerialization             (false);
const bool      ExternalEnvironment::SupportsBackwardsCompatibility    (false);

QString ExternalEnvironment::toString(const bool value)
{
    return value ? QLatin1String("yes") : QLatin1String("no");
}

QString ExternalEnvironment::retrieveProperty(const QName name)
{
    if(name.namespaceURI() != StandardNamespaces::xslt)
        return QString();

    switch(name.localName())
    {
        case StandardLocalNames::version:
            return QString::number(ExternalEnvironment::XSLVersion);
        case StandardLocalNames::vendor:
            return ExternalEnvironment::Vendor;
        case StandardLocalNames::vendor_url:
            return QString(ExternalEnvironment::VendorURL.toString());
        case StandardLocalNames::product_name:
            return ExternalEnvironment::ProductName;
        case StandardLocalNames::product_version:
            return ExternalEnvironment::ProductVersion;
        case StandardLocalNames::is_schema_aware:
            return toString(ExternalEnvironment::IsSchemaAware);
        case StandardLocalNames::supports_serialization:
            return toString(ExternalEnvironment::SupportsSerialization);
        case StandardLocalNames::supports_backwards_compatibility:
            return toString(ExternalEnvironment::SupportsBackwardsCompatibility);
        default:
            return QString();
    }
}

// vim: et:ts=4:sw=4:sts=4
