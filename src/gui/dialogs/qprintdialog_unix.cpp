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

#include "qprintdialog_unix.h"

#ifndef QT_NO_PRINTDIALOG

#include <private/qabstractprintdialog_p.h>

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

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
# undef connect
#endif

#endif // QT_NO_NIS

// UNIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

#include <ctype.h>
#include <stdlib.h>

class QPrintDialogUnixButtonGroup : public QObject
{
    Q_OBJECT
public:
    QPrintDialogUnixButtonGroup(QObject *parent);

    inline void hide(){}
    void insert(QAbstractButton *, int id = -1);
    void setButton(int id);
signals:
    void clicked(int);
private:
    QSignalMapper mapper;
    QButtonGroup group;
};


QPrintDialogUnixButtonGroup::QPrintDialogUnixButtonGroup(QObject *parent)
    :QObject(parent)
{
    connect(&mapper, SIGNAL(mapped(int)), this, SIGNAL(clicked(int)));
}

void QPrintDialogUnixButtonGroup::setButton(int id)
{
    if (QAbstractButton *button = static_cast<QAbstractButton*>(mapper.mapping(id)))
        button->click();
}

void QPrintDialogUnixButtonGroup::insert(QAbstractButton *button, int id)
{
    group.addButton(button);
    mapper.setMapping(button, id);
    connect(button, SIGNAL(clicked()), &mapper, SLOT(map()));
}

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

    int rowCount() const;
    int columnCount() const;
    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;

    QList<QPrinterDescription> lst;
};

QPrinterModel::QPrinterModel(const QList<QPrinterDescription> &printers, QObject *parent)
    : QAbstractTableModel(parent)
{
    lst = printers;
}

int QPrinterModel::rowCount() const
{
    return lst.count();
}

int QPrinterModel::columnCount() const
{
    return 3;
}

QVariant QPrinterModel::data(const QModelIndex &index, int) const
{
    if (index.type() == QModelIndex::HorizontalHeader) {
        const char *name = 0;
        switch(index.column()) {
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
    if (index.isValid() && index.row() < (int)lst.count()) {
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



class QPrintDialogUnixPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialogUnix)
public:
    QPrintDialogUnixButtonGroup *printerOrFile;

    bool outputToFile;
    QList<QPrinterDescription> printers;
    QPrinterModel *model;
    QTreeView *view;

    QLineEdit *fileName;
    QPushButton *browse, *ok;

    QPrintDialogUnixButtonGroup *printRange;
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

    QPrintDialogUnixButtonGroup *pageOrder;
    QPrinter::PageOrder pageOrder2;

    QPrintDialogUnixButtonGroup *colorMode;
    QPrinter::ColorMode colorMode2;

    QSpinBox *copies;
    int numCopies;

    QBoxLayout *customLayout;

    QPrinter::PageSize indexToPageSize[QPrinter::NPageSize];

    void init();
};


static void isc(QPrintDialogUnixPrivate *d, const QString & text,
                 QPrinter::PageSize ps);



static void perhapsAddPrinter(QList<QPrinterDescription> *printers, const QString &name,
                               QString host, QString comment,
                               QStringList aliases = QStringList())
{
    for (int i = 0; i < printers->size(); ++i)
        if (printers->at(i).samePrinter(name))
            return;

    if (host.isEmpty())
        host = QPrintDialogUnix::tr("locally connected");
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
            printerComment = QPrintDialogUnix::tr("Aliases: %1")
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
    if (!printcap.open(IO_ReadOnly))
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
                         printer.fileName().ascii());
            QFile configuration(tmp);
            char *line = new char[1025];
            QString remote(QLatin1String("Remote:"));
            QString contentType(QLatin1String("Content types:"));
            QString printerHost;
            bool canPrintPostscript = false;
            if (configuration.open(IO_ReadOnly)) {
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
    if (!pc.open(IO_ReadOnly)) {
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
                        qstrdup(printerDesc.mid(i, j-i).ascii());
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
                    printerComment = QPrintDialogUnix::tr("Aliases: %1")
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

                while (line[k] != '=' && line[k] != ']' &&
                        line[k] != '\0')
                    k++;
                if (line[k] == '=') {
                    k++;
                    skipSpaces();
                    action = tolower(line[k]);
                    while (!isspace((uchar) line[k]) &&
                            line[k] != ']')
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
    if (!nc.open(IO_ReadOnly))
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
                               QPrintDialogUnix::tr("unknown"),
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
        if (!configFile.open(IO_ReadOnly))
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
    if (!qconfig.open(IO_ReadOnly))
        return;

    QTextStream ts(&qconfig);
    QString line;

    QString stanzaName; // either a queue or a device name
    bool up = true; // queue up?  default true, can be false
    QString remoteHost; // null if local
    QString deviceName; // null if remote

    QRegExp newStanza(QLatin1String("^[0-z][0-z]*:$"));

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

        if (indented && line.contains('=')) { // line in stanza

            int i = line.indexOf('=');
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
                    (!indented &&
                      line.contains(newStanza))) {
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
                               QPrintDialogUnix::tr("Unknown Location"), QString());
            if (d[n].is_default && !defaultPrinter)
                defaultPrinter = qstrdup(d[n].instance);
            n++;
        }
    }
    return defaultPrinter;
}
#endif

/*!
  \class QPrintDialogUnix qprintdialog.h

  \brief The QPrintDialogUnix class provides a dialog for specifying
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
  function getPrinterSetup().  You can also subclass the QPrintDialogUnix
  and add some custom buttons with addButton() to extend the
  functionality of the print dialog.

  The printer dialog in Motif style:

  \img qprintdlg-m.png
*/


/*!
    Constructs a new modal printer dialog for the given \a printer
    with the given \a parent.
*/

#define d d_func()
#define q q_func()

QPrintDialogUnix::QPrintDialogUnix(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogUnixPrivate), printer, parent)
{
    d->init();
}


/*! Destroys the object and frees any allocated resources.  Does not
  delete the associated QPrinter object.
*/

QPrintDialogUnix::~QPrintDialogUnix()
{
}


QGroupBox *QPrintDialogUnix::setupPrinterSettings()
{
    QGroupBox *g = new QGroupBox(tr("Printer settings"), this);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    d->colorMode = new QPrintDialogUnixButtonGroup(this);
    d->colorMode->hide();
    connect(d->colorMode, SIGNAL(clicked(int)),
            this, SLOT(colorModeSelected(int)));

    QRadioButton *rb;
    rb = new QRadioButton(tr("Print in color if available"), g);
    d->colorMode->insert(rb, QPrinter::Color);
    rb->setChecked(true);
    tll->addWidget(rb);

    rb = new QRadioButton(tr("Print in grayscale"), g);
    d->colorMode->insert(rb, QPrinter::GrayScale);
    tll->addWidget(rb);

    return g;
}

QGroupBox *QPrintDialogUnix::setupDestination()
{
    QGroupBox *g = new QGroupBox(tr("Print destination"), this);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    d->printerOrFile = new QPrintDialogUnixButtonGroup(this);
    d->printerOrFile->hide();
    connect(d->printerOrFile, SIGNAL(clicked(int)),
             this, SLOT(printerOrFileSelected(int)));

    // printer radio button, list
    QRadioButton *rb = new QRadioButton(tr("Print to printer:"), g);
    tll->addWidget(rb);
    d->printerOrFile->insert(rb, 0);
    rb->setChecked(true);
    d->outputToFile = false;

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz, 3);
    horiz->addSpacing(19);

    char *etcLpDefault = 0;

#ifndef QT_NO_CUPS
    etcLpDefault = parseCupsOutput(&d->printers);
#endif
    if (d->printers.size() == 0) {
        // we only use other schemes when cups fails.

        parsePrintcap(&d->printers, QLatin1String("/etc/printcap"));
        parseEtcLpMember(&d->printers);
        parseSpoolInterface(&d->printers);
        parseQconfig(&d->printers);

        QFileInfo f;
        f.setFile(QLatin1String("/etc/lp/printers"));
        if (f.isDir()) {
            parseEtcLpPrinters(&d->printers);
            QFile def(QLatin1String("/etc/lp/default"));
            if (def.open(IO_ReadOnly)) {
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
            def = parseNsswitchConf(&d->printers);
        } else {
            f.setFile(QLatin1String("/etc/printers.conf"));
            if (f.isFile())
                def = parsePrintersConf(&d->printers);
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
        const char *t = getenv("PRINTER");
        if (!t || !*t)
            t = getenv("LPDEST");
        dollarPrinter = QLatin1String(t);
        if (!dollarPrinter.isEmpty())
            perhapsAddPrinter(&d->printers, dollarPrinter,
                               QPrintDialogUnix::tr("unknown"),
                              QLatin1String(""));
    }


    d->model = new QPrinterModel(d->printers, this);
    d->view = new QTreeView(g);
    d->view->setModel(d->model);
    d->view->setRootIsDecorated(false);
    d->view->header()->setResizeMode(QHeaderView::Stretch, 2);

    // bang the best default into the listview
    int quality = 0;
    int best = 0;
    for (int i = 0; i < d->printers.size(); ++i) {
        QRegExp ps(QLatin1String("[^a-z]ps(?:[^a-z]|$)"));
        QRegExp lp(QLatin1String("[^a-z]lp(?:[^a-z]|$)"));

        QString name = d->printers.at(i).name;
        QString comment = d->printers.at(i).comment;
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
    d->view->setCurrentItem(static_cast<QAbstractItemModel*>(d->model)->index(best, 0));

    if (etcLpDefault)                 // Avoid purify complaint
        delete[] etcLpDefault;

//     int h = fontMetrics().height();
//     if (d->printers.size())
//         h = d->view->itemViewportRect(d->model->index(0, 0)).height();
//     d->view->setMinimumSize(d->view->sizeHint().width(),
//                                  d->printers->header()->height() +
//                                  3 * h);
    horiz->addWidget(d->view, 3);

    // file radio button, edit/browse
    d->printToFileButton = new QRadioButton(tr("Print to file:"), g);
    tll->addWidget(d->printToFileButton);
    d->printerOrFile->insert(d->printToFileButton, 1);

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);
    horiz->addSpacing(19);

    d->fileName = new QLineEdit(g, "file name");
    connect(d->fileName, SIGNAL(textChanged(QString)),
             this, SLOT(fileNameEditChanged(QString)));
    horiz->addWidget(d->fileName, 1);
    d->browse = new QPushButton(tr("Browse..."), g);
    d->browse->setAutoDefault(false);
#ifdef QT_NO_FILEDIALOG
    d->browse->setEnabled(false);
#endif
    connect(d->browse, SIGNAL(clicked()),
             this, SLOT(browseClicked()));
    horiz->addWidget(d->browse);

    d->fileName->setEnabled(false);
    d->browse->setEnabled(false);

    return g;
}


QGroupBox *QPrintDialogUnix::setupOptions()
{
    QGroupBox *g = new QGroupBox(tr("Options"), this);

    QBoxLayout *lay = new QBoxLayout(QBoxLayout::LeftToRight, g);
    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, lay);

    d->printRange = new QPrintDialogUnixButtonGroup(this);
    d->printRange->hide();
    connect(d->printRange, SIGNAL(clicked(int)), this, SLOT(printRangeSelected(int)));

    d->pageOrder = new QPrintDialogUnixButtonGroup(this);
    d->pageOrder->hide();
    connect(d->pageOrder, SIGNAL(clicked(int)), this, SLOT(pageOrderSelected(int)));

    d->printAllButton = new QRadioButton(tr("Print all"), g);
    d->printRange->insert(d->printAllButton, 0);
    tll->addWidget(d->printAllButton);

    d->printSelectionButton = new QRadioButton(tr("Print selection"), g);
    d->printRange->insert(d->printSelectionButton, 1);
    tll->addWidget(d->printSelectionButton);

    d->printRangeButton = new QRadioButton(tr("Print range"), g);
    d->printRange->insert(d->printRangeButton, 2);
    tll->addWidget(d->printRangeButton);

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    d->firstPageLabel = new QLabel(tr("From page:"), g);
    horiz->addSpacing(19);
    horiz->addWidget(d->firstPageLabel);

    d->firstPage = new QSpinBox(g);
    d->firstPage->setRange(1, 9999);
    d->firstPage->setValue(1);
    horiz->addWidget(d->firstPage, 1);
    connect(d->firstPage, SIGNAL(valueChanged(int)),
             this, SLOT(setFirstPage(int)));

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    d->lastPageLabel = new QLabel(tr("To page:"), g);
    horiz->addSpacing(19);
    horiz->addWidget(d->lastPageLabel);

    d->lastPage = new QSpinBox(g);
    d->lastPage->setRange(1, 9999);
    d->lastPage->setValue(9999);
    horiz->addWidget(d->lastPage, 1);
    connect(d->lastPage, SIGNAL(valueChanged(int)),
             this, SLOT(setLastPage(int)));

    lay->addSpacing(25);
    tll = new QBoxLayout(QBoxLayout::Down, lay);

    // print order
    QRadioButton *rb = new QRadioButton(tr("Print first page first"), g);
    tll->addWidget(rb);
    d->pageOrder->insert(rb, QPrinter::FirstPageFirst);
    rb->setChecked(true);

    rb = new QRadioButton(tr("Print last page first"), g);
    tll->addWidget(rb);
    d->pageOrder->insert(rb, QPrinter::LastPageFirst);

    tll->addStretch();

    // copies

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    QLabel *l = new QLabel(tr("Number of copies:"), g);
    horiz->addWidget(l);

    d->copies = new QSpinBox(g);
    d->copies->setRange(1,99);
    d->copies->setValue(1);
    horiz->addWidget(d->copies, 1);
    connect(d->copies, SIGNAL(valueChanged(int)),
             this, SLOT(setNumCopies(int)));

    QSize s = d->firstPageLabel->sizeHint()
              .expandedTo(d->lastPageLabel->sizeHint())
              .expandedTo(l->sizeHint());
    d->firstPageLabel->setMinimumSize(s);
    d->lastPageLabel->setMinimumSize(s);
    l->setMinimumSize(s.width() + 19, s.height());

    return g;
}


void isc(QPrintDialogUnixPrivate *ptr, const QString & text, QPrinter::PageSize ps)
{
    if (ptr && !text.isEmpty() && ps < QPrinter::NPageSize) {
        ptr->sizeCombo->insertItem(text, -1);
        int index = ptr->sizeCombo->count()-1;
        if (index >= 0 && index < QPrinter::NPageSize)
            ptr->indexToPageSize[index] = ps;
    }
}

QGroupBox *QPrintDialogUnix::setupPaper()
{
    QGroupBox *g = new QGroupBox(tr("Paper format"), this);

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, g);
    d->pageSize = QPrinter::A4;

    // page orientation
    d->orientationCombo = new QComboBox(g);
    d->orientationCombo->insertItem(tr("Portrait"), -1);
    d->orientationCombo->insertItem(tr("Landscape"), -1);
    tll->addWidget(d->orientationCombo);

    d->orientation = QPrinter::Portrait;

    connect(d->orientationCombo, SIGNAL(activated(int)),
             this, SLOT(orientSelected(int)));

    // paper size
    d->sizeCombo = new QComboBox(g);
    tll->addWidget(d->sizeCombo);

    int n;
    for(n=0; n<QPrinter::NPageSize; n++)
        d->indexToPageSize[n] = QPrinter::A4;

    isc(d, tr("A0 (841 x 1189 mm)"), QPrinter::A0);
    isc(d, tr("A1 (594 x 841 mm)"), QPrinter::A1);
    isc(d, tr("A2 (420 x 594 mm)"), QPrinter::A2);
    isc(d, tr("A3 (297 x 420 mm)"), QPrinter::A3);
    isc(d, tr("A4 (210x297 mm, 8.26x11.7 inches)"), QPrinter::A4);
    isc(d, tr("A5 (148 x 210 mm)"), QPrinter::A5);
    isc(d, tr("A6 (105 x 148 mm)"), QPrinter::A6);
    isc(d, tr("A7 (74 x 105 mm)"), QPrinter::A7);
    isc(d, tr("A8 (52 x 74 mm)"), QPrinter::A8);
    isc(d, tr("A9 (37 x 52 mm)"), QPrinter::A9);
    isc(d, tr("B0 (1000 x 1414 mm)"), QPrinter::B0);
    isc(d, tr("B1 (707 x 1000 mm)"), QPrinter::B1);
    isc(d, tr("B2 (500 x 707 mm)"), QPrinter::B2);
    isc(d, tr("B3 (353 x 500 mm)"), QPrinter::B3);
    isc(d, tr("B4 (250 x 353 mm)"), QPrinter::B4);
    isc(d, tr("B5 (176 x 250 mm, 6.93x9.84 inches)"), QPrinter::B5);
    isc(d, tr("B6 (125 x 176 mm)"), QPrinter::B6);
    isc(d, tr("B7 (88 x 125 mm)"), QPrinter::B7);
    isc(d, tr("B8 (62 x 88 mm)"), QPrinter::B8);
    isc(d, tr("B9 (44 x 62 mm)"), QPrinter::B9);
    isc(d, tr("B10 (31 x 44 mm)"), QPrinter::B10);
    isc(d, tr("C5E (163 x 229 mm)"), QPrinter::C5E);
    isc(d, tr("DLE (110 x 220 mm)"), QPrinter::DLE);
    isc(d, tr("Executive (7.5x10 inches, 191x254 mm)"), QPrinter::Executive);
    isc(d, tr("Folio (210 x 330 mm)"), QPrinter::Folio);
    isc(d, tr("Ledger (432 x 279 mm)"), QPrinter::Ledger);
    isc(d, tr("Legal (8.5x14 inches, 216x356 mm)"), QPrinter::Legal);
    isc(d, tr("Letter (8.5x11 inches, 216x279 mm)"), QPrinter::Letter);
    isc(d, tr("Tabloid (279 x 432 mm)"), QPrinter::Tabloid);
    isc(d, tr("US Common #10 Envelope (105 x 241 mm)"), QPrinter::Comm10E);

    connect(d->sizeCombo, SIGNAL(activated(int)),
             this, SLOT(paperSizeSelected(int)));

    return g;
}


void QPrintDialogUnix::printerOrFileSelected(int id)
{
    d->outputToFile = id ? true : false;
    if (d->outputToFile) {
        d->ok->setEnabled(true);
        fileNameEditChanged(d->fileName->text());
        if (!d->fileName->isModified() && d->fileName->text().isEmpty()) {
            QString home = QLatin1String(::getenv("HOME"));
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
            d->fileName->setText(cur);
            d->fileName->setCursorPosition(cur.length());
            d->fileName->selectAll();
        }
        d->browse->setEnabled(true);
        d->fileName->setEnabled(true);
        d->fileName->setFocus();
        d->view->setEnabled(false);
    } else {
        d->ok->setEnabled(d->printers.count() != 0);
        d->view->setEnabled(true);
        if (d->fileName->hasFocus() || d->browse->hasFocus())
            d->view->setFocus();
        d->browse->setEnabled(false);
        d->fileName->setEnabled(false);
    }
}


void QPrintDialogUnix::landscapeSelected(int id)
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialogUnix::paperSizeSelected(int id)
{
    if (id < QPrinter::NPageSize)
        d->pageSize = QPrinter::PageSize(d->indexToPageSize[id]);
}


void QPrintDialogUnix::orientSelected(int id)
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialogUnix::pageOrderSelected(int id)
{
    d->pageOrder2 = (QPrinter::PageOrder)id;
}


void QPrintDialogUnix::setNumCopies(int copies)
{
    d->numCopies = copies;
}


void QPrintDialogUnix::browseClicked()
{
#if 0 // ### Fix before 4.0
#ifndef QT_NO_FILEDIALOG
    QString fn = QFileDialog::getSaveFileName(d->fileName->text(), tr("PostScript Files (*.ps);;All Files (*)"), this);
    if (!fn.isNull())
        d->fileName->setText(fn);
#endif
#endif
}


void QPrintDialogUnix::okClicked()
{
    d->lastPage->interpretText();
    d->firstPage->interpretText();
    d->copies->interpretText();
    if (d->outputToFile) {
        d->printer->setOutputToFile(true);
        d->printer->setOutputFileName(d->fileName->text());
    } else {
        d->printer->setOutputToFile(false);
        QModelIndex current = d->view->currentItem();
        if (current.isValid())
            d->printer->setPrinterName(d->printers.at(current.row()).name);
    }

    d->printer->setOrientation(d->orientation);
    d->printer->setPageSize(d->pageSize);
    d->printer->setPageOrder(d->pageOrder2);
    d->printer->setColorMode(d->colorMode2);
    d->printer->setNumCopies(d->numCopies);
    if (d->printAllButton->isChecked()) {
        setPrintRange(AllPages);
        setFromTo(minPage(), maxPage());
    } else {
        if (d->printSelectionButton->isChecked()) {
            setPrintRange(Selection);
            setFromTo(0, 0);
        } else {
            setPrintRange(PageRange);
            setFromTo(d->firstPage->value(), d->lastPage->value());
        }
    }
    accept();
}


void QPrintDialogUnix::printRangeSelected(int id)
{
    bool enable = id == 2 ? true : false;
    d->firstPage->setEnabled(enable);
    d->lastPage->setEnabled(enable);
    d->firstPageLabel->setEnabled(enable);
    d->lastPageLabel->setEnabled(enable);
}


void QPrintDialogUnix::setFirstPage(int fp)
{
    if (d->printer) {
        d->lastPage->setMinimum(fp);
        d->lastPage->setMaximum(qMax(fp, maxPage()));
    }
}


void QPrintDialogUnix::setLastPage(int lp)
{
    if (d->printer) {
        d->firstPage->setMinimum(qMin(lp, minPage()));
        d->firstPage->setMaximum(lp);
    }
}


/*!
  Sets this dialog to configure printer \a p, or no printer if \a p
  is null. If \a pickUpSettings is true, the dialog reads most of
  its settings from \a p. If \a pickUpSettings is false (the
  default) the dialog keeps its old settings.
*/

void QPrintDialogUnix::setPrinter(QPrinter *p, bool pickUpSettings)
{
    d->printer = p;

    if (p && pickUpSettings) {
        // top to botton in the old dialog.
        // printer or file
        d->printerOrFile->setButton(p->outputToFile());
        printerOrFileSelected(p->outputToFile());

        // printer name
        if (!p->printerName().isEmpty()) {
            for (int i = 0; i < d->printers.size(); ++i) {
                if (d->printers.at(i).name == p->printerName()) {
                    // ###############
//                    d->printers->setSelected(i, true);
                    d->ok->setEnabled(true);
                } else if (d->fileName->text().isEmpty()) {
                    d->ok->setEnabled(d->model->rowCount() != 0);
                }
            }
        }

        // print command does not exist any more

        // file name
        d->printToFileButton->setEnabled(isOptionEnabled(PrintToFile));
        d->fileName->setText(p->outputFileName());

        // orientation
        d->orientationCombo->setCurrentItem((int)p->orientation());
        orientSelected(p->orientation());

        // page size
        int n = 0;
        while (n < QPrinter::NPageSize &&
                d->indexToPageSize[n] != p->pageSize())
            n++;
        d->sizeCombo->setCurrentItem(n);
        paperSizeSelected(n);

        // New stuff (Options)

        // page order
        d->pageOrder->setButton((int)p->pageOrder());
        pageOrderSelected(p->pageOrder());

        // color mode
        d->colorMode->setButton((int)p->colorMode());
        colorModeSelected(p->colorMode());

        // number of copies
        d->copies->setValue(p->numCopies());
        setNumCopies(p->numCopies());
    }

    if(p) {
        d->printAllButton->setEnabled(true);
        d->printSelectionButton->setEnabled(isOptionEnabled(PrintSelection));
        d->printRangeButton->setEnabled(isOptionEnabled(PrintPageRange));

        switch (printRange()) {
        case AllPages:
            d->printAllButton->click();
            break;
        case Selection:
            d->printSelectionButton->click();
            break;
        case PageRange:
            d->printRangeButton->click();
            break;
        }
    }

    if (p && maxPage()) {
        d->firstPage->setMinimum(minPage());
        d->firstPage->setMaximum(maxPage());
        d->lastPage->setMinimum(minPage());
        d->lastPage->setMaximum(maxPage());
        if (fromPage() || toPage()) {
            setFirstPage(fromPage());
            setLastPage(toPage());
            d->firstPage->setValue(fromPage());
            d->lastPage->setValue(toPage());
        }
    }
}


/*!  Returns a pointer to the printer this dialog configures, or 0 if
  this dialog does not operate on any printer. */
QPrinter *QPrintDialogUnix::printer() const
{
    return d->printer;
}


void QPrintDialogUnix::colorModeSelected(int id)
{
    d->colorMode2 = (QPrinter::ColorMode)id;
}


/*!
  Adds the button \a but to the layout of the print dialog. The added
  buttons are arranged from the left to the right below the
  last groupbox of the printdialog.
*/
void QPrintDialogUnix::addButton(QPushButton *but)
{
    d->customLayout->addWidget(but);
}

void QPrintDialogUnix::fileNameEditChanged(const QString &text)
{
    if (d->fileName->isEnabled())
        d->ok->setEnabled(!text.isEmpty());
}

int QPrintDialogUnix::exec()
{
    return QDialog::exec();
}

void QPrintDialogUnixPrivate::init()
{
    numCopies = 1;

    QBoxLayout *tll = new QBoxLayout(QBoxLayout::Down, q);

    // destination
    QGroupBox *g;
    g = q->setupDestination();
    tll->addWidget(g, 1);

    // printer and paper settings
    QBoxLayout *lay = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(lay);

    g = q->setupPrinterSettings();
    lay->addWidget(g, 1);

    g = q->setupPaper();
    lay->addWidget(g);

    // options
    g = q->setupOptions();
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
        bool(q->style().styleHint(QStyle::SH_PrintDialog_RightAlignButtons, q));

    if (rightalign)
        horiz->addStretch(1);

    ok = new QPushButton(q->tr("OK"), q);
    ok->setDefault(true);
    horiz->addWidget(ok);
    if (! rightalign)
        horiz->addStretch(1);

    QPushButton *cancel = new QPushButton(q->tr("Cancel"), q);
    horiz->addWidget(cancel);

    QObject::connect(ok, SIGNAL(clicked()), q, SLOT(okClicked()));
    QObject::connect(cancel, SIGNAL(clicked()), q,  SLOT(reject()));

    QSize ms(q->minimumSize());
    QSize ss(QApplication::desktop()->screenGeometry(q->pos()).size());
    if (ms.height() < 512 && ss.height() >= 600)
        ms.setHeight(512);
    else if (ms.height() < 460 && ss.height() >= 480)
        ms.setHeight(460);
    q->resize(ms);

    q->setPrinter(printer, true);
    view->setFocus();
}

#include "qprintdialog_unix.moc"
#endif
