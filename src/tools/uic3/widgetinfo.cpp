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

#include <QtGui/QtGui>
#include <Qt3Compat/Qt3Compat>

WidgetInfo::WidgetInfo()
{
}

const QMetaObject *WidgetInfo::metaObject(const QString &widgetName)
{
    if (widgetName == QLatin1String("QObject"))
        return &QObject::staticMetaObject;
    else if (widgetName == QLatin1String("QPushButton"))
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
        return &Q3DateEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QTimeEdit"))
        return &Q3TimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QDateTimeEdit"))
        return &Q3DateTimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QListBox"))
        return &QListBox::staticMetaObject;
    else if (widgetName == QLatin1String("QListView") ||
             widgetName == QLatin1String("Q3ListView"))
        return &Q3ListView::staticMetaObject;
    else if (widgetName == QLatin1String("QLineEdit"))
        return &QLineEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QSpinBox"))
        return &QSpinBox::staticMetaObject;
    else if (widgetName == QLatin1String("QSplitter"))
        return &QSplitter::staticMetaObject;
    else if (widgetName == QLatin1String("QTextEdit"))
        return &Q3TextEdit::staticMetaObject;
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
        return &Q3MainWindow::staticMetaObject;
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

bool WidgetInfo::isValidProperty(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfProperty(name) != -1;
}

bool WidgetInfo::isValidSignal(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSignal(name) != -1;
}

bool WidgetInfo::isValidSlot(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSlot(name) != -1;
}

bool WidgetInfo::isValidEnumerator(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return checkEnumerator(meta, name);
}

bool WidgetInfo::checkEnumerator(const QMetaObject *meta, const QString &name)
{
    for (int i=0; i<meta->enumeratorCount(); ++i)
        if (checkEnumerator(meta->enumerator(i), name))
            return true;
    return false;
}

bool WidgetInfo::checkEnumerator(const QMetaEnum &metaEnum, const QString &name)
{
    return metaEnum.keyToValue(name) != -1;
}


QString WidgetInfo::resolveEnumerator(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta) {
        QString e = resolveEnumerator("QObject", QLatin1String("Qt::") + name);
        if (e.size())
            return e;

        return QLatin1String(name);
    }

    return resolveEnumerator(meta, name);
}

QString WidgetInfo::resolveEnumerator(const QMetaObject *meta, const QString &name)
{
    for (int i=0; i<meta->enumeratorCount(); ++i) {
        QString e = resolveEnumerator(meta->enumerator(i), name);
        if (e.size())
            return e;
    }

    return QString::null;
}

QString WidgetInfo::resolveEnumerator(const QMetaEnum &metaEnum, const QString &name)
{
    QString scope = QLatin1String(metaEnum.scope());

    int idx = metaEnum.keyToValue(name);
    if (idx != -1) {
        QString enumerator = name;
        int i = enumerator.indexOf("::");
        if (i != -1)
            enumerator = enumerator.mid(i + 2);

        return scope + QLatin1String("::") + enumerator;
    }

    return QString::null;
}

