/****************************************************************************
**
** Implementation of internal print dialog (X11) used by QPrinter::select().
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include "qlistview.h"
#include "qlayout.h"
#include "qbuttongroup.h"
#include "qradiobutton.h"
#include "qspinbox.h"
#include "qapplication.h"
#include "qheader.h"
#include "qstyle.h"
#include "qstring.h"
#include "qregexp.h"
#if !defined(QT_NO_CUPS)
#include "qlibrary.h"
#endif
#include "qgroupbox.h"
#include "qsignalmapper.h"
#include "qmap.h"

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

class QPrintDialogUnixSpinBox : public QSpinBox
{
public:
    QPrintDialogUnixSpinBox(int min, int max, int steps, QWidget *parent, const char *name)
        : QSpinBox(min, max, steps, parent, name)
    {}

    void interpretText()
    {
        QSpinBox::interpretText();
    }
};

enum { Success = 's', Unavail = 'u', NotFound = 'n', TryAgain = 't' };
enum { Continue = 'c', Return = 'r' };

class QPrintDialogUnixPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialogUnix)
public:
    QPrintDialogUnixButtonGroup *printerOrFile;

    bool outputToFile;
    QListView *printers;
    QLineEdit *fileName;
    QPushButton *browse, *ok;

    QPrintDialogUnixButtonGroup *printRange;
    QLabel *firstPageLabel;
    QPrintDialogUnixSpinBox *firstPage;
    QLabel *lastPageLabel;
    QPrintDialogUnixSpinBox *lastPage;
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

    QPrintDialogUnixSpinBox *copies;
    int numCopies;

    QBoxLayout *customLayout;

    QPrinter::PageSize indexToPageSize[QPrinter::NPageSize];

    void init();
};


typedef void (*Q_PrintDialogHook)(QListView *);
static Q_PrintDialogHook addPrinterHook = 0;

void qt_set_printdialog_hook(Q_PrintDialogHook hook)
{
    addPrinterHook = hook;
}

static void isc(QPrintDialogUnixPrivate *d, const QString & text,
                 QPrinter::PageSize ps);

class QPrinterListViewItem : public QListViewItem
{
public:
    QPrinterListViewItem(QListView *printers, const QString& name,
                          const QString& host, const QString& comment,
                          const QStringList& aliases)
        : QListViewItem(printers, name, host, comment), ali(aliases) { }

    bool samePrinter(const QString& name) {
        return text(0) == name || ali.contains(name);
    }

    QStringList ali;
};

static void perhapsAddPrinter(QListView *printers, const QString &name,
                               QString host, QString comment,
                               QStringList aliases = QStringList())
{
    const QListViewItem *i = printers->firstChild();
    while (i && !((QPrinterListViewItem *) i)->samePrinter(name))
        i = i->nextSibling();
    if (i)
        return;
    if (host.isEmpty())
        host = QPrintDialogUnix::tr("locally connected");
    (void)new QPrinterListViewItem(printers,
                                    name.simplified(),
                                    host.simplified(),
                                    comment.simplified(), aliases);
}

static void parsePrinterDesc(QString printerDesc, QListView *printers)
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
        i = printerDesc.indexOf(QRegExp(QString::fromLatin1(": *all *=")));
        if (i >= 0)
            printerName = "";
        // look for signs of this being a remote printer
        i = printerDesc.indexOf(QRegExp(QString::fromLatin1(": *rm *=")));
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

static int parsePrintcap(QListView *printers, const QString& fileName)
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
            line.truncate(line.length() - 1);
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
static void parseEtcLpPrinters(QListView *printers)
{
    QDir lp(QString::fromLatin1("/etc/lp/printers"));
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
            QString remote(QString::fromLatin1("Remote:"));
            QString contentType(QString::fromLatin1("Content types:"));
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
                                       printerHost, QString::fromLatin1(""));
            }
            delete[] line;
        }
    }
}


// solaris 2.6
static char *parsePrintersConf(QListView *printers, bool *found = 0)
{
    QFile pc(QString::fromLatin1("/etc/printers.conf"));
    if (!pc.open(IO_ReadOnly)) {
        if (*found)
            *found = false;
        return 0;
    }
    if (*found)
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
                if (printerName == QString::fromLatin1("_default")) {
                    i = printerDesc.indexOf(
                        QRegExp(QString::fromLatin1(": *use *=")));
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
                } else if (printerName == QString::fromLatin1("_all")) {
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
                    QRegExp(QString::fromLatin1(": *bsdaddr *=")));
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
                                QString::fromLatin1("Remote name: ");
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

static int foreach(int /*status */, char * /*key */, int /*keyLen */,
                    char *val, int valLen, char *data)
{
    parsePrinterDesc(QString::fromLatin1(val, valLen), (QListView *) data);
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif

static int retrieveNisPrinters(QListView *printers)
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
            (WildCast &) cb.foreach = (WildCast) foreach;
            cb.data = (char *) printers;
            err = _ypAll(domain, printersConfByname, &cb);
        }
        if (!err)
            return Success;
    }
    return Unavail;
}

#endif // QT_NO_NIS

static char *parseNsswitchPrintersEntry(QListView *printers, char *line)
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
                        QDir::homeDirPath() + "/.printers");
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

static char *parseNsswitchConf(QListView *printers)
{
    QFile nc(QString::fromLatin1("/etc/nsswitch.conf"));
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
static void parseEtcLpMember(QListView *printers)
{
    QDir lp(QString::fromLatin1("/etc/lp/member"));
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
                               QString::fromLatin1(""));
    }
}

// IRIX 6.x
static void parseSpoolInterface(QListView *printers)
{
    QDir lp(QString::fromLatin1("/usr/spool/lp/interface"));
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

        QString nameKey(QString::fromLatin1("NAME="));
        QString typeKey(QString::fromLatin1("TYPE="));
        QString hostKey(QString::fromLatin1("HOSTNAME="));
        QString hostPrinterKey(QString::fromLatin1("HOSTPRINTER="));

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
        if (printerType.indexOf("postscript", 0, QString::CaseInsensitive) < 0)
            continue;

        int ii = 0;
        while ((ii = namePrinter.indexOf('"', ii)) >= 0)
            namePrinter.remove(ii, 1);

        if (hostName.isEmpty() || hostPrinter.isEmpty()) {
            perhapsAddPrinter(printers, printer.fileName(),
                               QString::fromLatin1(""), namePrinter);
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
static void parseQconfig(QListView *printers)
{
    QFile qconfig(QString::fromLatin1("/etc/qconfig"));
    if (!qconfig.open(IO_ReadOnly))
        return;

    QTextStream ts(&qconfig);
    QString line;

    QString stanzaName; // either a queue or a device name
    bool up = true; // queue up?  default true, can be false
    QString remoteHost; // null if local
    QString deviceName; // null if remote

    QRegExp newStanza(QString::fromLatin1("^[0-z][0-z]*:$"));

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
            if (variable == QString::fromLatin1("device"))
                deviceName = value;
            else if (variable == QString::fromLatin1("host"))
                remoteHost = value;
            else if (variable == QString::fromLatin1("up"))
                up = !(value.toLower() == QString::fromLatin1("false"));
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
            line.truncate(line.length()-1);
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

static char *parseCupsOutput(QListView *printers)
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

  \warning The use of this class is not recommended since it is not
  present on all platforms; use QPrinter::setup() instead.

  \omit

  (ingroup dialogs)

  THIS DOCUMENTATION IS Not Revised. It must be revised before
  becoming public API.

  It encompasses both the sort of details needed for doing a simple
  print-out and some print configuration setup.

  The easiest way to use the class is through the static
  function getPrinterSetup().  You can also subclass the QPrintDialogUnix
  and add some custom buttons with addButton() to extend the
  functionality of the print dialog.

  <img src="qprintdlg-m.png"><br clear=all>
  The printer dialog, on a large screen, in Motif style.
*/


/*! Constructs a new modal printer dialog that configures \a prn and is a
  child of \a parent named \a name.
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
    QGroupBox *g = new QGroupBox(tr("Printer settings"), this, "settings group box");

    QBoxLayout *tll = new QBoxLayout(g, QBoxLayout::Down);
    d->colorMode = new QPrintDialogUnixButtonGroup(this);
    d->colorMode->hide();
    connect(d->colorMode, SIGNAL(clicked(int)),
            this, SLOT(colorModeSelected(int)));

    QRadioButton *rb;
    rb = new QRadioButton(tr("Print in color if available"),
                           g, "color");
    d->colorMode->insert(rb, QPrinter::Color);
    rb->setChecked(true);
    tll->addWidget(rb);

    rb = new QRadioButton(tr("Print in grayscale"),
                           g, "graysacle");
    d->colorMode->insert(rb, QPrinter::GrayScale);
    tll->addWidget(rb);

    return g;
}

QGroupBox *QPrintDialogUnix::setupDestination()
{
    QGroupBox *g = new QGroupBox(tr("Print destination"),
                                   this, "destination group box");

    QBoxLayout *tll = new QBoxLayout(g, QBoxLayout::Down);

    d->printerOrFile = new QPrintDialogUnixButtonGroup(this);
    d->printerOrFile->hide();
    connect(d->printerOrFile, SIGNAL(clicked(int)),
             this, SLOT(printerOrFileSelected(int)));

    // printer radio button, list
    QRadioButton *rb = new QRadioButton(tr("Print to printer:"), g,
                                          "printer");
    tll->addWidget(rb);
    d->printerOrFile->insert(rb, 0);
    rb->setChecked(true);
    d->outputToFile = false;

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz, 3);
    horiz->addSpacing(19);

    d->printers = new QListView(g, "list of printers");
    d->printers->setAllColumnsShowFocus(true);
    d->printers->addColumn(tr("Printer"), 125);
    d->printers->addColumn(tr("Host"), 125);
    d->printers->addColumn(tr("Comment"), 150);

#if defined(Q_OS_UNIX)
    char *etcLpDefault = 0;

#ifndef QT_NO_CUPS
    etcLpDefault = parseCupsOutput(d->printers);
#endif
    if (d->printers->childCount() == 0) {
        // we only use other schemes when cups fails.

        parsePrintcap(d->printers, QString::fromLatin1("/etc/printcap"));
        parseEtcLpMember(d->printers);
        parseSpoolInterface(d->printers);
        parseQconfig(d->printers);
        if (addPrinterHook)
            (*addPrinterHook)(d->printers);

        QFileInfo f;
        f.setFile(QString::fromLatin1("/etc/lp/printers"));
        if (f.isDir()) {
            parseEtcLpPrinters(d->printers);
            QFile def(QString::fromLatin1("/etc/lp/default"));
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
        f.setFile(QString::fromLatin1("/etc/nsswitch.conf"));
        if (f.isFile()) {
            def = parseNsswitchConf(d->printers);
        } else {
            f.setFile(QString::fromLatin1("/etc/printers.conf"));
            if (f.isFile())
                def = parsePrintersConf(d->printers);
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
        dollarPrinter = QString::fromLatin1(t);
        if (!dollarPrinter.isEmpty())
            perhapsAddPrinter(d->printers, dollarPrinter,
                               QPrintDialogUnix::tr("unknown"),
                               QString::fromLatin1(""));
    }
    int quality = 0;

    // bang the best default into the listview
    const QListViewItem *lvi = d->printers->firstChild();
    d->printers->setCurrentItem((QListViewItem *)lvi);
    while (lvi) {
        QRegExp ps(QString::fromLatin1("[^a-z]ps(?:[^a-z]|$)"));
        QRegExp lp(QString::fromLatin1("[^a-z]lp(?:[^a-z]|$)"));

        if (quality < 4 && lvi->text(0) == dollarPrinter) {
            d->printers->setCurrentItem((QListViewItem *)lvi);
            quality = 4;
        } else if (quality < 3 && etcLpDefault &&
                    lvi->text(0) == QString::fromLatin1(etcLpDefault)) {
            d->printers->setCurrentItem((QListViewItem *)lvi);
            quality = 3;
        } else if (quality < 2 &&
                    (lvi->text(0) == QString::fromLatin1("ps") ||
                     ps.indexIn(lvi->text(2)) != -1)) {
            d->printers->setCurrentItem((QListViewItem *)lvi);
            quality = 2;
        } else if (quality < 1 &&
                    (lvi->text(0) == QString::fromLatin1("lp") ||
                     lp.indexIn(lvi->text(2)) > -1)) {
            d->printers->setCurrentItem((QListViewItem *)lvi);
            quality = 1;
        }
        lvi = lvi->nextSibling();
    }

    if (d->printers->currentItem())
        d->printers->setSelected(d->printers->currentItem(), true);

    if (etcLpDefault)                 // Avoid purify complaint
        delete[] etcLpDefault;
#endif

    int h = fontMetrics().height();
    if (d->printers->firstChild())
        h = d->printers->firstChild()->height();
    d->printers->setMinimumSize(d->printers->sizeHint().width(),
                                 d->printers->header()->height() +
                                 3 *h);
    horiz->addWidget(d->printers, 3);

    tll->addSpacing(6);

    // file radio button, edit/browse
    d->printToFileButton = new QRadioButton(tr("Print to file:"), g, "file");
    tll->addWidget(d->printToFileButton);
    d->printerOrFile->insert(d->printToFileButton, 1);

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);
    horiz->addSpacing(19);

    d->fileName = new QLineEdit(g, "file name");
    connect(d->fileName, SIGNAL(textChanged(QString)),
             this, SLOT(fileNameEditChanged(QString)));
    horiz->addWidget(d->fileName, 1);
    horiz->addSpacing(6);
    d->browse = new QPushButton(tr("Browse..."), g, "browse files");
    d->browse->setAutoDefault(false);
#ifdef QT_NO_FILEDIALOG
    d->browse->setEnabled(false);
#endif
    connect(d->browse, SIGNAL(clicked()),
             this, SLOT(browseClicked()));
    horiz->addWidget(d->browse);

    d->fileName->setEnabled(false);
    d->browse->setEnabled(false);

    tll->activate();

    return g;
}


QGroupBox *QPrintDialogUnix::setupOptions()
{
    QGroupBox *g = new QGroupBox(tr("Options"),
                                   this, "options group box");

    QBoxLayout *lay = new QBoxLayout(g, QBoxLayout::LeftToRight);
    QBoxLayout *tll = new QBoxLayout(lay, QBoxLayout::Down);

    d->printRange = new QPrintDialogUnixButtonGroup(this);
    d->printRange->hide();
    connect(d->printRange, SIGNAL(clicked(int)),
             this, SLOT(printRangeSelected(int)));

    d->pageOrder = new QPrintDialogUnixButtonGroup(this);
    d->pageOrder->hide();
    connect(d->pageOrder, SIGNAL(clicked(int)),
             this, SLOT(pageOrderSelected(int)));

    d->printAllButton = new QRadioButton(tr("Print all"), g, "print all");
    d->printRange->insert(d->printAllButton, 0);
    tll->addWidget(d->printAllButton);

    d->printSelectionButton = new QRadioButton(tr("Print selection"),
                                                g, "print selection");
    d->printRange->insert(d->printSelectionButton, 1);
    tll->addWidget(d->printSelectionButton);

    d->printRangeButton = new QRadioButton(tr("Print range"),
                                            g, "print range");
    d->printRange->insert(d->printRangeButton, 2);
    tll->addWidget(d->printRangeButton);

    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    d->firstPageLabel = new QLabel(tr("From page:"), g, "first page");
    horiz->addSpacing(19);
    horiz->addWidget(d->firstPageLabel);

    d->firstPage = new QPrintDialogUnixSpinBox(1, 9999, 1, g, "first page");
    d->firstPage->setValue(1);
    horiz->addWidget(d->firstPage, 1);
    connect(d->firstPage, SIGNAL(valueChanged(int)),
             this, SLOT(setFirstPage(int)));

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    d->lastPageLabel = new QLabel(tr("To page:"), g, "last page");
    horiz->addSpacing(19);
    horiz->addWidget(d->lastPageLabel);

    d->lastPage = new QPrintDialogUnixSpinBox(1, 9999, 1, g, "last page");
    d->lastPage->setValue(9999);
    horiz->addWidget(d->lastPage, 1);
    connect(d->lastPage, SIGNAL(valueChanged(int)),
             this, SLOT(setLastPage(int)));

    lay->addSpacing(25);
    tll = new QBoxLayout(lay, QBoxLayout::Down);

    // print order
    QRadioButton *rb = new QRadioButton(tr("Print first page first"),
                                          g, "first page first");
    tll->addWidget(rb);
    d->pageOrder->insert(rb, QPrinter::FirstPageFirst);
    rb->setChecked(true);

    rb = new QRadioButton(tr("Print last page first"),
                           g, "last page first");
    tll->addWidget(rb);
    d->pageOrder->insert(rb, QPrinter::LastPageFirst);

    tll->addStretch();

    // copies

    horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    QLabel *l = new QLabel(tr("Number of copies:"), g, "Number of copies");
    horiz->addWidget(l);

    d->copies = new QPrintDialogUnixSpinBox(1, 99, 1, g, "copies");
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

    tll->activate();

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
    QGroupBox *g = new QGroupBox(tr("Paper format"),
                                   this, "Paper format");

    QBoxLayout *tll = new QBoxLayout(g, QBoxLayout::Down, 12, 0);
    d->pageSize = QPrinter::A4;

    // page orientation
    d->orientationCombo = new QComboBox(false, g);
    d->orientationCombo->insertItem(tr("Portrait"), -1);
    d->orientationCombo->insertItem(tr("Landscape"), -1);
    tll->addWidget(d->orientationCombo);

    d->orientation = QPrinter::Portrait;

    tll->addSpacing(8);

    connect(d->orientationCombo, SIGNAL(activated(int)),
             this, SLOT(orientSelected(int)));

    // paper size
    d->sizeCombo = new QComboBox(false, g);
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
            QString home = QString::fromLatin1(::getenv("HOME"));
            QString cur = QDir::currentDirPath();
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
        d->printers->setEnabled(false);
    } else {
        d->ok->setEnabled(d->printers->childCount() != 0);
        d->printers->setEnabled(true);
        if (d->fileName->hasFocus() || d->browse->hasFocus())
            d->printers->setFocus();
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
        QListViewItem *l = d->printers->currentItem();
        if (l)
            d->printer->setPrinterName(l->text(0));
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
        if (p->printerName().count()) {
            QListViewItem *i = d->printers->firstChild();
            while (i && i->text(0) != p->printerName())
                i = i->nextSibling();
            if (i) {
                d->printers->setSelected(i, true);
                d->ok->setEnabled(true);
            } else if (d->fileName->text().isEmpty()) {
                d->ok->setEnabled(d->printers->childCount() != 0);
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

    QBoxLayout *tll = new QBoxLayout(q, QBoxLayout::Down, 12, 0);

    // destination
    QGroupBox *g;
    g = q->setupDestination();
    tll->addWidget(g, 1);

    tll->addSpacing(12);

    // printer and paper settings
    QBoxLayout *lay = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(lay);

    g = q->setupPrinterSettings();
    lay->addWidget(g, 1);

    lay->addSpacing(12);

    g = q->setupPaper();
    lay->addWidget(g);

    tll->addSpacing(12);

    // options
    g = q->setupOptions();
    tll->addWidget(g);
    tll->addSpacing(12);

    QBoxLayout *l = new QBoxLayout(QBoxLayout::LeftToRight);
    customLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(l);
    l->addLayout(customLayout);
    l->addStretch();
    tll->addSpacing(12);

    // buttons
    QBoxLayout *horiz = new QBoxLayout(QBoxLayout::LeftToRight);
    tll->addLayout(horiz);

    bool rightalign =
        bool(q->style().styleHint(QStyle::SH_PrintDialog_RightAlignButtons, q));

    if (rightalign)
        horiz->addStretch(1);

    ok = new QPushButton(q, "ok");
    ok->setText(q->tr("OK"));
    ok->setDefault(true);
    horiz->addWidget(ok);
    if (! rightalign)
        horiz->addStretch(1);
    horiz->addSpacing(6);

    QPushButton *cancel = new QPushButton(q, "cancel");
    cancel->setText(q->tr("Cancel"));
    horiz->addWidget(cancel);

    QSize s1 = ok->sizeHint();
    QSize s2 = cancel->sizeHint();
    s1 = QSize(qMax(s1.width(), s2.width()),
                qMax(s1.height(), s2.height()));

    ok->setFixedSize(s1);
    cancel->setFixedSize(s1);

    tll->activate();

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
    printers->setFocus();
}

#include "qprintdialog_unix.moc"
#endif
