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

#include "qpagesetupdialog.h"

#include "qcombobox.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qprinter.h"
#include "qpushbutton.h"

#include <private/qabstractpagesetupdialog_p.h>

// Disabled untill we have support for papersources on unix
// #define PSD_ENABLE_PAPERSOURCE

static const char *pageSizeNames[] = {
    "A4 (210x297 mm, 8.26x11.7 inches)",
    "B5 (176 x 250 mm, 6.93x9.84 inches)",
    "Letter (8.5x11 inches, 216x279 mm)",
    "Legal (8.5x14 inches, 216x356 mm)",
    "Executive (7.5x10 inches, 191x254 mm)",
    "A0 (841 x 1189 mm)",
    "A1 (594 x 841 mm)",
    "A2 (420 x 594 mm)",
    "A3 (297 x 420 mm)",
    "A5 (148 x 210 mm)",
    "A6 (105 x 148 mm)",
    "A7 (74 x 105 mm)",
    "A8 (52 x 74 mm)",
    "A9 (37 x 52 mm)",
    "B0 (1000 x 1414 mm)",
    "B1 (707 x 1000 mm)",
    "B2 (500 x 707 mm)",
    "B3 (353 x 500 mm)",
    "B4 (250 x 353 mm)",
    "B6 (125 x 176 mm)",
    "B7 (88 x 125 mm)",
    "B8 (62 x 88 mm)",
    "B9 (44 x 62 mm)",
    "B10 (31 x 44 mm)",
    "C5E (163 x 229 mm)",
    "US Common #10 Envelope (105 x 241 mm)",
    "DLE (110 x 220 mm)",
    "Folio (210 x 330 mm)",
    "Ledger (432 x 279 mm)",
    "Tabloid (279 x 432 mm)",
    0
};

#ifdef PSD_ENABLE_PAPERSOURCE
static const char *paperSourceNames[] = {
    "Only One",
    "Lower",
    "Middle",
    "Manual",
    "Envelope",
    "Envelope manual",
    "Auto",
    "Tractor",
    "Small format",
    "Large format",
    "Large capacity",
    "Cassette",
    "Form source",
    0
};

struct PaperSourceNames
{
    PaperSourceNames(const char *nam, QPrinter::PaperSource ps)
        : paperSource(ps), name(nam) {}
    QPrinter::PaperSource paperSource;
    const char *name;
};
#endif


class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
public:
    QComboBox *pageSize;
#ifdef PSD_ENABLE_PAPERSOURCE
    QComboBox *paperSource;
#endif
    QComboBox *orientation;
};

#define d d_func()

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(11);
    layout->setSpacing(6);

    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::GroupBoxPanel);
    frame->setFrameShadow(QFrame::Sunken);
    QGridLayout *frameLayout = new QGridLayout(frame);
    frameLayout->setMargin(11);
    frameLayout->setSpacing(6);
    frameLayout->setObjectName("frame3Layout");

    QLabel *pageSizeLabel = new QLabel(tr("Page Size"), frame);
    d->pageSize = new QComboBox(frame);
    d->pageSize->setObjectName("pageSizeCombo");
    frameLayout->addWidget(pageSizeLabel, 0, 0);
    frameLayout->addWidget(d->pageSize, 0, 1);

    QLabel *orientationLabel = new QLabel(tr("Orientation"), frame);
    d->orientation = new QComboBox(frame);
    d->orientation->setObjectName("orientationCombo");
    frameLayout->addWidget(orientationLabel, 2, 0);
    frameLayout->addWidget(d->orientation, 2, 1);

#ifdef PSD_ENABLE_PAPERSOURCE
    QLabel *paperSourceLabel = new QLabel(tr("Paper Source"), frame);
    d->paperSource = new QComboBox(frame);
    d->paperSource->setObjectName("paperSourceCombo");
    frameLayout->addWidget(paperSourceLabel, 1, 0);
    frameLayout->addWidget(d->paperSource, 1, 1);
#endif

    layout->addWidget(frame);

    QSpacerItem* spacer = new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(spacer);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(6);
    buttonLayout->setObjectName("layout2");
    QSpacerItem *buttonSpacer = new QSpacerItem(71, 20, QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    QPushButton *okButton = new QPushButton(tr("OK"), this);
    okButton->setObjectName("okButton");
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setObjectName("cancelButton");

    buttonLayout->addItem(buttonSpacer);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addItem(buttonLayout);

    clearWState(Qt::WState_Polished);

    for (int i=0; pageSizeNames[i]; ++i)
        d->pageSize->insertItem(pageSizeNames[i]);
    d->pageSize->setCurrentItem(printer->pageSize());

#ifdef PSD_ENABLE_PAPERSOURCE
    for (int i=0; paperSourceNames[i]; ++i)
        d->paperSource->insertItem(paperSourceNames[i]);
    d->paperSource->setCurrentItem(printer->paperSource());
#endif

    d->orientation->insertItem(tr("Portrait"));
    d->orientation->insertItem(tr("Landscape"));
    d->orientation->setCurrentItem(printer->orientation());


    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

int QPageSetupDialog::exec()
{
    int ret = QDialog::exec();
    if (ret == Accepted) {
        // Read out the data
        d->printer->setPageSize((QPrinter::PageSize)d->pageSize->currentItem());
        d->printer->setOrientation((QPrinter::Orientation)d->orientation->currentItem());
#ifdef PSD_ENABLE_PAPERSOURCE
        d->printer->setPaperSource((QPrinter::PaperSource)d->paperSource->currentItem());
#endif
    }
    return ret;
}
