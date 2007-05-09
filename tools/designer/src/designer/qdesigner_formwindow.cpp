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

#include "qdesigner_formwindow.h"
#include "qdesigner_workbench.h"
#include "qdesigner_settings.h"

// sdk
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QUndoCommand>
#include <QtGui/QWindowStateChangeEvent>


QDesignerFormWindow::QDesignerFormWindow(QDesignerFormWindowInterface *editor, QDesignerWorkbench *workbench, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_editor(editor),
      m_workbench(workbench),
      m_action(new QAction(this)),
      m_initialized(false),
      m_windowTitleInitialized(false)
{
    Q_ASSERT(workbench);

    setMaximumSize(0xFFF, 0xFFF);

    if (m_editor) {
        m_editor->setParent(this);
    } else {
        m_editor = workbench->core()->formWindowManager()->createFormWindow(this);
    }

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->addWidget(m_editor);

    m_action->setCheckable(true);

    connect(m_editor->commandHistory(), SIGNAL(indexChanged(int)), this, SLOT(updateChanged()));
    connect(m_editor, SIGNAL(geometryChanged()), this, SLOT(geometryChanged()));
    connect(m_editor, SIGNAL(activated(QWidget *)), this, SLOT(widgetActivated(QWidget *)));
}

QDesignerFormWindow::~QDesignerFormWindow()
{
    if (workbench())
        workbench()->removeFormWindow(this);
}

void QDesignerFormWindow::widgetActivated(QWidget *widget)
{
    if (const QDesignerTaskMenuExtension *taskMenu = qt_extension<QDesignerTaskMenuExtension*>(m_editor->core()->extensionManager(), widget)) {
        QAction *action = taskMenu->preferredEditAction();
        if (!action) {
            const QList<QAction *> actions = taskMenu->taskActions();
            if (!actions.isEmpty())
                action = actions.first();
        }
        if (action) {
            QTimer::singleShot(0, action, SIGNAL(triggered()));
        }
    }
}

QAction *QDesignerFormWindow::action() const
{
    return m_action;
}

void QDesignerFormWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::ActivationChange: {
            if (isActiveWindow()) {
                m_action->setChecked(true);
                // ### raise();
            }
        } break;
        case QEvent::WindowTitleChange:
            m_action->setText(windowTitle().remove(QLatin1String("[*]")));
            break;
        case QEvent::WindowIconChange:
            m_action->setIcon(windowIcon());
            break;
    case QEvent::WindowStateChange: {
        const  QWindowStateChangeEvent *wsce =  static_cast<const QWindowStateChangeEvent *>(e);
        const bool wasMinimized = Qt::WindowMinimized & wsce->oldState();
        const bool isMinimizedNow = isMinimized();
        if (wasMinimized != isMinimizedNow )
            emit minimizationStateChanged(m_editor, isMinimizedNow);
    }
        break;
        default:
            break;
    }
    QWidget::changeEvent(e);
}

QRect QDesignerFormWindow::geometryHint() const
{
    const QPoint point(0, 0);
    // If we have a container, we want to be just as big.
    // QMdiSubWindow attempts to resize its children to sizeHint() when switching user interface modes.
    if (QWidget *mainContainer = m_editor->mainContainer())
        return QRect(point, mainContainer->size());

    return QRect(point, sizeHint());
}

QDesignerFormWindowInterface *QDesignerFormWindow::editor() const
{
    return m_editor;
}

QDesignerWorkbench *QDesignerFormWindow::workbench() const
{
    return m_workbench;
}

void QDesignerFormWindow::firstShow()
{
    // Set up handling of file name changes and set initial title.
    if (!m_windowTitleInitialized) {
        m_windowTitleInitialized = true;
        if (m_editor) {
            connect(m_editor, SIGNAL(fileNameChanged(QString)), this, SLOT(updateWindowTitle(QString)));
            updateWindowTitle(m_editor->fileName());
        }
    }
    show();
}

int QDesignerFormWindow::getNumberOfUntitledWindows() const
{
    const int totalWindows = m_workbench->formWindowCount();
    if (!totalWindows)
        return 0;

    int maxUntitled = 0;
    // Find the number of untitled windows excluding ourselves.
    // Do not fall for 'untitled.ui', match with modified place holder.
    // This will cause some problems with i18n, but for now I need the string to be "static"
    QRegExp rx(QLatin1String("untitled( (\\d+))?\\[\\*\\]"));
    for (int i = 0; i < totalWindows; ++i) {
        QDesignerFormWindow *fw =  m_workbench->formWindow(i);
        if (fw != this) {
            const QString title = m_workbench->formWindow(i)->windowTitle();
            if (rx.indexIn(title) != -1) {
                if (maxUntitled == 0)
                    ++maxUntitled;
                if (rx.numCaptures() > 1) {
                    const QString numberCapture = rx.cap(2);
                    if (!numberCapture.isEmpty())
                        maxUntitled = qMax(numberCapture.toInt(), maxUntitled);
                }
            }
        }
    }
    return maxUntitled;
}

void QDesignerFormWindow::updateWindowTitle(const QString &fileName)
{
    if (!m_windowTitleInitialized) {
        m_windowTitleInitialized = true;
        if (m_editor)
            connect(m_editor, SIGNAL(fileNameChanged(QString)), this, SLOT(updateWindowTitle(QString)));
    }

    QString fileNameTitle;
    if (fileName.isEmpty()) {
        fileNameTitle = QLatin1String("untitled");
        if (const int maxUntitled = getNumberOfUntitledWindows()) {
            fileNameTitle += QLatin1Char(' ');
            fileNameTitle += QString::number(maxUntitled + 1);
        }
    } else {
        fileNameTitle = QFileInfo(fileName).fileName();
    }

    if (const QWidget *mc = m_editor->mainContainer()) {
        setWindowIcon(mc->windowIcon());
        setWindowTitle(tr("%1 - %2[*]").arg(mc->windowTitle()).arg(fileNameTitle));
    } else {
        setWindowTitle(fileNameTitle);
    }
}

void QDesignerFormWindow::closeEvent(QCloseEvent *ev)
{
    if (m_editor->isDirty()) {
        raise();
        QMessageBox box(QMessageBox::Information, tr("Save Form?"),
                tr("Do you want to save the changes to this document before closing?"),
                QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save, m_editor);
        box.setInformativeText(tr("If you don't save, your changes will be lost."));
        box.setWindowModality(Qt::WindowModal);
        static_cast<QPushButton *>(box.button(QMessageBox::Save))->setDefault(true);

        switch (box.exec()) {
            case QMessageBox::Save: {
                bool ok = workbench()->saveForm(m_editor);
                ev->setAccepted(ok);
                m_editor->setDirty(!ok);
                break;
            }
            case QMessageBox::Discard:
                m_editor->setDirty(false); // Not really necessary, but stops problems if we get close again.
                ev->accept();
                break;
            case QMessageBox::Cancel:
                ev->ignore();
                break;
        }
    }
}

void QDesignerFormWindow::updateChanged()
{
    // Sometimes called after form window destruction.
    if (m_editor) {
        setWindowModified(m_editor->isDirty());
        updateWindowTitle(m_editor->fileName());
    }
}

void QDesignerFormWindow::resizeEvent(QResizeEvent *rev)
{
    if(m_initialized) {
        m_editor->setDirty(true);
        setWindowModified(true);
    }

    m_initialized = true;
    QWidget::resizeEvent(rev);

    // update the maincontainer on resize
    m_editor->mainContainer()->raise();
    m_editor->mainContainer()->update();
}

void QDesignerFormWindow::geometryChanged()
{
    if(QObject *object = m_editor->core()->propertyEditor()->object()) {
        QDesignerPropertySheetExtension *sheet =
            qt_extension<QDesignerPropertySheetExtension*>(m_editor->core()->extensionManager(), object);
        m_editor->core()->propertyEditor()->setPropertyValue(QLatin1String("geometry"), sheet->property(sheet->indexOf(QLatin1String("geometry"))));
    }
}
