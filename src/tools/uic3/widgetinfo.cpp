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

#include "widgetinfo.h"

#include <QMetaEnum>
#include <QTextBrowser>
#include <QToolBar>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDial>
#include <QSlider>
#include <QLCDNumber>
#include <QProgressBar>
#include <QLabel>
#include <QToolBox>
#include <QMainWindow>
#include <QToolButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QActionGroup>
#include <QSplitter>
#include <QTabWidget>
#include <Q3DateEdit>
#include <Q3TimeEdit>
#include <Q3DateTimeEdit>
#include <Q3ListBox>
#include <Q3ListView>
#include <Q3TextEdit>
#include <Q3WidgetStack>
#include <Q3Wizard>
#include <Q3TextView>
#include <Q3MainWindow>
#include <Q3GroupBox>
#include <Q3ButtonGroup>
#include <Q3IconView>
#include <Q3ProgressBar>

WidgetInfo::WidgetInfo()
{
}

const QMetaObject *WidgetInfo::metaObject(const QString &widgetName)
{
    if (widgetName == QLatin1String("QObject"))
        return &QObject::staticMetaObject;
    else if (widgetName == QLatin1String("QToolBar"))
        return &QToolBar::staticMetaObject;
    else if (widgetName == QLatin1String("Q3ToolBar"))
        return &Q3ToolBar::staticMetaObject;
    else if (widgetName == QLatin1String("QPushButton"))
        return &QPushButton::staticMetaObject;
    else if (widgetName == QLatin1String("QToolButton"))
        return &QToolButton::staticMetaObject;
    else if (widgetName == QLatin1String("QCheckBox"))
        return &QCheckBox::staticMetaObject;
    else if (widgetName == QLatin1String("QRadioButton"))
        return &QRadioButton::staticMetaObject;
    else if (widgetName == QLatin1String("QGroupBox")
            || widgetName == QLatin1String("Q3GroupBox"))
        return &Q3GroupBox::staticMetaObject;
    else if (widgetName == QLatin1String("QButtonGroup")
            || widgetName == QLatin1String("Q3ButtonGroup"))
        return &Q3ButtonGroup::staticMetaObject;
    else if (widgetName == QLatin1String("QDateEdit"))
        return &Q3DateEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QTimeEdit"))
        return &Q3TimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QDateTimeEdit"))
        return &Q3DateTimeEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QListBox")
             || widgetName == QLatin1String("Q3ListBox"))
        return &Q3ListBox::staticMetaObject;
    else if (widgetName == QLatin1String("QListView") ||
             widgetName == QLatin1String("Q3ListView"))
        return &Q3ListView::staticMetaObject;
    else if (widgetName == QLatin1String("Q3IconView"))
        return &Q3IconView::staticMetaObject;
    else if (widgetName == QLatin1String("QLineEdit"))
        return &QLineEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QSpinBox"))
        return &QSpinBox::staticMetaObject;
    else if (widgetName == QLatin1String("QSplitter"))
        return &QSplitter::staticMetaObject;
    else if (widgetName == QLatin1String("QTextEdit") ||
             widgetName == QLatin1String("Q3TextEdit"))
        return &Q3TextEdit::staticMetaObject;
    else if (widgetName == QLatin1String("QLabel"))
        return &QLabel::staticMetaObject;
    else if (widgetName == QLatin1String("QTabWidget"))
        return &QTabWidget::staticMetaObject;
    else if (widgetName == QLatin1String("QWidgetStack"))
        return &Q3WidgetStack::staticMetaObject;
    else if (widgetName == QLatin1String("QComboBox"))
        return &QComboBox::staticMetaObject;
    else if (widgetName == QLatin1String("QWidget"))
        return &QWidget::staticMetaObject;
    else if (widgetName == QLatin1String("QDialog"))
        return &QDialog::staticMetaObject;
    else if (widgetName == QLatin1String("QWizard"))
        return &Q3Wizard::staticMetaObject;
    else if (widgetName == QLatin1String("QLCDNumber"))
        return &QLCDNumber::staticMetaObject;
    else if (widgetName == QLatin1String("QProgressBar"))
        return &QProgressBar::staticMetaObject;
    else if (widgetName == QLatin1String("Q3ProgressBar"))
        return &Q3ProgressBar::staticMetaObject;
    else if (widgetName == QLatin1String("QTextView")
             || widgetName == QLatin1String("Q3TextView"))
        return &Q3TextView::staticMetaObject;
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
    else if (widgetName == QLatin1String("Q3MainWindow"))
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

    return 0;
}

bool WidgetInfo::isValidProperty(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfProperty(name.toLatin1()) != -1;
}

bool WidgetInfo::isValidSignal(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSignal(name.toLatin1()) != -1;
}

bool WidgetInfo::isValidSlot(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta)
        return true;

    return meta->indexOfSlot(name.toLatin1()) != -1;
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
    return metaEnum.keyToValue(name.toLatin1()) != -1;
}


QString WidgetInfo::resolveEnumerator(const QString &className, const QString &name)
{
    const QMetaObject *meta = metaObject(className);
    if (!meta) {
        QString e = resolveEnumerator(QLatin1String("QObject"), QLatin1String("Qt::") + name);
        if (e.size())
            return e;

        return name;
    }

    return resolveEnumerator(meta, name);
}

QString WidgetInfo::resolveEnumerator(const QMetaObject *meta, const QString &name)
{
    for (int i=meta->enumeratorCount() - 1; i>=0; --i) {
        QString e = resolveEnumerator(meta->enumerator(i), name);
        if (e.size())
            return e;
    }

    if (meta != &staticQtMetaObject)
        return resolveEnumerator(&staticQtMetaObject, name);

    return QString();
}

QString WidgetInfo::resolveEnumerator(const QMetaEnum &metaEnum, const QString &name)
{
    QString scope = QLatin1String(metaEnum.scope());

    QByteArray key = name.toLatin1();
    for (int idx = 0; idx < metaEnum.keyCount(); ++idx) {
        if (metaEnum.key(idx) == key) {
            QString enumerator = name;
            int i = enumerator.indexOf(QLatin1String("::"));
            if (i != -1)
                enumerator = enumerator.mid(i + 2);
            
            return scope + QLatin1String("::") + enumerator;
        }
    }

    return QString();
}
