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

#include "defaultspecialeditor.h"
#include <abstractformwindowcursor.h>
#include <abstractformwindow.h>
#include <qextensionmanager.h>
#include <propertysheet.h>

#include <QDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>

QDesignerSpecialEditor::QDesignerSpecialEditor(QWidget *theWidget, IPropertySheet *properties,
                                               QObject *parent)
    : QObject(parent), mWidget(theWidget), mDialog(0), mLe(0), mProperties(properties)
{
}

QDesignerSpecialEditor::~QDesignerSpecialEditor()
{
    delete mDialog;
}

QWidget *QDesignerSpecialEditor::createEditor(QWidget * /*parent*/)
{
    // ### rewrite this using designer :-) (if you don't want to, remove the comment).
    if (!mDialog) {
        mDialog = new QDialog;
        QLabel *lbl = new QLabel(tr("Enter the new text for the widget:"), mDialog);
        mLe = new QLineEdit(mDialog);
        QVBoxLayout *layout = new QVBoxLayout(mDialog);
        layout->addWidget(lbl);
        layout->addWidget(mLe);

        QHBoxLayout *hLayout = new QHBoxLayout;
        QPushButton *cmdOK = new QPushButton(tr("OK"), mDialog);
        QPushButton *cmdCancel = new QPushButton(tr("Cancel"), mDialog);

        connect(mLe, SIGNAL(returnPressed()), mDialog, SLOT(accept()));
        connect(mLe, SIGNAL(returnPressed()), this, SLOT(applyChanges()));
        connect(cmdOK, SIGNAL(clicked()), mDialog, SLOT(accept()));
        connect(cmdOK, SIGNAL(clicked()), this, SLOT(applyChanges()));
        cmdOK->setDefault(true);
        connect(cmdCancel, SIGNAL(clicked()), mDialog, SLOT(reject()));
        connect(cmdCancel, SIGNAL(clicked()), this, SLOT(revertChanges()));

        hLayout->addWidget(cmdCancel);
        hLayout->addWidget(cmdOK);
        layout->addLayout(hLayout);
    }
    mLe->setText(mProperties->property(mProperties->indexOf("text")).toString());
    mLe->selectAll();
    return mDialog;
}

void QDesignerSpecialEditor::applyChanges()
{
    if (!mDialog)
        return;
    AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(mWidget);
    fw->cursor()->setProperty("text", mLe->text());
}

void QDesignerSpecialEditor::revertChanges()
{
}

QDesignerSpecialEditorFactory::QDesignerSpecialEditorFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *QDesignerSpecialEditorFactory::createExtension(QObject *object, const QString &iid,
                                                        QObject *parent) const
{
    if (iid == Q_TYPEID(ISpecialEditor) && object->isWidgetType()) {
        IPropertySheet *properties = qt_cast<IPropertySheet *>(extensionManager()->extension(object,
                                                                        Q_TYPEID(IPropertySheet)));
        if (properties && properties->indexOf("text") != -1) {
            if (qt_cast<QAbstractButton *>(object))
                return new QButtonSpecialEditor(static_cast<QPushButton *>(object), properties,
                                                parent);
            else
                return new QDesignerSpecialEditor(static_cast<QWidget *>(object), properties,
                                                  parent);
        }
    }
    return 0;
}

class ButtonLineEdit : public QWidget // QLineEdit doesn't have let me pass flags
{
    Q_OBJECT
    QLineEdit *mLineEdit;
public:
    ButtonLineEdit(QWidget *parent) : QWidget(parent, Qt::WType_Popup)
    {
        mLineEdit = new QLineEdit(this);
        QVBoxLayout *vb = new QVBoxLayout(this);
        vb->setSpacing(0);
        vb->setMargin(0);
        vb->addWidget(mLineEdit);
    }
    ~ButtonLineEdit() {};
    QLineEdit *lineEdit() const { return mLineEdit; }

protected:
    void showEvent(QShowEvent *ev) {
        QWidget::showEvent(ev);
        mLineEdit->setFocus();
    }
    void closeEvent(QCloseEvent *ev) {
        emit changesRejected();
        QWidget::closeEvent(ev);
    }
signals:
    void changesRejected();

};

QButtonSpecialEditor::QButtonSpecialEditor(QAbstractButton *theButton, IPropertySheet *properties,
                                           QObject *parent)
    : QObject(parent), mButton(theButton), mLe(0), mProperties(properties)
{
}

QButtonSpecialEditor::~QButtonSpecialEditor()
{
}

QWidget *QButtonSpecialEditor::createEditor(QWidget *parent)
{
    if (!mLe) {
        mLe = new ButtonLineEdit(parent);
        connect(mLe->lineEdit(), SIGNAL(returnPressed()), this, SLOT(applyChanges()));
        connect(mLe, SIGNAL(changesRejected()), this, SLOT(revertChanges()));
    }
    mOriginalText = mProperties->property(mProperties->indexOf("text")).toString();
    mLe->lineEdit()->setAlignment(Qt::AlignCenter);
    mLe->lineEdit()->setText(mOriginalText);
    mLe->lineEdit()->selectAll();
    mLe->resize(QSize(qMax(mButton->width(), mLe->fontMetrics().width(mOriginalText) + 20),
                mLe->height()));
    QRect rect = QRect(QPoint(0, 0), QSize(mLe->size()));
    rect.moveCenter(parent->mapToGlobal(mButton->geometry().center()));
    mLe->move(rect.topLeft());
    return mLe;
}

void QButtonSpecialEditor::applyChanges()
{
    AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(mButton);
    fw->cursor()->setProperty("text", mLe->lineEdit()->text());
    mLe->hide();
}

void QButtonSpecialEditor::revertChanges()
{
    AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(mButton);
    fw->cursor()->setProperty("text", mOriginalText);
    mLe->hide();
}

#include "defaultspecialeditor.moc"
