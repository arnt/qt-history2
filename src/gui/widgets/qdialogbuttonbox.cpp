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

#include <QtCore/QHash>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/private/qwidget_p.h>

#include "qdialogbuttonbox.h"

/*!
    \class QDialoButtonBox

    \brief The QDialogButtonBox class is a widget that presents buttons in a
    certain layout depending upon the style.

    \ingroup Not sure...

    Dialogs and message boxes typically present buttons in a layout that
    conforms to the interface guidelines for that platform. Invariably,
    different platforms have different layouts for their dialogs.
    QDialogButtonBox allows a user to add buttons to it and have the button box
    determine the layout for user.

    Most buttons for a dialog follow into certain roles. These roles include,
    accepting or rejecting the dialog, asking for help, doing actions on the
    dialog itself (such as resetting fields or applying changes). There can
    also be alternate ways of dismissing the dialog (some with destructive
    results).

    Most dialogs have buttons that can almost considered to be standard (e.g.
    OK and Cancel buttons). It is sometimes convenient to create these buttons
    in a standard way.

    Typically, you create a QDialogButtonBox with the orientation you want and
    then add buttons to it specifying the role for each button.

    ### Some quoted example showing this in action

    Currently the buttons are laid out in the following way.

    ### Some sort of table indicating the layout style.

    When a button is clicked in the button box, the clicked() signal is emitted
    (once with the ButtonRole and once with the actual button). For
    convenience, if the button has an AcceptRole or a RejectRole the accepted
    and rejected signals are emitted respectively.
*/

enum { AcceptRole = QDialogButtonBox::AcceptRole, RejectRole = QDialogButtonBox::RejectRole,
       AlternateRole = QDialogButtonBox::AlternateRole,
       DestructiveRole = QDialogButtonBox::DestructiveRole,
       ActionRole = QDialogButtonBox::ActionRole, HelpRole = QDialogButtonBox::HelpRole,
       Stretch = 0x10000000, MacSpacer = 0x20000000, EOL = 0x40000000 };


static QDialogButtonBox::ButtonRole roleFor(QDialogButtonBox::StandardButton button)
{
    switch (button) {
    case QDialogButtonBox::Ok:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Save:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Open:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Cancel:
        return QDialogButtonBox::RejectRole;
    case QDialogButtonBox::Close:
        return QDialogButtonBox::RejectRole;
    case QDialogButtonBox::DontSave:
        return QDialogButtonBox::DestructiveRole;
    case QDialogButtonBox::Help:
        return QDialogButtonBox::HelpRole;
    case QDialogButtonBox::Apply:
        return QDialogButtonBox::ActionRole;
    case QDialogButtonBox::Reset:
        return QDialogButtonBox::ActionRole;
    default:
        return QDialogButtonBox::InvalidRole;
    }
}

static const int layouts[2][4][9] =
{
    // Qt::Horizontal
    {
        // WinLayout
        { Stretch, AcceptRole, AlternateRole, DestructiveRole, RejectRole, ActionRole, HelpRole, EOL, EOL },

        // MacLayout
        { HelpRole, ActionRole, Stretch, -DestructiveRole, MacSpacer, -AlternateRole, -RejectRole, -AcceptRole, EOL },

        // KdeLayout
        { HelpRole, Stretch, AcceptRole, AlternateRole, ActionRole, DestructiveRole, RejectRole, EOL, EOL },

        // GnomeLayout
        { HelpRole, Stretch, -ActionRole, -DestructiveRole, -AlternateRole, -RejectRole, -AcceptRole, EOL, EOL }
    },

    // Qt::Vertical
    {
        // WinLayout
        { AcceptRole, AlternateRole, DestructiveRole, RejectRole, ActionRole, HelpRole, Stretch, EOL, EOL },

        // MacLayout
        { AcceptRole, RejectRole, AlternateRole, MacSpacer, DestructiveRole, Stretch, ActionRole, HelpRole, EOL },

        // KdeLayout
        { ActionRole, Stretch, AcceptRole, AlternateRole, DestructiveRole, RejectRole, HelpRole, EOL, EOL },

        // GnomeLayout
        { AcceptRole, RejectRole, AlternateRole, DestructiveRole, ActionRole, Stretch, HelpRole, EOL, EOL }
    }
};

class QDialogButtonBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDialogButtonBox)
public:
    QDialogButtonBoxPrivate(Qt::Orientation orient);
    ~QDialogButtonBoxPrivate();

    QList<QAbstractButton *> buttonLists[QDialogButtonBox::NRoles - 1];
    QHash<QAbstractButton *, QDialogButtonBox::StandardButton> standardButtonHash;

    Qt::Orientation orientation;
    QDialogButtonBox::LayoutPolicy layoutPolicy;
    QBoxLayout *buttonLayout;
    bool skipDisconnect;

    void createStandardButtons(QDialogButtonBox::StandardButtons buttons);

    void layoutButtons();
    void initLayout();
    QAbstractButton *createButton(QDialogButtonBox::StandardButton button, bool doLayout = true);
    void addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role, bool doLayout = true);
    void _q_handleButtonDestroyed();
    void _q_handleButtonClicked();
};

QDialogButtonBoxPrivate::QDialogButtonBoxPrivate(Qt::Orientation orient)
    : orientation(orient), skipDisconnect(false)
{
    layoutPolicy = QDialogButtonBox::MacLayout; // q->styleHint(SH_DialogButtonLayoutPolicy)
}

QDialogButtonBoxPrivate::~QDialogButtonBoxPrivate()
{
    // Just clear the lists, since I can't really guarantee when those other things will be deleted
    standardButtonHash.clear();
    for (int i = 0; i < QDialogButtonBox::NRoles - 1; ++i)
        buttonLists[i].clear();
}

void QDialogButtonBoxPrivate::initLayout()
{
    Q_Q(QDialogButtonBox);
    if (orientation == Qt::Horizontal)
        buttonLayout = new QHBoxLayout(q);
    else
        buttonLayout = new QVBoxLayout(q);
    switch (layoutPolicy) {
    case QDialogButtonBox::MacLayout:
        buttonLayout->setMargin(0);
        buttonLayout->setSpacing(0);
        break;
    default:
        buttonLayout->setMargin(0);
        break;
    }
}

void QDialogButtonBoxPrivate::layoutButtons()
{
    for (int i = buttonLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = buttonLayout->takeAt(i);
        if (QWidget *widget = item->widget())
            widget->hide();
        delete item;
    }
    

    const int *currentLayout = layouts[orientation == Qt::Vertical][layoutPolicy];

    while (*currentLayout != EOL) {
        switch (*currentLayout) {
        case Stretch:
            buttonLayout->addStretch();
            break;
        case MacSpacer:
            buttonLayout->addSpacing(19);
            break;
        case QDialogButtonBox::AcceptRole:
        case QDialogButtonBox::RejectRole:
        case QDialogButtonBox::AlternateRole:
        case QDialogButtonBox::DestructiveRole:
        case QDialogButtonBox::ActionRole:
        case QDialogButtonBox::HelpRole:
        case -QDialogButtonBox::AcceptRole:
        case -QDialogButtonBox::RejectRole:
        case -QDialogButtonBox::AlternateRole:
        case -QDialogButtonBox::DestructiveRole:
        case -QDialogButtonBox::ActionRole:
        case -QDialogButtonBox::HelpRole: {
            const QList<QAbstractButton *> &list = buttonLists[qAbs(*currentLayout) - 1];
            int start,
                stop,
                dir;
            if (currentLayout < 0) {
                start = list.count() - 1;
                stop = dir = -1;
            } else {
                start = 0;
                stop = list.count();
                dir = 1;
            }
            while (start != stop) {
                QAbstractButton *button = list.at(start);
                buttonLayout->addWidget(button);
                button->show();
                start += dir;
            }
        }
        default:
            break;
        }
        ++currentLayout;
    }
}

QAbstractButton *QDialogButtonBoxPrivate::createButton(QDialogButtonBox::StandardButton sbutton,
                                                       bool doLayout)
{
    Q_Q(QDialogButtonBox);
    QString buttonText;

    switch (sbutton) {
    case QDialogButtonBox::Ok:
        buttonText = QDialogButtonBox::tr("OK"); // Well, at least I can kill "Ok" buttons.
        break;
    case QDialogButtonBox::Save:
        buttonText = QDialogButtonBox::tr("Save");
        break;
    case QDialogButtonBox::Open:
        buttonText = QDialogButtonBox::tr("Open");
        break;
    case QDialogButtonBox::Cancel:
        buttonText = QDialogButtonBox::tr("Cancel");
        break;
    case QDialogButtonBox::Close:
        buttonText = QDialogButtonBox::tr("Close");
        break;
    case QDialogButtonBox::Apply:
        buttonText = QDialogButtonBox::tr("Apply");
        break;
    case QDialogButtonBox::Reset:
        buttonText = QDialogButtonBox::tr("Reset");
        break;
    case QDialogButtonBox::Help:
        buttonText = QDialogButtonBox::tr("Help");
        break;
    case QDialogButtonBox::DontSave:
        if (layoutPolicy == QDialogButtonBox::MacLayout)
            buttonText = QDialogButtonBox::tr("Don't Save");
        else
            buttonText = QDialogButtonBox::tr("Discard");
        break;
    default:
        break;
    }

    if (buttonText.isEmpty())
        return 0;

    QPushButton *button = new QPushButton(buttonText, q);
    standardButtonHash.insert(button, sbutton);
    addButton(button, roleFor(sbutton), doLayout);
    return button;
}

void QDialogButtonBoxPrivate::addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role,
                                        bool doLayout)
{
    Q_Q(QDialogButtonBox);
    QObject::connect(button, SIGNAL(clicked()), q, SLOT(_q_handleButtonClicked()));
    QObject::connect(button, SIGNAL(destroyed()), q, SLOT(_q_handleButtonDestroyed()));
    buttonLists[role - 1].append(button);
    if (doLayout)
        layoutButtons();
}

void QDialogButtonBoxPrivate::createStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
    uint i = QDialogButtonBox::Ok;
    while (i <= QDialogButtonBox::Help) {
        if (i & buttons) {
            createButton(QDialogButtonBox::StandardButton(i), false);
        }
        i = i << 1;
    }
    layoutButtons();
}

/*!
    Create a horizontal QDialogButtonBox with \a parent as its parent widget. The
    QDialogButtonBox will have no buttons.

    \sa setOrientation() addButton()
*/
QDialogButtonBox::QDialogButtonBox(QWidget *parent)
    : QWidget(*new QDialogButtonBoxPrivate(Qt::Horizontal), parent, 0)
{
    d_func()->initLayout();
}

/*!
    Create a QDialogButtonBox with orientation \a orientation and \a parent as
    its parent widget.  The QDialogButtonBox will have no buttons.

    \sa setOrientiation() addButton()
*/
QDialogButtonBox::QDialogButtonBox(Qt::Orientation orientation, QWidget *parent)
    : QWidget(*new QDialogButtonBoxPrivate(orientation), parent, 0)
{
    d_func()->initLayout();
}

/*!
    Create a QDialogButtonBox with orientation \a orientation, parent \a
    parent, and create the standard buttons as indicated by \a buttons

    \sa setOrientiation() addButton()
*/
QDialogButtonBox::QDialogButtonBox(StandardButtons buttons, Qt::Orientation orientation,
                                   QWidget *parent)
    : QWidget(*new QDialogButtonBoxPrivate(orientation), parent, 0)
{
    d_func()->initLayout();
    d_func()->createStandardButtons(buttons);
}

/*!
    Destroys the button box.
*/
QDialogButtonBox::~QDialogButtonBox()
{
}

/*!
    enum QDialogButtonBox::ButtonRole

    This enum describes roles for a button.

    \value InvalidRole An invalid role.
    \value AcceptRole  The role for accepting a dialog (e.g. OK).
    \value RejectRole The role for rejecting a dialog (e.g. Cancel).
    \value AlternateRole The role for dismissing a dialog in an alternate way
    \value DestructiveRole The role for dismissing a dialog when dismissing the dialog will cause a destructive change (e.g. for Discarding Changes).
    \value ActionRole The role for buttons in the dialog that affect elements in the dialog (e.g. reset all the values or read defaults)
    \value HelpRole The role for asking for help,
    \omitvalue NRoles
*/

/*!
    enum QDialogButtonBox::StandardButton

    These enums describe flags for standard buttons. Each button has a
    defined ButtonRole.

    \value Ok An "OK" button. It has the AcceptRole.
    \value Open A "Open" button. It has the AcceptRole.
    \value Save A "Save" button. It has the AcceptRole.
    \value Cancel A "Cancel" button, It has the RejectRole.
    \value Close A "Close" button. It has the RejectRole.
    \value DontSave A "Don't Save" or "Discard" button depending on the platform.
                    It has the DestructiveRole.
    \value Apply An "Apply" button. It has the ActionRole.
    \value Reset A "Reset" button. It has the ActionRole.
    \value Help A "Help" button. It has the HelpRole.
*/

/*!
    \fn QDialogButtonBox::clicked(int buttonRole)

    This signal is emitted when a button inside the button box is clicked. The
    ButtonRole of the button that was clicked is contained \a buttonRole.
*/

/*!
    \fn QDialogButtonBox::clicked(QAbstractButton *button)

    This signal is emitted when a button inside the button box is clicked. The
    specific button that was pressed is in \a button.
*/

/*!
    \fn QDialogButtonBox::accepted()

    This signal is emitted by any button that has the AcceptRole and is clicked.
*/

/*!
    \fn QDialogButtonBox::rejected()

    This signal is emitted by any button that has the RejectRole and is clicked.
*/

/*!
    \property QDialogButtonBox::orientation
    \brief the orientation of the button box

    By default the orientation is horzontal (i.e., the buttons are laid out
    side by side). The possible orientations are Qt::Horizontal and
    Qt::Vertical.
*/
Qt::Orientation QDialogButtonBox::orientation() const
{
    return d_func()->orientation;
}

void QDialogButtonBox::setOrientation(Qt::Orientation orientation)
{
    Q_D(QDialogButtonBox);
    if (orientation == d->orientation)
        return;

    d->orientation = orientation;
    delete d->buttonLayout;
    d->initLayout();
    d->layoutButtons();
}

/*!
    \property QDialogButtonBox::layoutPolicy
    \brief the layout style of the button box

    This controls the way that buttons are laid out. By default, this follows
    the style. This is included here to allow people to override the
    correct style and check how things look on other platforms.
*/
void QDialogButtonBox::setLayoutPolicy(LayoutPolicy style)
{
    Q_D(QDialogButtonBox);
    if (style == d->layoutPolicy)
        return;

    d->layoutPolicy = style;
    d->layoutButtons();
}

QDialogButtonBox::LayoutPolicy QDialogButtonBox::layoutPolicy() const
{
    return d_func()->layoutPolicy;
}

/*!
    Delete all buttons from the button box.

    \sa removeButton() addButton()
*/
void QDialogButtonBox::clear()
{
    Q_D(QDialogButtonBox);
    // Remove the created standard buttons, they should be in the other lists, which will
    // do the deletion
    d->standardButtonHash.clear();
    for (int i = 0; i < NRoles - 1; ++i) {
        QList<QAbstractButton *> &list = d->buttonLists[i];
        while (list.count()) {
            QAbstractButton *button = list.takeAt(0);
            QObject::disconnect(button, SIGNAL(destroyed()), this, SLOT(_q_handleButtonDestroyed()));
            delete button;
        }
    }
}

/*!

    Return a list of all the buttons that have been added to the
    QDialogButtonBox. The current order follows the ButtonRoles (i.e. all
    buttons with the AcceptRole, followed by all buttons with the RejectRole,
    etc.).

    \sa buttonRole() addButton() removeButton()

*/
QList<QAbstractButton *> QDialogButtonBox::buttons() const
{
    Q_D(const QDialogButtonBox);
    QList<QAbstractButton *> finalList;
    for (int i = 0; i < NRoles - 1; ++i) {
        const QList<QAbstractButton *> &list = d->buttonLists[i];
        for (int j = 0; j < list.count(); ++j)
            finalList.append(list.at(j));
    }
    return finalList;
}

/*!
    Returns the button role for \a button. This function returns InvalidRole if
    \a button is 0 or has not been added to the QDialogButtonBox.

    \sa buttons() addButton()
*/
QDialogButtonBox::ButtonRole QDialogButtonBox::buttonRole(QAbstractButton *button) const
{
    Q_D(const QDialogButtonBox);
    for (int i = 0; i < NRoles - 1; ++i) {
        const QList<QAbstractButton *> &list = d->buttonLists[i];
        for (int j = 0; j < list.count(); ++j) {
            if (list.at(j) == button)
                return ButtonRole(i + 1);
        }
    }
    return InvalidRole;
}

/*!
    Removes \a button from the QDialogButtonBox without deleting it. This is
    typically done when you want to change the parent of \a button.

    \sa clear() buttons() addButton()
*/
void QDialogButtonBox::removeButton(QAbstractButton *button)
{
    Q_D(QDialogButtonBox);
    // Remove it from the standard button hash first and then from the roles
    d->standardButtonHash.remove(button);
    for (int i = 0; i < NRoles - 1; ++i) {
        QList<QAbstractButton *> &list = d->buttonLists[i];
        for (int j = 0; j < list.count(); ++j) {
            if (list.at(j) == button) {
                list.takeAt(j);
                if (!d->skipDisconnect) {
                    disconnect(button, SIGNAL(clicked()), this, SLOT(_q_handleButtonClicked()));
                    disconnect(button, SIGNAL(destroyed()), this, SLOT(_q_handleButtonDestroyed()));
                }
                break;
            }
        }
    }
}

/*!
    Add \a button to the QDialogButtonBox with ButtonRole \a role. If \a role
    is invalid, nothing happens. If \a button has already been added, it is
    removed and added again with the new \a role.

    \sa removeButton() clear()
*/
void QDialogButtonBox::addButton(QAbstractButton *button, ButtonRole role)
{
    Q_D(QDialogButtonBox);
    if (role <= InvalidRole || role >= NRoles) {
        qWarning("QDialogButtonBox::addButton: Invalid ButtonRole, button not added");
        return;
    }
    removeButton(button);
    button->setParent(this);
    d->addButton(button, role);
}

/*!
    Creates a QPushButton with text \a text and add it to the QDialogButtonBox
    with role \a role. The button is returned from the function. If \a role is
    invalid, no button is created and zero is returned.
    \sa removeButton() clear()
*/
QAbstractButton *QDialogButtonBox::addButton(const QString &text, ButtonRole role)
{
    Q_D(QDialogButtonBox);
    if (role <= InvalidRole || role >= NRoles) {
        qWarning("QDialogButtonBox::addButton: Invalid ButtonRole, button not added");
        return 0;
    }
    QPushButton *button = new QPushButton(text, this);
    d->addButton(button, role);
    return button;
}

/*!
    Add StandardButton \a button to the QDialogButtonBox.

    If \a button is valid, a valid QPushButton pointer is returned, otherwise a
    null pointer is returned.

    \sa removeButton() clear()
*/
QAbstractButton *QDialogButtonBox::addButton(StandardButton button)
{
    Q_D(QDialogButtonBox);
    return d->createButton(button);
}

/*!
    \property QDialogButtonBox::standardButtons
    \brief The collection of standard buttons the button box has.

    This property controls which standard buttons the dialog button box has.

    \sa addButton()
*/
void QDialogButtonBox::setStandardButtons(StandardButtons buttons)
{
    Q_D(QDialogButtonBox);
    // Clear out all the old standard buttons, then recreate them.
    qDeleteAll(d->standardButtonHash.keys());
    d->standardButtonHash.clear();

    d->createStandardButtons(buttons);
}

QDialogButtonBox::StandardButtons QDialogButtonBox::standardButtons() const
{
    Q_D(const QDialogButtonBox);
    StandardButtons standardButtons = NoButtons;
    QHash<QAbstractButton *, StandardButton>::const_iterator it
                                                            = d->standardButtonHash.constBegin();
    while (it != d->standardButtonHash.constEnd()) {
        standardButtons |= it.value();
        ++it;
    }
    return standardButtons;
}

void QDialogButtonBoxPrivate::_q_handleButtonClicked()
{
    Q_Q(QDialogButtonBox);
    if (QAbstractButton *button = qobject_cast<QAbstractButton *>(q->sender())) {
        emit q->clicked(button);
        QDialogButtonBox::ButtonRole role = q->buttonRole(button);
        emit q->clicked(role);
        switch (role) {
        case AcceptRole:
            emit q->accepted();
            break;
        case RejectRole:
            emit q->rejected();
            break;
        default:
            break;
        }
    }
}

void QDialogButtonBoxPrivate::_q_handleButtonDestroyed()
{
    Q_Q(QDialogButtonBox);
    if (QObject *object = q->sender()) {
        QBoolBlocker skippy(skipDisconnect);
        q->removeButton(static_cast<QAbstractButton *>(object));
    }
}

#include "moc_qdialogbuttonbox.cpp"
