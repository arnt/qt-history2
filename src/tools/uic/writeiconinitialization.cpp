/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "writeiconinitialization.h"
#include "writeicondata.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"

WriteIconInitialization::WriteIconInitialization(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void WriteIconInitialization::accept(DomUI *node)
{
    if (node->elementImages() == 0)
        return;

    QString className = node->elementClass() + option.postfix;

    output << "inline QPixmap " << className << "::icon(" << className << "::IconID id)\n"
           << "{\n";

    WriteIconData(uic).accept(node);

    output << option.indent << "switch (id) {\n";

    TreeWalker::accept(node);

    output << option.indent << option.indent << "default: return QPixmap();\n";

    output << option.indent << "}\n"
           << "}\n\n";
}

void WriteIconInitialization::accept(DomImages *images)
{
    TreeWalker::accept(images);
}

void WriteIconInitialization::accept(DomImage *image)
{
    QString img = image->attributeName() + QLatin1String("_data");
    QString data = image->elementData()->text();
    QString fmt = image->elementData()->attributeFormat();

    QString imageId = image->attributeName() + QLatin1String("_ID");
    QString imageData = image->attributeName() + QLatin1String("_data");
    QString ind = option.indent + option.indent;

    output << ind << "case " << imageId << ": ";

    if (fmt == QLatin1String("XPM.GZ")) {
        output << "return " << "QPixmap((const char**)" << imageData << ");\n";
    } else {
        output << " { QImage img; img.loadFromData(" << imageData << ", sizeof(" << imageData << "), " << fixString(fmt) << "); return img; }\n";
    }
}

