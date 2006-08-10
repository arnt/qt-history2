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
#include <qdebug.h>
#include "qcups_p.h"

#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)

// ================ CUPS Support class ========================

QCUPSSupport::QCUPSSupport()
    : cupsLib(QLatin1String("cups"), 2),
    _cupsGetDests(0),
    _cupsGetPPD(0),
    _ppdOpenFile(0),
    _ppdMarkDefaults(0),
    _cupsFreeOptions(0),
    prnCount(0),
    printers(0),
    page_sizes(0),
    currPrinterIndex(0),
    currPPD(0)
{
    if (cupsLib.load()) {
        _cupsGetDests = (CupsGetDests) cupsLib.resolve("cupsGetDests");
        _cupsGetPPD = (CupsGetPPD) cupsLib.resolve("cupsGetPPD");
        _ppdOpenFile = (PPDOpenFile) cupsLib.resolve("ppdOpenFile");
        _ppdMarkDefaults = (PPDMarkDefaults) cupsLib.resolve("ppdMarkDefaults");
        _ppdClose = (PPDClose) cupsLib.resolve("ppdClose");
        _cupsMarkOptions = (CupsMarkOptions) cupsLib.resolve("cupsMarkOptions");
        _ppdMarkOption = (PPDMarkOption) cupsLib.resolve("ppdMarkOption");
        _cupsFreeOptions = (CupsFreeOptions) cupsLib.resolve("cupsFreeOptions");
        _cupsSetDests = (CupsSetDests) cupsLib.resolve("cupsSetDests");
        _cupsAddOption = (CupsAddOption) cupsLib.resolve("cupsAddOption");

        // getting all available printers
        if (isAvailable())
            prnCount = _cupsGetDests(&printers);

        for (int i = 0; i <  prnCount; ++i) {
            if (printers[i].is_default) {
                currPrinterIndex = i;
                setCurrentPrinter(i);
                break;
            }
        }
    }
}

QCUPSSupport::~QCUPSSupport()
{
     if (currPPD)
        _ppdClose(currPPD);
}

int QCUPSSupport::availablePrintersCount() const
{
    return prnCount;
}

const cups_dest_t* QCUPSSupport::availablePrinters() const
{
    return printers;
}

const ppd_file_t* QCUPSSupport::currentPPD() const
{
    return currPPD;
}

const ppd_file_t* QCUPSSupport::setCurrentPrinter(int index)
{
    Q_ASSERT(index >= 0 && index <= prnCount);
    if (index == prnCount)
        return 0;

    currPrinterIndex = index;

    if (currPPD)
        _ppdClose(currPPD);

    const char *ppdFile = _cupsGetPPD(printers[index].name);

    if (!ppdFile)
      return 0;

    currPPD = _ppdOpenFile(ppdFile);
    unlink(ppdFile);

    // marking default options
    _ppdMarkDefaults(currPPD);

    // marking options explicitely set
    _cupsMarkOptions(currPPD, printers[currPrinterIndex].num_options, printers[currPrinterIndex].options);

    // getting pointer to page sizes
    page_sizes = ppdOption("PageSize");

    return currPPD;
}

int QCUPSSupport::currentPrinterIndex() const
{
    return currPrinterIndex;
}

bool QCUPSSupport::isAvailable() const
{
    return cupsLib.isLoaded() &&
        _cupsGetDests &&
        _cupsGetPPD &&
        _ppdOpenFile &&
        _ppdMarkDefaults &&
        _ppdClose &&
        _cupsMarkOptions &&
        _ppdMarkOption &&
        _cupsFreeOptions &&
        _cupsSetDests &&
        _cupsAddOption;
}

const ppd_option_t* QCUPSSupport::ppdOption(const char *key) const
{
    for (int gr = 0; gr < currPPD->num_groups; ++gr) {
        for (int opt = 0; opt < currPPD->groups[gr].num_options; ++opt) {
            if (qstrcmp(currPPD->groups[gr].options[opt].keyword, key) == 0)
                return &currPPD->groups[gr].options[opt];
        }
    }
    return 0;
}

#if 0
const cups_option_t* QCUPSSupport::printerOption(const QString &key) const
{
    for (int i = 0; i < printers[currPrinterIndex].num_options; ++i) {
        if (printers[currPrinterIndex].options[i].name == key)
            return &printers[currPrinterIndex].options[i];
    }
    return 0;
}
#endif

const ppd_option_t* QCUPSSupport::pageSizes() const
{
    return page_sizes;
}

int QCUPSSupport::markOption(const char* name, const char* value)
{
    return _ppdMarkOption(currPPD, name, value);
}

void QCUPSSupport::saveOptions(QList<const ppd_option_t*> options, QList<const char*> markedOptions)
{
    int oldOptionCount = printers[currPrinterIndex].num_options;
    cups_option_t* oldOptions = printers[currPrinterIndex].options;

    int newOptionCount = 0;
    cups_option_t* newOptions = 0;

    // copying old options that are not on the new list
    for (int i = 0; i < oldOptionCount; ++i) {
        bool contains = false;
        for (int j = 0; j < options.count(); ++j) {
            if (qstrcmp(options.at(j)->keyword, oldOptions[i].name) == 0) {
                contains = true;
                break;
            }
        }

        if (!contains) {
            newOptionCount = _cupsAddOption(oldOptions[i].name, oldOptions[i].value, newOptionCount, &newOptions);
        }
    }

    // we can release old option list
     _cupsFreeOptions(oldOptionCount, oldOptions);

    // adding marked options
    for (int i = 0; i < markedOptions.count(); ++i) {
        const char* name = markedOptions.at(i);
        ++i;
        newOptionCount = _cupsAddOption(name, markedOptions.at(i), newOptionCount, &newOptions);
    }

    // placing the new option list
    printers[currPrinterIndex].num_options = newOptionCount;
    printers[currPrinterIndex].options = newOptions;

    // saving new default values
    _cupsSetDests(prnCount, printers);
}

QRect QCUPSSupport::paperRect() const
{
    for (int i = 0; i < currPPD->num_sizes; ++i) {
        if (currPPD->sizes[i].marked == 0)
            return QRect(0, 0, (int)currPPD->sizes[i].width, (int)currPPD->sizes[i].length);
    }
    return QRect();
}

QRect QCUPSSupport::pageRect() const
{
    for (int i = 0; i < currPPD->num_sizes; ++i) {
        if (currPPD->sizes[i].marked == 0)
            return QRect((int)currPPD->sizes[i].left,
                         (int)currPPD->sizes[i].right,
                         (int)(currPPD->sizes[i].right - currPPD->sizes[i].left),
                         (int)(currPPD->sizes[i].bottom - currPPD->sizes[i].top));
    }
    return QRect();
}

QStringList QCUPSSupport::options() const
{
    QStringList list;
    collectMarkedOptions(list);
    return list;
}

void QCUPSSupport::collectMarkedOptions(QStringList& list, const ppd_group_t* group) const
{
    if (group == 0) {
        for (int i = 0; i < currPPD->num_groups; ++i) {
            collectMarkedOptions(list, &currPPD->groups[i]);
            collectMarkedOptionsHelper(list, &currPPD->groups[i]);
        }
    } else {
        for (int i = 0; i < group->num_subgroups; ++i)
            collectMarkedOptionsHelper(list, &group->subgroups[i]);
    }
}

void QCUPSSupport::collectMarkedOptionsHelper(QStringList& list, const ppd_group_t* group) const
{
    for (int i = 0; i < group->num_options; ++i) {
        for (int j = 0; j < group->options[i].num_choices; ++j) {
            if (group->options[i].choices[j].marked == 1 && qstrcmp(group->options[i].choices[j].choice, group->options[i].defchoice) != 0)
                list << QString::fromLocal8Bit(group->options[i].keyword) << QString::fromLocal8Bit(group->options[i].choices[j].choice);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////


#endif // !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)

