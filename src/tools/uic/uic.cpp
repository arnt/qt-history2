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
#include "writeiconinitialization.h"

#include <qtextstream.h>
#include <qfileinfo.h>
#include <qprocess.h>
#include <qregexp.h>
#include <qcoreapplication.h>
#include <qdebug.h>

#if defined Q_WS_WIN
#include <qt_windows.h>
#endif

Uic::Uic(Driver *d)
     : drv(d),
       out(d->output()),
       opt(d->option()),
       info(d),
       cWidgetsInfo(d),
       externalPix(true)
{
}

Uic::~Uic()
{
}

void Uic::writeCopyrightHeader(DomUI *ui)
{
    QString comment = ui->elementComment();
    if (comment.size())
        out << "/*\n" << comment << "\n*/\n";
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

        if (opt.inputFile.isEmpty()) {
            fprintf(stderr, "impossible to convert a file from the stdin\n");
            return false;
        }
        //qWarning("converting file '%s'", opt.inputFile.latin1());
        BlockingProcess uic3;
        QString exe = option().uic3;
        uic3.setArguments(QStringList() << exe << QLatin1String("-convert") << opt.inputFile);
        if (!uic3.start()) {
            fprintf(stderr, "Couldn't start uic3\n");
            return false;
        }

        QString contents = QString::fromAscii(uic3.out);
        if (!doc.setContent(contents))
            return false;

        ui = new DomUI();
        QDomElement root = doc.firstChild().toElement();
        ui->read(root);
    }

    bool rtn = write(ui);
    delete ui;

    return rtn;
}

bool Uic::write(DomUI *ui)
{
    if (!ui || !ui->elementWidget())
        return false;

    if (opt.copyrightHeader)
        writeCopyrightHeader(ui);

    if (opt.headerProtection)
        writeHeaderProtectionStart();

    pixFunction = ui->elementPixmapFunction();
    if (pixFunction == QLatin1String("QPixmap::fromMimeSource"))
        pixFunction = QLatin1String("qPixmapFromMimeSource");

    externalPix = ui->elementImages() == 0;

    info.accept(ui);
    cWidgetsInfo.accept(ui);
    WriteIncludes(this).accept(ui);

    if (opt.generateNamespace)
        out << "namespace Ui {\n\n";

    Validator(this).accept(ui);
    WriteDeclaration(this).accept(ui);
    WriteInitialization(this).accept(ui);
    WriteIconInitialization(this).accept(ui);

    if (opt.generateNamespace)
        out << "}\n\n";

    if (opt.headerProtection)
        writeHeaderProtectionEnd();

    return true;
}

void Uic::writeHeaderProtectionStart()
{
    QString h = drv->headerFileName();
    out << "#ifndef " << h << "\n"
        << "#define " << h << "\n\n";
}

void Uic::writeHeaderProtectionEnd()
{
    QString h = drv->headerFileName();
    out << "#endif // " << h << "\n\n";
}

bool Uic::isMainWindow(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("Q3MainWindow"))
        || customWidgetsInfo()->extends(className, QLatin1String("QMainWindow"));
}

bool Uic::isToolBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("Q3ToolBar"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolBar"));
}

bool Uic::isButton(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QRadioButton"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolButton"))
        || customWidgetsInfo()->extends(className, QLatin1String("QCheckBox"))
        || customWidgetsInfo()->extends(className, QLatin1String("QPushButton"));
}

bool Uic::isContainer(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QStackedBox"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolBox"))
        || customWidgetsInfo()->extends(className, QLatin1String("QTabWidget"))
        || customWidgetsInfo()->extends(className, QLatin1String("QWidgetStack"))
        || customWidgetsInfo()->extends(className, QLatin1String("QWizard"));
}

bool Uic::isStatusBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QStatusBar"));
}

bool Uic::isMenuBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"));
}

bool Uic::isMenu(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QMenu"))
        || customWidgetsInfo()->extends(className, QLatin1String("QPopupMenu"));
}
