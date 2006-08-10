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

#ifndef QCUPS_P_H
#define QCUPS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include "QtCore/qstring.h"
#include "QtCore/qstringlist.h"
#include "QtGui/qprinter.h"

#ifndef QT_NO_PRINTER

#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
#include <QtCore/qlibrary.h>
#include <cups/cups.h>

typedef int (*CupsGetDests)(cups_dest_t **dests);
typedef const char* (*CupsGetPPD)(const char *printer);
typedef int (*CupsMarkOptions)(ppd_file_t *ppd, int num_options, cups_option_t *options);
typedef ppd_file_t* (*PPDOpenFile)(const char *filename);
typedef void (*PPDMarkDefaults)(ppd_file_t *ppd);
typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);
typedef void (*PPDClose)(ppd_file_t *ppd);
typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);
typedef void (*CupsFreeOptions)(int num_options, cups_option_t *options);
typedef void (*CupsSetDests)(int num_dests, cups_dest_t *dests);
typedef int (*CupsAddOption)(const char *name, const char *value, int num_options, cups_option_t **options);

class QCUPSSupport
{
public:
    QCUPSSupport();
    ~QCUPSSupport();

    bool isAvailable() const;

    int availablePrintersCount() const;
    const cups_dest_t* availablePrinters() const;
    int currentPrinterIndex() const;
    const ppd_file_t* setCurrentPrinter(int index);

    const ppd_file_t* currentPPD() const;
    const ppd_option_t* ppdOption(const char *key) const;

//     const cups_option_t* printerOption(const QString &key) const;
    const ppd_option_t* pageSizes() const;

    int markOption(const char* name, const char* value);
    void saveOptions(QList<const ppd_option_t*> options, QList<const char*> markedOptions);

    QRect paperRect() const;
    QRect pageRect() const;

    QStringList options() const;

private:
    void collectMarkedOptions(QStringList& list, const ppd_group_t* group = 0) const;
    void collectMarkedOptionsHelper(QStringList& list, const ppd_group_t* group) const;

    QLibrary cupsLib;

    CupsGetDests _cupsGetDests;
    CupsGetPPD _cupsGetPPD;
    PPDOpenFile _ppdOpenFile;
    PPDMarkDefaults _ppdMarkDefaults;
    PPDClose _ppdClose;
    CupsMarkOptions _cupsMarkOptions;
    PPDMarkOption _ppdMarkOption;
    CupsFreeOptions _cupsFreeOptions;
    CupsSetDests _cupsSetDests;
    CupsAddOption _cupsAddOption;

    int prnCount;
    cups_dest_t *printers;
    const ppd_option_t* page_sizes;
    int currPrinterIndex;
    ppd_file_t *currPPD;
};

#endif // !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)

#endif

#endif
