/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "blockingprocess.h"
#include "driver.h"
#include "option.h"
#include "ui4.h"
#include "uic.h"

// operations
#include "treewalker.h"
#include "validator.h"
#include "writedeclaration.h"
#include "writeincludes.h"
#include "writeinitialization.h"

#include <qtextstream.h>
#include <qfileinfo.h>
#include <qprocess.h>
#include <qregexp.h>
#include <qcoreapplication.h>
#include <qdebug.h>

#if defined Q_WS_WIN
#include <qt_windows.h>
#endif

Uic::Uic(Driver *drv)
     : driver(drv),
       output(driver->output()),
       option(driver->option())
{
}

Uic::~Uic()
{
}

void Uic::writeCopyrightHeader(DomUI *ui)
{
    QString comment = ui->elementComment();
    if (comment.size())
        output << "/*\n" << comment << "\n*/\n";
}

bool Uic::write(QIODevice *in)
{
    QDomDocument doc;
    if (!doc.setContent(in))
        return false;

    QDomElement root = doc.firstChild().toElement();
    DomUI *ui = new DomUI();
    ui->read(root);

    double version = ui->attributeVersion().toDouble();
    if (version < 4.0) {
        delete ui;

        if (option.inputFile.isEmpty()) {
            fprintf(stderr, "impossible to convert a file from the stdin\n");
            return false;
        }
        //qWarning("converting file '%s'", option.inputFile.latin1());
        BlockingProcess uic3;
        uic3.setArguments(QStringList() << "uic3" << "-convert" << option.inputFile);
        if (!uic3.start()) {
            fprintf(stderr, "Couldn't start uic3\n");
            return false;
        }

        QString contents = uic3.out;
        if (!doc.setContent(contents))
            return false;

        ui = new DomUI();
        QDomElement root = doc.firstChild().toElement();
        ui->read(root);
    }

    if (option.copyrightHeader)
        writeCopyrightHeader(ui);

    if (option.headerProtection)
        writeHeaderProtectionStart();

    bool rtn = write(ui);

    if (option.headerProtection)
        writeHeaderProtectionEnd();

    delete ui;

    return rtn;
}

bool Uic::write(DomUI *ui)
{
    if (!ui || !ui->elementWidget())
        return false;

    WriteIncludes(driver).accept(ui);

    if (option.generateNamespace)
        output << "namespace Ui {\n\n";

    Validator(driver).accept(ui);
    WriteDeclaration(driver).accept(ui);
    WriteInitialization(driver).accept(ui);

    if (option.generateNamespace)
        output << "}\n\n";

    return true;
}

void Uic::writeHeaderProtectionStart()
{
    QString h = driver->headerFileName();
    output << "#ifndef " << h << "\n"
           << "#define " << h << "\n\n";
}

void Uic::writeHeaderProtectionEnd()
{
    QString h = driver->headerFileName();
    output << "#endif // " << h << "\n\n";
}
