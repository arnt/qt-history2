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

#include "widgetinfo.h"

#include <qmetaobject.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwizard.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <qmainwindow.h>
#include <qtoolbox.h>
#include <qgroupbox.h>
#include <qscrollview.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qtextbrowser.h>
#include <qdial.h>
#include <qslider.h>
#include <qiconview.h>
#include <qaction.h>
#include <q3buttongroup.h>

WidgetInfo::WidgetInfo()
{
}

const QMetaObject *WidgetInfo::metaObject(const QString &widgetName)
{
    if (widgetName == QLatin1String("QPushButton"))
        return &QPushButton::staticMetaObject;
    else if (widgetName == QLatin1String("QToolButton"))
        return &QToolButton::staticMetaObject;
    else if (widgetName == QLatin1String("QCheckBox"))
        return &QCheckBox::staticMetaObject;
    else if (widgetName == QLatin1String("QRadioButton"))
        return &QRadioButton::staticMetaObject;
    else if (widgetName == QLatin1String("QGroupBox"))
        return &Q3GroupBox::staticMetaObject;
    else if (widgetName == QLatin1String("QButtonGroup"))
        return &Q3ButtonGroup::staticMetaObject;
    else if (widgetName == QLatin1String("QDateEdit"))
        return &QDateEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QTimeEdit"))
        return &QTimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QDateTimeEdit"))
        return &QDateTimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QListBox"))
        return &QListBox::staticMetaObject;
    else if (widgetName == QLatin1String("QListView"))
        return &QListView::staticMetaObject;
    else if (widgetName == QLatin1String("QLineEdit"))
        return &QLineEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QSpinBox"))
        return &QSpinBox::staticMetaObject;
    else if (widgetName == QLatin1String("QSplitter"))
        return &QSplitter::staticMetaObject;
    else if (widgetName == QLatin1String("QTextEdit"))
        return &QTextEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QLabel"))
        return &QLabel::staticMetaObject;
    else if (widgetName == QLatin1String("QTabWidget"))
        return &QTabWidget::staticMetaObject;
    else if (widgetName == QLatin1String("QWidgetStack"))
        return &QWidgetStack::staticMetaObject;
    else if (widgetName == QLatin1String("QComboBox"))
        return &QComboBox::staticMetaObject;
    else if (widgetName == QLatin1String("QWidget"))
        return &QWidget::staticMetaObject;
    else if (widgetName == QLatin1String("QDialog"))
        return &QDialog::staticMetaObject;
    else if (widgetName == QLatin1String("QWizard"))
        return &QWizard::staticMetaObject;
    else if (widgetName == QLatin1String("QLCDNumber"))
        return &QLCDNumber::staticMetaObject;
    else if (widgetName == QLatin1String("QProgressBar"))
        return &QProgressBar::staticMetaObject;
    else if (widgetName == QLatin1String("QTextView"))
        return &QTextView::staticMetaObject;
    else if (widgetName == QLatin1String("QTextBrowser"))
        return &QTextBrowser::staticMetaObject;
    else if (widgetName == QLatin1String("QDial"))
        return &QDial::staticMetaObject;
    else if (widgetName == QLatin1String("QSlider"))
        return &QSlider::staticMetaObject;
    else if (widgetName == QLatin1String("QScrollBar"))
        return &QScrollBar::staticMetaObject;
    else if (widgetName == QLatin1String("QFrame"))
        return &QFrame::staticMetaObject;
    else if (widgetName == QLatin1String("QMainWindow"))
        return &QMainWindow::staticMetaObject;
    else if (widgetName == QLatin1String("QToolBox"))
        return &QToolBox::staticMetaObject;
    else if (widgetName == QLatin1String("Line"))
        return &QFrame::staticMetaObject;
    else if (widgetName == QLatin1String("TextLabel"))
        return &QLabel::staticMetaObject;
    else if (widgetName == QLatin1String("PixmapLabel"))
        return &QLabel::staticMetaObject;
    else if (widgetName == QLatin1String("QActionGroup"))
        return &QActionGroup::staticMetaObject;
    else if (widgetName == QLatin1String("QAction"))
        return &QAction::staticMetaObject;

    //qWarning("widget '%s' is not supported!!!", widgetName.latin1());

    // ### custom widgets
    return 0;
}

bool WidgetInfo::isValidProperty(const QString &className, const char *name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfProperty(name) != -1;
}

bool WidgetInfo::isValidEnumerator(const QString &className, const char *name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return checkEnumerator(meta, name);
}

bool WidgetInfo::isValidSignal(const QString &className, const char *name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSignal(name) != -1;
}

bool WidgetInfo::isValidSlot(const QString &className, const char *name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSlot(name) != -1;
}

bool WidgetInfo::checkEnumerator(const QMetaObject *meta, const char *name)
{
    for (int i=0; i<meta->enumeratorCount(); ++i)
        if (checkEnumerator(meta->enumerator(i), name))
            return true;
    return false;
}

bool WidgetInfo::checkEnumerator(const QMetaEnum &metaEnum, const char *name)
{
    return metaEnum.keyToValue(name) != -1;
}
