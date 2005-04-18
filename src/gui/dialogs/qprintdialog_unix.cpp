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

#ifndef QT_NO_PRINTDIALOG

#include "qplatformdefs.h"

#include <private/qabstractprintdialog_p.h>
#include "qprintdialog.h"

#include "qfiledialog.h"
#include "qdir.h"
#include "qdesktopwidget.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qcombobox.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qprinter.h"
#include "qlayout.h"
#include "qbuttongroup.h"
#include "qradiobutton.h"
#include "qspinbox.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qstring.h"
#include "qregexp.h"
#include "qgroupbox.h"
#include "qsignalmapper.h"
#include "qmap.h"
#include "qabstractitemmodel.h"
#include "qtreeview.h"
#include "qheaderview.h"

#if !defined(QT_NO_CUPS) || !defined(QT_NO_NIS)
#include "qlibrary.h"
#endif

#ifndef QT_NO_NIS

#ifndef BOOL_DEFINED
#define BOOL_DEFINED
#endif

#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>

#endif // QT_NO_NIS

#include <ctype.h>
#include <stdlib.h>

enum { Success = 's', Unavail = 'u', NotFound = 'n', TryAgain = 't' };
enum { Continue = 'c', Return = 'r' };


struct QPrinterDescription {
    QPrinterDescription(const QString &n, const QString &h, const QString &c, const QStringList &a)
        : name(n), host(h), comment(c), aliases(a) {}
    QString name;
    QString host;
    QString comment;
    QStringList aliases;
    bool samePrinter(const QString& printer) const {
        return name == printer || aliases.contains(printer);
    }
};

class QPrinterModel : public QAbstractTableModel
{
public:
    QPrinterModel(const QList<QPrinterDescription> &printers, QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QList<QPrinterDescription> lst;
};

QPrinterModel::QPrinterModel(const QList<QPrinterDescription> &printers, QObject *parent)
    : QAbstractTableModel(parent)
{
    lst = printers;
}

int QPrinterModel::rowCount(const QModelIndex &) const
{
    return lst.count();
}

int QPrinterModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant QPrinterModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < (int)lst.count()
        && role == Qt::DisplayRole) {
        const QPrinterDescription &desc = lst.at(index.row());
        switch(index.column()) {
        case 0:
            return desc.name;
        case 1:
            return desc.host;
        case 2:
            return desc.comment;
        }
    }
    return QVariant();
}

QVariant QPrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        const char *name = 0;
        switch(section) {
        case 0:
            name = "Printer";
            break;
        case 1:
            name = "Host";
            break;
        case 2:
            name = "Comment";
            break;
        }
        return qApp->translate("QPrintDialog", name);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialog)
public:
    QButtonGroup *printerOrFile;

    bool outputToFile;
    QList<QPrinterDescription> printers;
    QPrinterModel *model;
    QTreeView *view;

    QLineEdit *fileName;
    QPushButton *browse, *ok;

    QButtonGroup *printRange;
    QLabel *firstPageLabel;
    QSpinBox *firstPage;
    QLabel *lastPageLabel;
    QSpinBox *lastPage;
    QRadioButton *printAllButton;
    QRadioButton *printRangeButton;
    QRadioButton *printSelectionButton;
    QRadioButton *printToFileButton;
    QComboBox *orientationCombo, *sizeCombo;

    QPrinter::PageSize pageSize;
    QPrinter::Orientation orientation;

    QButtonGroup *pageOrder;
    QRadioButton *firstPageFirst;
    QRadioButton *lastPageFirst;
    QPrinter::PageOrder pageOrder2;

    QButtonGroup *colorMode;
    QRadioButton *printColor;
    QRadioButton *printGray;
    QPrinter::ColorMode colorMode2;

    QSpinBox *copies;
    int numCopies;

    QBoxLayout *customLayout;

    QPrinter::PageSize indexToPageSize[QPrinter::NPageSize];

    void init();

    void browseClicked();
    void okClicked();
    void printerOrFileSelected(QAbstractButton *b);
    void landscapeSelected(int);
    void paperSizeSelected(int);
    void orientSelected(int);
    void pageOrderSelected(QAbstractButton *);
    void colorModeSelected(QAbstractButton *);
    void setNumCopies(int);
    void printRangeSelected(QAbstractButton *);
    void setFirstPage(int);
    void setLastPage(int);
    void fileNameEditChanged(const QString &text);

    QGroupBox *setupDestination();
    QGroupBox *setupOptions();
    QGroupBox *setupPaper();
    QGroupBox *setupPrinterSettings();

    void setPrinter(QPrinter *p, bool pickUpSettings);
};

static void isc(QPrintDialogPrivate *d, const QString & text,
                 QPrinter::PageSize ps);



static void perhapsAddPrinter(QList<QPrinterDescription> *printers, const QString &name,
                               QString host, QString comment,
                               QStringList aliases = QStringList())
{
    for (int i = 0; i < printers->size(); ++i)
        if (printers->at(i).samePrinter(name))
            return;

    if (host.isEmpty())
        host = QPrintDialog::tr("locally connected");
    printers->append(QPrinterDescription(name.simplified(), host.simplified(), comment.simplified(), aliases));
}

static void parsePrinterDesc(QString printerDesc, QList<QPrinterDescription> *printers)
{
    if (printerDesc.length() < 1)
        return;

    printerDesc = printerDesc.simplified();
    int i = printerDesc.indexOf(':');
    QString printerName, printerComment, printerHost;
    QStringList aliases;

    if (i >= 0) {
        // have ':' want '|'
        int j = printerDesc.indexOf('|');
        if (j > 0 && j < i) {
            printerName = printerDesc.left(j);
            aliases = printerDesc.mid(j + 1, i - j - 1).split('|');
            // try extracting a comment from the aliases
            printerComment = QPrintDialog::tr("Aliases: %1")
                             .arg(aliases.join(", "));
        } else {
            printerName = printerDesc.left(i);
        }
        // look for lprng pseudo all printers entry
        i = printerDesc.indexOf(QRegExp(QLatin1String(": *all *=")));
        if (i >= 0)
            printerName = "";
        // look for signs of this being a remote printer
        i = printerDesc.indexOf(QRegExp(QLatin1String(": *rm *=")));
        if (i >= 0) {
            // point k at the end of remote host name
            while (printerDesc[i] != '=')
                i++;
            while (printerDesc[i] == '=' || printerDesc[i].isSpace())
                i++;
            j = i;
            while (j < (int)printerDesc.length() && printerDesc[j] != ':')
                j++;

            // and stuff that into the string
            printerHost = printerDesc.mid(i, j - i);
        }
    }
    if (printerName.length())
        perhapsAddPrinter(printers, printerName, printerHost, printerComment,
                           aliases);
}

static int parsePrintcap(QList<QPrinterDescription> *printers, const QString& fileName)
{
    QFile printcap(fileName);
    if (!printcap.open(QIODevice::ReadOnly))
        return NotFound;

    char *line_ascii = new char[1025];
    line_ascii[1024] = '\0';

    QString printerDesc;
    bool atEnd = false;

    while (!atEnd) {
        if (printcap.atEnd() || printcap.readLine(line_ascii, 1024) <= 0)
            atEnd = true;
        QString line = line_ascii;
        line = line.trimmed();
        if (line.length() >= 1 && line[int(line.length()) - 1] == '\\')
            line.chop(1);
        if (line[0] == '#') {
            if (!atEnd)
                continue;
        } else if (line[0] == '|' || line[0] == ':') {
            printerDesc += line;
            if (!atEnd)
                continue;
        }

        parsePrinterDesc(printerDesc, printers);

        // add the first line of the new printer definition
        printerDesc = line;
    }
    delete[] line_ascii;
    return Success;
}


// solaris, not 2.6
static void parseEtcLpPrinters(QList<QPrinterDescription> *printers)
{
    QDir lp(QLatin1String("/etc/lp/printers"));
    QFileInfoList dirs = lp.entryInfoList();
    if (dirs.isEmpty())
        return;

    QString tmp;
    for (int i = 0; i < dirs.size(); ++i) {
        QFileInfo printer = dirs.at(i);
        if (printer.isDir()) {
            tmp.sprintf("/etc/lp/printers/%s/configuration",
                         printer.fileName().toAscii().data());
            QFile configuration(tmp);
            char *line = new char[1025];
            QString remote(QLatin1String("Remote:"));
            QString contentType(QLatin1String("Content types:"));
            QString printerHost;
            bool canPrintPostscript = false;
            if (configuration.open(QIODevice::ReadOnly)) {
                while (!configuration.atEnd() &&
                        configuration.readLine(line, 1024) > 0) {
                    if (QString::fromLatin1(line).startsWith(remote)) {
                        const char *p = line;
                        while (*p != ':')
                            p++;
                        p++;
                        while (isspace((uchar) *p))
                            p++;
                        printerHost = QString::fromLocal8Bit(p);
                        printerHost = printerHost.simplified();
                    } else if (QString::fromLatin1(line).startsWith(contentType)) {
                        char *p = line;
                        while (*p != ':')
                            p++;
                        p++;
                        char *e;
                        while (*p) {
                            while (isspace((uchar) *p))
                                p++;
                            if (*p) {
                                char s;
                                e = p;
                                while (isalnum((uchar) *e))
                                    e++;
                                s = *e;
                                *e = '\0';
                                if (!qstrcmp(p, "postscript") ||
                                     !qstrcmp(p, "any"))
                                    canPrintPostscript = true;
                                *e = s;
                                if (s == ',')
                                    e++;
                                p = e;
                            }
                        }
                    }
                }
                if (canPrintPostscript)
                    perhapsAddPrinter(printers, printer.fileName(),
                                       printerHost, QLatin1String(""));
            }
            delete[] line;
        }
    }
}


// solaris 2.6
static char *parsePrintersConf(QList<QPrinterDescription> *printers, bool *found = 0)
{
    QFile pc(QLatin1String("/etc/printers.conf"));
    if (!pc.open(QIODevice::ReadOnly)) {
        if (found)
            *found = false;
        return 0;
    }
    if (found)
        *found = true;

    char *line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength = 0;

    char *defaultPrinter = 0;

    while (!pc.atEnd() &&
            (lineLength=pc.readLine(line, 1024)) > 0) {
        if (*line == '#') {
            *line = '\0';
            lineLength = 0;
        }
        if (lineLength >= 2 && line[lineLength-2] == '\\') {
            line[lineLength-2] = '\0';
            printerDesc += QString::fromLocal8Bit(line);
        } else {
            printerDesc += QString::fromLocal8Bit(line);
            printerDesc = printerDesc.simplified();
            int i = printerDesc.indexOf(':');
            QString printerName, printerHost, printerComment;
            QStringList aliases;
            if (i >= 0) {
                // have : want |
                int j = printerDesc.indexOf('|');
                if (j >= i)
                    j = -1;
                printerName = printerDesc.mid(0, j < 0 ? i : j);
                if (printerName == QLatin1String("_default")) {
                    i = printerDesc.indexOf(
                        QRegExp(QLatin1String(": *use *=")));
                    while (printerDesc[i] != '=')
                        i++;
                    while (printerDesc[i] == '=' || printerDesc[i].isSpace())
                        i++;
                    j = i;
                    while (j < (int)printerDesc.length() &&
                            printerDesc[j] != ':' && printerDesc[j] != ',')
                        j++;
                    // that's our default printer
                    defaultPrinter =
                        qstrdup(printerDesc.mid(i, j-i).toAscii().data());
                    printerName = "";
                    printerDesc = "";
                } else if (printerName == QLatin1String("_all")) {
                    // skip it.. any other cases we want to skip?
                    printerName = "";
                    printerDesc = "";
                }

                if (j > 0) {
                    // try extracting a comment from the aliases
                    aliases = printerDesc.mid(j + 1, i - j - 1).split('|');
                    printerComment = QPrintDialog::tr("Aliases: %1")
                                     .arg(aliases.join(", "));
                }
                // look for signs of this being a remote printer
                i = printerDesc.indexOf(
                    QRegExp(QLatin1String(": *bsdaddr *=")));
                if (i >= 0) {
                    // point k at the end of remote host name
                    while (printerDesc[i] != '=')
                        i++;
                    while (printerDesc[i] == '=' || printerDesc[i].isSpace())
                        i++;
                    j = i;
                    while (j < (int)printerDesc.length() &&
                            printerDesc[j] != ':' && printerDesc[j] != ',')
                        j++;
                    // and stuff that into the string
                    printerHost = printerDesc.mid(i, j-i);
                    // maybe stick the remote printer name into the comment
                    if (printerDesc[j] == ',') {
                        i = ++j;
                        while (printerDesc[i].isSpace())
                            i++;
                        j = i;
                        while (j < (int)printerDesc.length() &&
                                printerDesc[j] != ':' && printerDesc[j] != ',')
                            j++;
                        if (printerName != printerDesc.mid(i, j-i)) {
                            printerComment =
                                QLatin1String("Remote name: ");
                            printerComment += printerDesc.mid(i, j-i);
                        }
                    }
                }
            }
            if (printerComment == ":")
                printerComment = ""; // for cups
            if (printerName.length())
                perhapsAddPrinter(printers, printerName, printerHost,
                                   printerComment, aliases);
            // chop away the line, for processing the next one
            printerDesc = "";
        }
    }
    delete[] line;
    return defaultPrinter;
}

#ifndef QT_NO_NIS

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static int pd_foreach(int /*status */, char * /*key */, int /*keyLen */,
                    char *val, int valLen, char *data)
{
    parsePrinterDesc(QString::fromLatin1(val, valLen), (QList<QPrinterDescription> *)data);
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif

static int retrieveNisPrinters(QList<QPrinterDescription> *printers)
{
    typedef int (*WildCast)(int, char *, int, char *, int, char *);
    char printersConfByname[] = "printers.conf.byname";
    char *domain;
    int err;

    QLibrary lib("nsl");
    typedef int (*ypGetDefaultDomain)(char **);
    ypGetDefaultDomain _ypGetDefaultDomain = (ypGetDefaultDomain)lib.resolve("yp_get_default_domain");
    typedef int (*ypAll)(const char *, const char *, const struct ypall_callback *);
    ypAll _ypAll = (ypAll)lib.resolve("yp_all");

    if (_ypGetDefaultDomain && _ypAll) {
        err = _ypGetDefaultDomain(&domain);
        if (err == 0) {
            ypall_callback cb;
            // wild cast to support K&R-style system headers
            (WildCast &) cb.foreach = (WildCast) pd_foreach;
            cb.data = (char *) printers;
            err = _ypAll(domain, printersConfByname, &cb);
        }
        if (!err)
            return Success;
    }
    return Unavail;
}

#endif // QT_NO_NIS

static char *parseNsswitchPrintersEntry(QList<QPrinterDescription> *printers, char *line)
{
#define skipSpaces() \
    while (isspace((uchar) line[k])) \
        k++

    char *defaultPrinter = 0;
    bool stop = false;
    int lastStatus = NotFound;

    int k = 8;
    skipSpaces();
    if (line[k] != ':')
        return 0;
    k++;

    char *cp = strchr(line, '#');
    if (cp != 0)
        *cp = '\0';

    while (line[k] != '\0') {
        if (isspace((uchar) line[k])) {
            k++;
        } else if (line[k] == '[') {
            k++;
            skipSpaces();
            while (line[k] != '\0') {
                char status = tolower(line[k]);
                char action = '?';

                while (line[k] != '=' && line[k] != ']' && line[k] != '\0')
                    k++;
                if (line[k] == '=') {
                    k++;
                    skipSpaces();
                    action = tolower(line[k]);
                    while (line[k] != '\0' && !isspace((uchar) line[k]) && line[k] != ']')
                        k++;
                } else if (line[k] == ']') {
                    k++;
                    break;
                }
                skipSpaces();

                if (lastStatus == status)
                    stop = (action == (char) Return);
            }
        } else {
            if (stop)
                break;

            QByteArray source;
            while (!isspace((uchar) line[k]) && line[k] != '[') {
                source += line[k];
                k++;
            }

            if (source == "user") {
                lastStatus = parsePrintcap(printers,
                        QDir::homePath() + "/.printers");
            } else if (source == "files") {
                bool found;
                defaultPrinter = parsePrintersConf(printers, &found);
                if (found)
                    lastStatus = Success;
#ifndef QT_NO_NIS
            } else if (source == "nis") {
                lastStatus = retrieveNisPrinters(printers);
#endif
            } else {
                // nisplus, dns, etc., are not implemented yet
                lastStatus = NotFound;
            }
            stop = (lastStatus == Success);
        }
    }
    return defaultPrinter;
}

static char *parseNsswitchConf(QList<QPrinterDescription> *printers)
{
    QFile nc(QLatin1String("/etc/nsswitch.conf"));
    if (!nc.open(QIODevice::ReadOnly))
        return 0;

    char *defaultPrinter = 0;

    char *line = new char[1025];
    line[1024] = '\0';

    while (!nc.atEnd() &&
            nc.readLine(line, 1024) > 0) {
        if (strncmp(line, "printers", 8) == 0) {
            defaultPrinter = parseNsswitchPrintersEntry(printers, line);
            delete[] line;
            return defaultPrinter;
        }
    }

    strcpy(line, "printers: user files nis nisplus xfn");
    defaultPrinter = parseNsswitchPrintersEntry(printers, line);
    delete[] line;
    return defaultPrinter;
}

// HP-UX
static void parseEtcLpMember(QList<QPrinterDescription> *printers)
{
    QDir lp(QLatin1String("/etc/lp/member"));
    if (!lp.exists())
        return;
    QFileInfoList dirs = lp.entryInfoList();
    if (dirs.isEmpty())
        return;

    QString tmp;
    for (int i = 0; i < dirs.size(); ++i) {
        QFileInfo printer = dirs.at(i);
        // I haven't found any real documentation, so I'm guessing that
        // since lpstat uses /etc/lp/member rather than one of the
        // other directories, it's the one to use.  I did not find a
        // decent way to locate aliases and remote printers.
        if (printer.isFile())
            perhapsAddPrinter(printers, printer.fileName(),
                               QPrintDialog::tr("unknown"),
                               QLatin1String(""));
    }
}

// IRIX 6.x
static void parseSpoolInterface(QList<QPrinterDescription> *printers)
{
    QDir lp(QLatin1String("/usr/spool/lp/interface"));
    if (!lp.exists())
        return;
    QFileInfoList files = lp.entryInfoList();
    if(files.isEmpty())
        return;

    for (int i = 0; i < files.size(); ++i) {
        QFileInfo printer = files.at(i);

        if (!printer.isFile())
            continue;

        // parse out some information
        QFile configFile(printer.filePath());
        if (!configFile.open(QIODevice::ReadOnly))
            continue;

        QByteArray line;
        line.resize(1025);
        QString namePrinter;
        QString hostName;
        QString hostPrinter;
        QString printerType;

        QString nameKey(QLatin1String("NAME="));
        QString typeKey(QLatin1String("TYPE="));
        QString hostKey(QLatin1String("HOSTNAME="));
        QString hostPrinterKey(QLatin1String("HOSTPRINTER="));

        while (!configFile.atEnd() &&
                (configFile.readLine(line.data(), 1024)) > 0) {
            QString uline = line;
            if (uline.startsWith(typeKey) ) {
                printerType = line.mid(nameKey.length());
                printerType = printerType.simplified();
            } else if (uline.startsWith(hostKey)) {
                hostName = line.mid(hostKey.length());
                hostName = hostName.simplified();
            } else if (uline.startsWith(hostPrinterKey)) {
                hostPrinter = line.mid(hostPrinterKey.length());
                hostPrinter = hostPrinter.simplified();
            } else if (uline.startsWith(nameKey)) {
                namePrinter = line.mid(nameKey.length());
                namePrinter = namePrinter.simplified();
            }
        }
        configFile.close();

        printerType = printerType.trimmed();
        if (printerType.indexOf("postscript", 0, Qt::CaseInsensitive) < 0)
            continue;

        int ii = 0;
        while ((ii = namePrinter.indexOf('"', ii)) >= 0)
            namePrinter.remove(ii, 1);

        if (hostName.isEmpty() || hostPrinter.isEmpty()) {
            perhapsAddPrinter(printers, printer.fileName(),
                               QLatin1String(""), namePrinter);
        } else {
            QString comment;
            comment = namePrinter;
            comment += " (";
            comment += hostPrinter;
            comment += ")";
            perhapsAddPrinter(printers, printer.fileName(),
                               hostName, comment);
        }
    }
}


// Every unix must have its own.  It's a standard.  Here is AIX.
static void parseQconfig(QList<QPrinterDescription> *printers)
{
    QFile qconfig(QLatin1String("/etc/qconfig"));
    if (!qconfig.open(QIODevice::ReadOnly))
        return;

    QTextStream ts(&qconfig);
    QString line;

    QString stanzaName; // either a queue or a device name
    bool up = true; // queue up?  default true, can be false
    QString remoteHost; // null if local
    QString deviceName; // null if remote

    QRegExp newStanza(QLatin1String("^[0-z\\-]*:$"));

    // our basic strategy here is to process each line, detecting new
    // stanzas.  each time we see a new stanza, we check if the
    // previous stanza was a valid queue for a) a remote printer or b)
    // a local printer.  if it wasn't, we assume that what we see is
    // the start of the first stanza, or that the previous stanza was
    // a device stanza, or that there is some syntax error (we don't
    // report those).

    do {
        line = ts.readLine();
        bool indented = line[0].isSpace();
        line = line.simplified();

        int i = line.indexOf('=');
        if (indented && i != -1) { // line in stanza
            QString variable = line.left(i).simplified();
            QString value=line.mid(i+1, line.length()).simplified();
            if (variable == QLatin1String("device"))
                deviceName = value;
            else if (variable == QLatin1String("host"))
                remoteHost = value;
            else if (variable == QLatin1String("up"))
                up = !(value.toLower() == QLatin1String("false"));
        } else if (line[0] == '*') { // comment
            // nothing to do
        } else if (ts.atEnd() || // end of file, or beginning of new stanza
                    (!indented && line.contains(newStanza))) {
            if (up && stanzaName.length() > 0 && stanzaName.length() < 21) {
                if (remoteHost.length()) // remote printer
                    perhapsAddPrinter(printers, stanzaName, remoteHost,
                                       QString::null);
                else if (deviceName.length()) // local printer
                    perhapsAddPrinter(printers, stanzaName, QString::null,
                                       QString::null);
            }
            line.chop(1);
            if (line.length() >= 1 && line.length() <= 20)
                stanzaName = line;
            up = true;
            remoteHost = QString::null;
            deviceName = QString::null;
        } else {
            // syntax error?  ignore.
        }
    } while (!ts.atEnd());
}


#ifndef QT_NO_CUPS
#include <cups/cups.h>

static char *parseCupsOutput(QList<QPrinterDescription> *printers)
{
    char *defaultPrinter = 0;
    int nd;
    cups_dest_t *d;
    QLibrary lib("cups");
    typedef int (*CupsGetDests)(cups_dest_t **dests);
    CupsGetDests _cupsGetDests = (CupsGetDests)lib.resolve("cupsGetDests");
    if (_cupsGetDests) {
        nd = _cupsGetDests(&d);
        if (nd < 1)
            return 0;

        int n = 0;
        while (n < nd) {
            perhapsAddPrinter(printers, d[n].name,
                               QPrintDialog::tr("Unknown Location"), QString());
            if (d[n].is_default && !defaultPrinter)
                defaultPrinter = qstrdup(d[n].instance);
            n++;
        }
    }
    return defaultPrinter;
}
#endif

/*!
  \class QPrintDialog qprintdialog.h

  \brief The QPrintDialog class provides a dialog for specifying
  the printer's configuration.

  \internal

  \ingroup dialogs

  \warning The use of this class is not recommended since it is not
  present on all platforms; use QPrinter::setup() instead.

  \omit

  (ingroup dialogs)

  THIS DOCUMENTATION IS Not Revised. It must be revised before
  becoming public API.

  \endomit

  It encompasses both the sort of details needed for doing a simple
  print-out and some print configuration setup.

  The easiest way to use the class is through the static
  function getPrinterSetup().  You can also subclass the QPrintDialog
  and add some custom buttons with addButton() to extend the
  functionality of the print dialog.

  The printer dialog in Motif style:

  \img qprintdlg-m.png
*/


/*!
    Constructs a new modal printer dialog for the given \a printer
    with the given \a parent.
*/

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogPrivate), printer, parent)
{
    d_func()->init();
}


/*! Destroys the object and frees any allocated resources.  Does not
  delete the associated QPrinter object.
*/

QPrintDialog::~QPrintDialog()
{
}


QGroupBox *QPrintDialogPrivate::setupPrinterSettings()
{
    Q_Q(QPrintDialog);
    QGroupBox *g = new QGroupBox(q->tr("Printer settings"), q);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    colorMode = new QButtonGroup(q);
    QObject::connect(colorMode, SIGNAL(buttonChecked(QAbstractButton*)),
                     q, SLOT(colorModeSelected(QAbstractButton*)));

    printColor = new QRadioButton(q->tr("Print in color if available"), g);
    colorMode->addButton(printColor);
    printColor->setChecked(true);
    tll->addWidget(printColor);

    printGray = new QRadioButton(q->tr("Print in grayscale"), g);
    colorMode->addButton(printGray);
    tll->addWidget(printGray);

    return g;
}

QGroupBox *QPrintDialogPrivate::setupDestination()
{
    Q_Q(QPrintDialog);
    QGroupBox *g = new QGroupBox(q->tr("Print destination"), q);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    printerOrFile = new QButtonGroup(q);

    // printer radio button, list
    QRadioButton *rb = new QRadioButton(q->tr("Print to printer:"), g);
    tll->addWidget(rb);
    printerOrFile->addButton(rb);
    rb->setChecked(true);
    outputToFile = false;

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz, 3);
    horiz->addSpacing(19);

    char *etcLpDefault = 0;

#ifndef QT_NO_CUPS
    etcLpDefault = parseCupsOutput(&printers);
#endif
    if (printers.size() == 0) {
        // we only use other schemes when cups fails.

        parsePrintcap(&printers, QLatin1String("/etc/printcap"));
        parseEtcLpMember(&printers);
        parseSpoolInterface(&printers);
        parseQconfig(&printers);

        QFileInfo f;
        f.setFile(QLatin1String("/etc/lp/printers"));
        if (f.isDir()) {
            parseEtcLpPrinters(&printers);
            QFile def(QLatin1String("/etc/lp/default"));
            if (def.open(QIODevice::ReadOnly)) {
                if (etcLpDefault)
                    delete[] etcLpDefault;
                etcLpDefault = new char[1025];
                def.readLine(etcLpDefault, 1024);
                char *p = etcLpDefault;
                while (p && *p) {
                    if (!isprint((uchar) *p) || isspace((uchar) *p))
                        *p = 0;
                    else
                        p++;
                }
            }
        }

        char *def = 0;
        f.setFile(QLatin1String("/etc/nsswitch.conf"));
        if (f.isFile()) {
            def = parseNsswitchConf(&printers);
        } else {
            f.setFile(QLatin1String("/etc/printers.conf"));
            if (f.isFile())
                def = parsePrintersConf(&printers);
        }

        if (def) {
            if (etcLpDefault)
                delete[] etcLpDefault;
            etcLpDefault = def;
        }
    }

    // all printers hopefully known.  try to find a good default
    QString dollarPrinter;
    {
        const char *t = qgetenv("PRINTER");
        if (!t || !*t)
            t = qgetenv("LPDEST");
        dollarPrinter = QLatin1String(t);
        if (!dollarPrinter.isEmpty())
            perhapsAddPrinter(&printers, dollarPrinter,
                               QPrintDialog::tr("unknown"),
                              QLatin1String(""));
    }


    model = new QPrinterModel(printers, q);
    view = new QTreeView(g);
    view->setModel(model);
    view->setRootIsDecorated(false);
    view->header()->setResizeMode(QHeaderView::Stretch, 2);

    // bang the best default into the listview
    int quality = 0;
    int best = 0;
    for (int i = 0; i < printers.size(); ++i) {
        QRegExp ps(QLatin1String("[^a-z]ps(?:[^a-z]|$)"));
        QRegExp lp(QLatin1String("[^a-z]lp(?:[^a-z]|$)"));

        QString name = printers.at(i).name;
        QString comment = printers.at(i).comment;
        if (quality < 4 && name == dollarPrinter) {
            best = i;
            quality = 4;
        } else if (quality < 3 && etcLpDefault &&
                    name == QLatin1String(etcLpDefault)) {
            best = i;
            quality = 3;
        } else if (quality < 2 &&
                    (name == QLatin1String("ps") ||
                     ps.indexIn(comment) != -1)) {
            best = i;
            quality = 2;
        } else if (quality < 1 &&
                    (name == QLatin1String("lp") ||
                     lp.indexIn(comment) > -1)) {
            best = i;
            quality = 1;
        }
    }
    view->setCurrentIndex(static_cast<QAbstractTableModel*>(model)->index(best, 0));

    if (etcLpDefault)                 // Avoid purify complaint
        delete[] etcLpDefault;

//     int h = fontMetrics().height();
//     if (printers.size())
//         h = view->itemViewportRect(model->index(0, 0)).height();
//     view->setMinimumSize(view->sizeHint().width(),
//                                  printers->header()->height() +
//                                  3 * h);
    horiz->addWidget(view, 3);

    // file radio button, edit/browse
    printToFileButton = new QRadioButton(q->tr("Print to file:"), g);
    tll->addWidget(printToFileButton);
    printerOrFile->addButton(printToFileButton);

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);
    horiz->addSpacing(19);

    fileName = new QLineEdit(g);
    QObject::connect(fileName, SIGNAL(textChanged(QString)),
                     q, SLOT(fileNameEditChanged(QString)));
    horiz->addWidget(fileName, 1);
    browse = new QPushButton(q->tr("Browse..."), g);
    browse->setAutoDefault(false);
    QObject::connect(browse, SIGNAL(clicked()),
                     q, SLOT(browseClicked()));
    horiz->addWidget(browse);

    fileName->setEnabled(false);
    browse->setEnabled(false);

    QObject::connect(printerOrFile, SIGNAL(buttonChecked(QAbstractButton*)),
             q, SLOT(printerOrFileSelected(QAbstractButton*)));
    return g;
}


QGroupBox *QPrintDialogPrivate::setupOptions()
{
    Q_Q(QPrintDialog);
    QGroupBox *g = new QGroupBox(q->tr("Options"), q);

    QBoxLayout *lay = new QBoxLayout(QBoxLayout::LeftToRight, g);
    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down);
    lay->addLayout(tll);

    printRange = new QButtonGroup(q);
    QObject::connect(printRange, SIGNAL(buttonChecked(QAbstractButton*)), q, SLOT(printRangeSelected(QAbstractButton*)));

    pageOrder = new QButtonGroup(q);
    QObject::connect(pageOrder, SIGNAL(buttonChecked(QAbstractButton*)), q, SLOT(pageOrderSelected(QAbstractButton*)));

    printAllButton = new QRadioButton(q->tr("Print all"), g);
    printRange->addButton(printAllButton);
    tll->addWidget(printAllButton);

    printSelectionButton = new QRadioButton(q->tr("Print selection"), g);
    printRange->addButton(printSelectionButton);
    tll->addWidget(printSelectionButton);

    printRangeButton = new QRadioButton(q->tr("Print range"), g);
    printRange->addButton(printRangeButton);
    tll->addWidget(printRangeButton);

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    firstPageLabel = new QLabel(q->tr("From page:"), g);
    horiz->addSpacing(19);
    horiz->addWidget(firstPageLabel);

    firstPage = new QSpinBox(g);
    firstPage->setRange(1, 9999);
    firstPage->setValue(1);
    horiz->addWidget(firstPage, 1);
    QObject::connect(firstPage, SIGNAL(valueChanged(int)),
             q, SLOT(setFirstPage(int)));

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    lastPageLabel = new QLabel(q->tr("To page:"), g);
    horiz->addSpacing(19);
    horiz->addWidget(lastPageLabel);

    lastPage = new QSpinBox(g);
    lastPage->setRange(1, 9999);
    lastPage->setValue(9999);
    horiz->addWidget(lastPage, 1);
    QObject::connect(lastPage, SIGNAL(valueChanged(int)),
             q, SLOT(setLastPage(int)));

    lay->addSpacing(25);
    tll = new QBoxLayout(QBoxLayout::Down);
    lay->addLayout(tll);

    // print order
    firstPageFirst = new QRadioButton(q->tr("Print first page first"), g);
    tll->addWidget(firstPageFirst);
    pageOrder->addButton(firstPageFirst);
    firstPageFirst->setChecked(true);

    lastPageFirst = new QRadioButton(q->tr("Print last page first"), g);
    tll->addWidget(lastPageFirst);
    pageOrder->addButton(lastPageFirst);

    tll->addStretch();

    // copies

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    QLabel *l = new QLabel(q->tr("Number of copies:"), g);
    horiz->addWidget(l);

    copies = new QSpinBox(g);
    copies->setRange(1,99);
    copies->setValue(1);
    horiz->addWidget(copies, 1);
    QObject::connect(copies, SIGNAL(valueChanged(int)),
             q, SLOT(setNumCopies(int)));

    QSize s = firstPageLabel->sizeHint()
              .expandedTo(lastPageLabel->sizeHint())
              .expandedTo(l->sizeHint());
    firstPageLabel->setMinimumSize(s);
    lastPageLabel->setMinimumSize(s);
    l->setMinimumSize(s.width() + 19, s.height());

    return g;
}


void isc(QPrintDialogPrivate *ptr, const QString & text, QPrinter::PageSize ps)
{
    if (ptr && !text.isEmpty() && ps < QPrinter::NPageSize) {
        ptr->sizeCombo->addItem(text);
        int index = ptr->sizeCombo->count()-1;
        if (index >= 0 && index < QPrinter::NPageSize)
            ptr->indexToPageSize[index] = ps;
    }
}

QGroupBox *QPrintDialogPrivate::setupPaper()
{
    Q_Q(QPrintDialog);
    QGroupBox *g = new QGroupBox(q->tr("Paper format"), q);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    pageSize = QPrinter::A4;

    // page orientation
    orientationCombo = new QComboBox(g);
    orientationCombo->addItem(q->tr("Portrait"));
    orientationCombo->addItem(q->tr("Landscape"));
    tll->addWidget(orientationCombo);

    orientation = QPrinter::Portrait;

    QObject::connect(orientationCombo, SIGNAL(activated(int)),
             q, SLOT(orientSelected(int)));

    // paper size
    sizeCombo = new QComboBox(g);
    tll->addWidget(sizeCombo);

    int n;
    for(n=0; n<QPrinter::NPageSize; n++)
        indexToPageSize[n] = QPrinter::A4;

    isc(this, q->tr("A0 (841 x 1189 mm)"), QPrinter::A0);
    isc(this, q->tr("A1 (594 x 841 mm)"), QPrinter::A1);
    isc(this, q->tr("A2 (420 x 594 mm)"), QPrinter::A2);
    isc(this, q->tr("A3 (297 x 420 mm)"), QPrinter::A3);
    isc(this, q->tr("A4 (210x297 mm, 8.26x11.7 inches)"), QPrinter::A4);
    isc(this, q->tr("A5 (148 x 210 mm)"), QPrinter::A5);
    isc(this, q->tr("A6 (105 x 148 mm)"), QPrinter::A6);
    isc(this, q->tr("A7 (74 x 105 mm)"), QPrinter::A7);
    isc(this, q->tr("A8 (52 x 74 mm)"), QPrinter::A8);
    isc(this, q->tr("A9 (37 x 52 mm)"), QPrinter::A9);
    isc(this, q->tr("B0 (1000 x 1414 mm)"), QPrinter::B0);
    isc(this, q->tr("B1 (707 x 1000 mm)"), QPrinter::B1);
    isc(this, q->tr("B2 (500 x 707 mm)"), QPrinter::B2);
    isc(this, q->tr("B3 (353 x 500 mm)"), QPrinter::B3);
    isc(this, q->tr("B4 (250 x 353 mm)"), QPrinter::B4);
    isc(this, q->tr("B5 (176 x 250 mm, 6.93x9.84 inches)"), QPrinter::B5);
    isc(this, q->tr("B6 (125 x 176 mm)"), QPrinter::B6);
    isc(this, q->tr("B7 (88 x 125 mm)"), QPrinter::B7);
    isc(this, q->tr("B8 (62 x 88 mm)"), QPrinter::B8);
    isc(this, q->tr("B9 (44 x 62 mm)"), QPrinter::B9);
    isc(this, q->tr("B10 (31 x 44 mm)"), QPrinter::B10);
    isc(this, q->tr("C5E (163 x 229 mm)"), QPrinter::C5E);
    isc(this, q->tr("DLE (110 x 220 mm)"), QPrinter::DLE);
    isc(this, q->tr("Executive (7.5x10 inches, 191x254 mm)"), QPrinter::Executive);
    isc(this, q->tr("Folio (210 x 330 mm)"), QPrinter::Folio);
    isc(this, q->tr("Ledger (432 x 279 mm)"), QPrinter::Ledger);
    isc(this, q->tr("Legal (8.5x14 inches, 216x356 mm)"), QPrinter::Legal);
    isc(this, q->tr("Letter (8.5x11 inches, 216x279 mm)"), QPrinter::Letter);
    isc(this, q->tr("Tabloid (279 x 432 mm)"), QPrinter::Tabloid);
    isc(this, q->tr("US Common #10 Envelope (105 x 241 mm)"), QPrinter::Comm10E);

    QObject::connect(sizeCombo, SIGNAL(activated(int)),
             q, SLOT(paperSizeSelected(int)));

    return g;
}


void QPrintDialogPrivate::printerOrFileSelected(QAbstractButton *b)
{
    outputToFile = (b == printToFileButton);
    if (outputToFile) {
        ok->setEnabled(true);
        fileNameEditChanged(fileName->text());
        if (!fileName->isModified() && fileName->text().isEmpty()) {
            QString home = QLatin1String(::qgetenv("HOME"));
            QString cur = QDir::currentPath();
            if (home.at(home.length()-1) != '/')
                home += '/';
            if (cur.at(cur.length()-1) != '/')
                cur += '/';
            if (cur.left(home.length()) != home)
                cur = home;
#ifdef Q_WS_X11
            cur += "print.ps";
#endif
            fileName->setText(cur);
            fileName->setCursorPosition(cur.length());
            fileName->selectAll();
        }
        browse->setEnabled(true);
        fileName->setEnabled(true);
        fileName->setFocus();
        view->setEnabled(false);
    } else {
        ok->setEnabled(printers.count() != 0);
        view->setEnabled(true);
        if (fileName->hasFocus() || browse->hasFocus())
            view->setFocus();
        browse->setEnabled(false);
        fileName->setEnabled(false);
    }
}


void QPrintDialogPrivate::landscapeSelected(int id)
{
    orientation = (QPrinter::Orientation)id;
}


void QPrintDialogPrivate::paperSizeSelected(int id)
{
    if (id < QPrinter::NPageSize)
        pageSize = QPrinter::PageSize(indexToPageSize[id]);
}


void QPrintDialogPrivate::orientSelected(int id)
{
    orientation = (QPrinter::Orientation)id;
}


void QPrintDialogPrivate::pageOrderSelected(QAbstractButton *b)
{
    pageOrder2 = (b == firstPageFirst) ? QPrinter::FirstPageFirst : QPrinter::LastPageFirst;
}


void QPrintDialogPrivate::setNumCopies(int copies)
{
    numCopies = copies;
}


void QPrintDialogPrivate::browseClicked()
{
    Q_Q(QPrintDialog);
    QString fn = QFileDialog::getSaveFileName(q, QString(), fileName->text(),
                                              q->tr("PostScript Files (*.ps);;All Files (*)"));
    if (!fn.isNull())
        fileName->setText(fn);
}


void QPrintDialogPrivate::okClicked()
{
    Q_Q(QPrintDialog);
    lastPage->interpretText();
    firstPage->interpretText();
    copies->interpretText();
    if (outputToFile) {
        printer->setOutputFileName(fileName->text());
    } else {
        printer->setOutputFileName(QString());
        QModelIndex current = view->currentIndex();
        if (current.isValid())
            printer->setPrinterName(printers.at(current.row()).name);
    }

    printer->setOrientation(orientation);
    printer->setPageSize(pageSize);
    printer->setPageOrder(pageOrder2);
    printer->setColorMode(colorMode2);
    printer->setNumCopies(numCopies);
    if (printAllButton->isChecked()) {
        q->setPrintRange(QPrintDialog::AllPages);
        q->setFromTo(q->minPage(), q->maxPage());
    } else {
        if (printSelectionButton->isChecked()) {
            q->setPrintRange(QPrintDialog::Selection);
            q->setFromTo(0, 0);
        } else {
            q->setPrintRange(QPrintDialog::PageRange);
            q->setFromTo(firstPage->value(), lastPage->value());
        }
    }
    q->accept();
}


void QPrintDialogPrivate::printRangeSelected(QAbstractButton *b)
{
    bool enable = (b == printRangeButton);
    firstPage->setEnabled(enable);
    lastPage->setEnabled(enable);
    firstPageLabel->setEnabled(enable);
    lastPageLabel->setEnabled(enable);
}


void QPrintDialogPrivate::setFirstPage(int fp)
{
    Q_Q(QPrintDialog);
    if (printer) {
        lastPage->setMinimum(fp);
        lastPage->setMaximum(qMax(fp, q->maxPage()));
    }
}


void QPrintDialogPrivate::setLastPage(int lp)
{
    Q_Q(QPrintDialog);
    if (printer) {
        firstPage->setMinimum(qMin(lp, q->minPage()));
        firstPage->setMaximum(lp);
    }
}


#ifdef QT3_SUPPORT
/*!
  Adds the button \a but to the layout of the print dialog. The added
  buttons are arranged from the left to the right below the
  last groupbox of the printdialog.
*/
void QPrintDialog::addButton(QPushButton *but)
{
    Q_D(QPrintDialog);
    d->customLayout->addWidget(but);
}

/*!  Returns a pointer to the printer this dialog configures, or 0 if
  this dialog does not operate on any printer. */
QPrinter *QPrintDialog::printer() const
{
    Q_D(const QPrintDialog);
    return d->printer;
}

/*!
  Sets this dialog to configure printer \a p, or no printer if \a p
  is null. If \a pickUpSettings is true, the dialog reads most of
  its settings from \a p. If \a pickUpSettings is false (the
  default) the dialog keeps its old settings.
*/

void QPrintDialog::setPrinter(QPrinter *p, bool pickUpSettings)
{
    Q_D(QPrintDialog);
    d->setPrinter(p, pickUpSettings);
}
#endif

void QPrintDialogPrivate::setPrinter(QPrinter *p, bool pickUpSettings)
{
    Q_Q(QPrintDialog);
    printer = p;

    if (p && pickUpSettings) {
        // top to botton in the old dialog.
        // printer or file
        if (!p->outputFileName().isEmpty())
            printToFileButton->setChecked(true);

        // printer name
        if (!p->printerName().isEmpty()) {
            for (int i = 0; i < printers.size(); ++i) {
                if (printers.at(i).name == p->printerName()) {
                    // ###############
//                    printers->setSelected(i, true);
                    ok->setEnabled(true);
                } else if (fileName->text().isEmpty()) {
                    ok->setEnabled(model->rowCount() != 0);
                }
            }
        }

        // print command does not exist any more

        // file name
        printToFileButton->setEnabled(q->isOptionEnabled(QPrintDialog::PrintToFile));
        fileName->setText(p->outputFileName());

        // orientation
        orientationCombo->setCurrentIndex((int)p->orientation());
        orientSelected(p->orientation());

        // page size
        int n = 0;
        while (n < QPrinter::NPageSize &&
                indexToPageSize[n] != p->pageSize())
            n++;
        sizeCombo->setCurrentIndex(n);
        paperSizeSelected(n);

        // New stuff (Options)

        // page order
        pageOrder2 = p->pageOrder();
        if (pageOrder2 == QPrinter::LastPageFirst)
            lastPageFirst->setChecked(true);

        // color mode
        colorMode2 = p->colorMode();
        if (colorMode2 == QPrinter::Color)
            printColor->setChecked(true);

        // number of copies
        copies->setValue(p->numCopies());
        setNumCopies(p->numCopies());
    }

    if(p) {
        printAllButton->setEnabled(true);
        printSelectionButton->setEnabled(q->isOptionEnabled(QPrintDialog::PrintSelection));
        printRangeButton->setEnabled(q->isOptionEnabled(QPrintDialog::PrintPageRange));

        switch (q->printRange()) {
        case QPrintDialog::AllPages:
            printAllButton->click();
            break;
        case QPrintDialog::Selection:
            printSelectionButton->click();
            break;
        case QPrintDialog::PageRange:
            printRangeButton->click();
            break;
        }
    }

    if (p && q->maxPage()) {
        firstPage->setMinimum(q->minPage());
        firstPage->setMaximum(q->maxPage());
        lastPage->setMinimum(q->minPage());
        lastPage->setMaximum(q->maxPage());
        if (q->fromPage() || q->toPage()) {
            setFirstPage(q->fromPage());
            setLastPage(q->toPage());
            firstPage->setValue(q->fromPage());
            lastPage->setValue(q->toPage());
        }
    }
}

void QPrintDialogPrivate::colorModeSelected(QAbstractButton *b)
{
    colorMode2 = (b == printColor) ? QPrinter::Color : QPrinter::GrayScale;
}

void QPrintDialogPrivate::fileNameEditChanged(const QString &text)
{
    if (fileName->isEnabled())
        ok->setEnabled(!text.isEmpty());
}

int QPrintDialog::exec()
{
    return QDialog::exec();
}

void QPrintDialogPrivate::init()
{
    Q_Q(QPrintDialog);
    numCopies = 1;

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, q);

    // destination
    QGroupBox *g;
    g = setupDestination();
    tll->addWidget(g, 1);

    // printer and paper settings
    QBoxLayout *lay = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(lay);

    g = setupPrinterSettings();
    lay->addWidget(g, 1);

    g = setupPaper();
    lay->addWidget(g);

    // options
    g = setupOptions();
    tll->addWidget(g);

    QBoxLayout *l = new QBoxLayout(QBoxLayout::LeftToRight);
    customLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(l);
    l->addLayout(customLayout);
    l->addStretch();

    // buttons
    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    bool rightalign =
        bool(q->style()->styleHint(QStyle::SH_PrintDialog_RightAlignButtons, 0, q));

    if (rightalign)
        horiz->addStretch(1);

    ok = new QPushButton(q->tr("OK"), q);
    ok->setDefault(true);
    horiz->addWidget(ok);
    if (! rightalign)
        horiz->addStretch(1);

    QPushButton *cancel = new QPushButton(q->tr("Cancel"), q);
    horiz->addWidget(cancel);

    q->connect(ok, SIGNAL(clicked()), q, SLOT(okClicked()));
    q->connect(cancel, SIGNAL(clicked()), q,  SLOT(reject()));

    QSize ms(q->minimumSize());
    QSize ss(QApplication::desktop()->screenGeometry(q->pos()).size());
    if (ms.height() < 512 && ss.height() >= 600)
        ms.setHeight(512);
    else if (ms.height() < 460 && ss.height() >= 480)
        ms.setHeight(460);
    q->resize(ms);

    setPrinter(printer, true);
    view->setFocus();
}

#include "moc_qprintdialog.cpp"
#endif
