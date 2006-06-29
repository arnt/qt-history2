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

#include <QtCore/QHash>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStyle>
#include <QtGui/QVBoxLayout>
#include <QtGui/private/qwidget_p.h>

#include "qdialogbuttonbox.h"

/*!
    \class QDialogButtonBox
    \since 4.2
    \brief The QDialogButtonBox class is a widget that presents buttons in a
    layout that is appropriate to the current widget style.

    \ingroup application

    Dialogs and message boxes typically present buttons in a layout that
    conforms to the interface guidelines for that platform. Invariably,
    different platforms have different layouts for their dialogs.
    QDialogButtonBox allows a developer to add buttons to it and will
    automatically use the appropriate layout for the user's desktop
    environment.

    Most buttons for a dialog follow certain roles. Such roles include:

    \list
    \o Accepting or rejecting the dialog.
    \o Asking for help.
    \o Performing actions on the dialog itself (such as resetting fields or
       applying changes).
    \endlist

    There can also be alternate ways of dismissing the dialog which may cause
    destructive results.

    Most dialogs have buttons that can almost be considered standard (e.g.
    \gui OK and \gui Cancel buttons). It is sometimes convenient to create these
    buttons in a standard way.

    There are a couple ways of using QDialogButtonBox. One ways is to create
    the buttons (or button texts) yourself and add them to the button box,
    specifying their role.

    \quotefromfile dialogs/extension/finddialog.cpp
    \skipto findButton
    \printuntil buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);

    Alternatively, QDialogButtonBox provides several standard buttons (e.g. OK, Cancel, Save)
    that you can use. They exist as flags so you can OR them together in the constructor.

    \quotefromfile dialogs/tabdialog/tabdialog.cpp
    \skipto buttonBox
    \printuntil connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    You can mix and match normal buttons and standard buttons.

    Currently the buttons are laid out in the following way if the button box is horizontal:
    \table 100%
    \row \o \inlineimage buttonbox-gnomelayout-horizontal.png GnomeLayout Horizontal
         \o Button box laid out in horizontal GnomeLayout
    \row \o \inlineimage buttonbox-kdelayout-horizontal.png KdeLayout Horizontal
         \o Button box laid out in horizontal KdeLayout
    \row \o \inlineimage buttonbox-maclayout-horizontal.png MacLayout Horizontal
         \o Button box laid out in horizontal MacLayout
    \row \o \inlineimage buttonbox-winlayout-horizontal.png  WinLayout Horizontal
         \o Button box laid out in horizontal WinLayout
    \endtable

    The buttons are laid out the following way if the button box is vertical:

    \table 100%
    \row \o \inlineimage buttonbox-gnomelayout-vertical.png GnomeLayout Vertical
         \o Button box laid out in vertical GnomeLayout
    \row \o \inlineimage buttonbox-kdelayout-vertical.png KdeLayout Vertical
         \o Button box laid out in vertical KdeLayout
    \row \o \inlineimage buttonbox-maclayout-vertical.png MacLayout Vertical
         \o Button box laid out in vertical MacLayout
    \row \o \inlineimage buttonbox-winlayout-vertical.png WinLayout Vertical
         \o Button box laid out in vertical WinLayout
    \endtable

    Additionally, button boxes that contain only buttons with ActionRole or
    HelpRole can be considered modeless and have an alternate look on the mac:

    \table 100%
    \row \o \inlineimage buttonbox-mac-modeless-horizontal.png Screenshot of modeless horizontal MacLayout
         \o modeless horizontal MacLayout
    \row \o \inlineimage buttonbox-mac-modeless-vertical.png Screenshot of modeless vertical MacLayout
         \o modeless vertical MacLayout
    \endtable

    When a button is clicked in the button box, the clicked() signal is emitted
    for the actual button is that is pressed. For convenience, if the button
    has an AcceptRole, RejectRole, or HelpRole, the accepted(), rejected(), or
    helpRequested() signals are emitted respectively.

    \sa QMessageBoxEx, QPushButton, QDialog
*/

enum { AcceptRole = QDialogButtonBox::AcceptRole, RejectRole = QDialogButtonBox::RejectRole,
       DestructiveRole = QDialogButtonBox::DestructiveRole,
       ActionRole = QDialogButtonBox::ActionRole, HelpRole = QDialogButtonBox::HelpRole, AlternateRole,
       Stretch = 0x10000000, MacSpacer = 0x20000000, EOL = 0x40000000, Reverse = 0x80000000 };

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
    case QDialogButtonBox::Discard:
        return QDialogButtonBox::DestructiveRole;
    case QDialogButtonBox::Help:
        return QDialogButtonBox::HelpRole;
    case QDialogButtonBox::Apply:
        return QDialogButtonBox::ActionRole;
    case QDialogButtonBox::Reset:
        return QDialogButtonBox::ActionRole;
    case QDialogButtonBox::Yes:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::YesToAll:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::No:
        return QDialogButtonBox::RejectRole;
    case QDialogButtonBox::NoToAll:
        return QDialogButtonBox::RejectRole;
    case QDialogButtonBox::SaveAll:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Abort:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Retry:
        return QDialogButtonBox::AcceptRole;
    case QDialogButtonBox::Ignore:
        return QDialogButtonBox::RejectRole;
    default:
        return QDialogButtonBox::InvalidRole;
    }
}

static const int layouts[2][5][9] =
{
    // Qt::Horizontal
    {
        // WinLayout
        { Stretch, AcceptRole, AlternateRole, DestructiveRole, RejectRole, ActionRole, HelpRole, EOL, EOL },

        // MacLayout
        { HelpRole, ActionRole, Stretch, DestructiveRole | Reverse, MacSpacer, AlternateRole | Reverse, RejectRole | Reverse, AcceptRole | Reverse, EOL },

        // KdeLayout
        { HelpRole, Stretch, AcceptRole, AlternateRole, ActionRole, DestructiveRole, RejectRole, EOL, EOL },

        // GnomeLayout
        { HelpRole, Stretch, ActionRole | Reverse, DestructiveRole | Reverse, AlternateRole | Reverse, RejectRole | Reverse, AcceptRole | Reverse, EOL, EOL },

        // Mac modeless
        { ActionRole, Stretch, HelpRole, EOL, EOL, EOL, EOL, EOL, EOL }
    },

    // Qt::Vertical
    {
        // WinLayout
        { ActionRole, AcceptRole, AlternateRole, DestructiveRole, RejectRole, HelpRole, Stretch, EOL, EOL },

        // MacLayout
        { AcceptRole, RejectRole, AlternateRole, MacSpacer, DestructiveRole, Stretch, ActionRole, HelpRole, EOL },

        // KdeLayout
        { ActionRole, Stretch, AcceptRole, AlternateRole, DestructiveRole, RejectRole, HelpRole, EOL, EOL },

        // GnomeLayout
        { AcceptRole, RejectRole, AlternateRole, DestructiveRole, ActionRole, Stretch, HelpRole, EOL, EOL },

        // Mac modeless
        { ActionRole, Stretch, HelpRole, EOL, EOL, EOL, EOL, EOL, EOL }
    }
};

class QDialogButtonBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDialogButtonBox)
public:
    QDialogButtonBoxPrivate(Qt::Orientation orient);
    ~QDialogButtonBoxPrivate();

    QList<QAbstractButton *> buttonLists[QDialogButtonBox::NRoles];
    QHash<QPushButton *, QDialogButtonBox::StandardButton> standardButtonHash;

    Qt::Orientation orientation;
    QDialogButtonBox::ButtonLayout layoutPolicy;
    QBoxLayout *buttonLayout;
    bool skipDisconnect;

    void createStandardButtons(QDialogButtonBox::StandardButtons buttons);

    void layoutButtons();
    void initLayout();
    void resetLayout();
    QPushButton *createButton(QDialogButtonBox::StandardButton button, bool doLayout = true);
    void addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role, bool doLayout = true);
    void _q_handleButtonDestroyed();
    void _q_handleButtonClicked();
    void addButtonsToLayout(int start, int stop, int dir, const QList<QAbstractButton *> buttonList);
};

QDialogButtonBoxPrivate::QDialogButtonBoxPrivate(Qt::Orientation orient)
    : orientation(orient), skipDisconnect(false)
{
}

QDialogButtonBoxPrivate::~QDialogButtonBoxPrivate()
{
    // Just clear the lists, since I can't really guarantee when those other things will be deleted
    standardButtonHash.clear();
    for (int i = 0; i < QDialogButtonBox::NRoles; ++i)
        buttonLists[i].clear();
}

void QDialogButtonBoxPrivate::initLayout()
{
    Q_Q(QDialogButtonBox);
    layoutPolicy = QDialogButtonBox::ButtonLayout(q->style()->styleHint(QStyle::SH_DialogButtonLayout));
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

void QDialogButtonBoxPrivate::resetLayout()
{
    delete buttonLayout;
    initLayout();
    layoutButtons();
}

void QDialogButtonBoxPrivate::addButtonsToLayout(int start, int stop, int dir,
                                                 const QList<QAbstractButton *> buttonList)
{
    while (start != stop) {
        QAbstractButton *button = buttonList.at(start);
        buttonLayout->addWidget(button);
        button->show();
        start += dir;
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

    int tmpPolicy = layoutPolicy;

    static const int M = 3;
    static int ModalRoles[M] = { AcceptRole, RejectRole, DestructiveRole };
    if (tmpPolicy == QDialogButtonBox::MacLayout) {
        bool hasModalButton = false;
        for (int i = 0; i < M; ++i)
            if (!buttonLists[ModalRoles[i]].isEmpty()) {
                hasModalButton = true;
                break;
            }
        if (!hasModalButton)
            tmpPolicy = 4;  // Mac modeless
    }


    const int *currentLayout = layouts[orientation == Qt::Vertical][tmpPolicy];

    while (*currentLayout != EOL) {
        switch (*currentLayout & ~Reverse) {
        case Stretch:
            buttonLayout->addStretch();
            break;
        case MacSpacer:
            buttonLayout->addSpacing(19);
            break;
        case AcceptRole: {
            const QList<QAbstractButton *> &list = buttonLists[AcceptRole];
            if (list.isEmpty())
                break;
            // Only the first one
            QAbstractButton *button = list.at(0);
            buttonLayout->addWidget(button);
            button->show();
        }
            break;
        case AlternateRole: {
            const QList<QAbstractButton *> &list = buttonLists[AcceptRole];
            if (list.size() < 2)
                break;
            int start,
                stop,
                dir;
            if (*currentLayout & Reverse) {
                start = list.size() - 1;
                stop = 0;
                dir = -1;
            } else {
                start = dir = 1;
                stop = list.size();
            }
            addButtonsToLayout(start, stop, dir, list);
        }
            break;
        case RejectRole:
        case DestructiveRole:
        case ActionRole:
        case HelpRole: {
            const QList<QAbstractButton *> &list = buttonLists[*currentLayout & ~Reverse];
            int start,
                stop,
                dir;
            if (*currentLayout & Reverse) {
                start = list.size() - 1;
                stop = dir = -1;
            } else {
                start = 0;
                stop = list.size();
                dir = 1;
            }
            addButtonsToLayout(start, stop, dir, list);
        }
            break;
        default:
            break;
        }
        ++currentLayout;
    }
}

QPushButton *QDialogButtonBoxPrivate::createButton(QDialogButtonBox::StandardButton sbutton,
                                                   bool doLayout)
{
    Q_Q(QDialogButtonBox);
    QString buttonText;
    int icon = 0;

    switch (sbutton) {
    case QDialogButtonBox::Ok:
        icon = QStyle::SP_StandardButtonOk;
        buttonText = QDialogButtonBox::tr("OK"); // Well, at least I can kill "Ok" buttons.
        break;
    case QDialogButtonBox::Save:
        icon = QStyle::SP_StandardButtonSave;
        buttonText = QDialogButtonBox::tr("Save");
        break;
    case QDialogButtonBox::Open:
        icon = QStyle::SP_StandardButtonOpen;
        buttonText = QDialogButtonBox::tr("Open");
        break;
    case QDialogButtonBox::Cancel:
        icon = QStyle::SP_StandardButtonCancel;
        buttonText = QDialogButtonBox::tr("Cancel");
        break;
    case QDialogButtonBox::Close:
        icon = QStyle::SP_StandardButtonClose;
        buttonText = QDialogButtonBox::tr("Close");
        break;
    case QDialogButtonBox::Apply:
        icon = QStyle::SP_StandardButtonApply;
        buttonText = QDialogButtonBox::tr("Apply");
        break;
    case QDialogButtonBox::Reset:
        icon = QStyle::SP_StandardButtonReset;
        buttonText = QDialogButtonBox::tr("Reset");
        break;
    case QDialogButtonBox::Help:
        icon = QStyle::SP_StandardButtonHelp;
        buttonText = QDialogButtonBox::tr("Help");
        break;
    case QDialogButtonBox::Discard:
        icon = QStyle::SP_StandardButtonDiscard;
        if (layoutPolicy == QDialogButtonBox::MacLayout)
            buttonText = QDialogButtonBox::tr("Don't Save");
        else
            buttonText = QDialogButtonBox::tr("Discard");
        break;
    case QDialogButtonBox::Yes:
        icon = QStyle::SP_StandardButtonYes;
        buttonText = QDialogButtonBox::tr("&Yes");
        break;
    case QDialogButtonBox::YesToAll:
        buttonText = QDialogButtonBox::tr("Yes to &All");
        break;
    case QDialogButtonBox::No:
        icon = QStyle::SP_StandardButtonNo;
        buttonText = QDialogButtonBox::tr("&No");
        break;
    case QDialogButtonBox::NoToAll:
        buttonText = QDialogButtonBox::tr("N&o to All");
        break;
    case QDialogButtonBox::SaveAll:
        buttonText = QDialogButtonBox::tr("Save All");
        break;
    case QDialogButtonBox::Abort:
        buttonText = QDialogButtonBox::tr("Abort");
        break;
    case QDialogButtonBox::Retry:
        buttonText = QDialogButtonBox::tr("Retry");
        break;
    case QDialogButtonBox::Ignore:
        buttonText = QDialogButtonBox::tr("Ignore");
        break;
    default:
        icon = 0;
        break;
    }

    if (buttonText.isEmpty())
        return 0;

    QPushButton *button = new QPushButton(buttonText, q);
    if (q->style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons, 0, q) && icon != 0)
        button->setIcon(q->style()->standardIcon(QStyle::StandardPixmap(icon), 0, q));
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
    buttonLists[role].append(button);
    if (doLayout)
        layoutButtons();
}

void QDialogButtonBoxPrivate::createStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
    uint i = QDialogButtonBox::Ok;
    while (i <= QDialogButtonBox::Reset) {
        if (i & buttons) {
            createButton(QDialogButtonBox::StandardButton(i), false);
        }
        i = i << 1;
    }
    layoutButtons();
}

/*!
    Constructs an empty, horizontal button box with the given \a parent.

    \sa orientation, addButton()
*/
QDialogButtonBox::QDialogButtonBox(QWidget *parent)
    : QWidget(*new QDialogButtonBoxPrivate(Qt::Horizontal), parent, 0)
{
    d_func()->initLayout();
}

/*!
    Constructs an empty button box with the given \a orientation and \a parent.

    \sa orientation, addButton()
*/
QDialogButtonBox::QDialogButtonBox(Qt::Orientation orientation, QWidget *parent)
    : QWidget(*new QDialogButtonBoxPrivate(orientation), parent, 0)
{
    d_func()->initLayout();
}

/*!
    Constructs a button box with the given \a orientation and \a parent, containing
    the standard buttons specified by \a buttons.

    \sa orientation, addButton()
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
    \enum QDialogButtonBox::ButtonRole

    This enum describes the roles that can be used to describe buttons in
    the button box. Combinations of these roles are as flags used to
    describe different aspects of their behavior.

    \value InvalidRole The button is invalid.
    \value AcceptRole Clicking the button causes the dialog to be accepted
           (e.g. OK).
    \value RejectRole Clicking the button causes the dialog to be rejected
           (e.g. Cancel).
    \value DestructiveRole Clicking the button causes a destructive change
           (e.g. for Discarding Changes).
    \value ActionRole Clicking the button causes changes to the elements in
           the dialog (e.g. reset all the values or read defaults).
    \value HelpRole The button can be clicked to request help.
    \omitvalue NRoles
*/

/*!
    \enum QDialogButtonBox::StandardButton

    These enums describe flags for standard buttons. Each button has a
    defined \l ButtonRole.

    \value Ok An "OK" button defined with the \l AcceptRole.
    \value Open A "Open" button defined with the \l AcceptRole.
    \value Save A "Save" button defined with the \l AcceptRole.
    \value Cancel A "Cancel" button defined with the \l RejectRole.
    \value Close A "Close" button defined with the \l RejectRole.
    \value Discard A "Discard" or "Don't Save" button, depending on the platform,
                    defined with the \l DestructiveRole.
    \value Apply An "Apply" button defined with the \l ActionRole.
    \value Reset A "Reset" button defined with the \l ActionRole.
    \value Help A "Help" button defined with the \l HelpRole.
    \value SaveAll A "Save All" button defined with the \l AcceptRole
    \value Yes A "Yes" button defined with the \l AcceptRole
    \value YesToAll A "Yes to All" button defined with the \l AcceptRole
    \value No A "No" button defined with the \l RejectRole
    \value NoToAll A "No to All" button defined with the \l RejectRole
    \value Abort An "Abort" button defined with the \l RejectRole
    \value Retry A "Retry" button defined with the \l AcceptRole
    \value Ignore An "Ignore" button defined with the \l AcceptRole
    \omitvalue NoButtons
*/

/*!
    \enum QDialogButtonBox::ButtonLayout

    This enum describes the layout policy to be used when arranging the buttons
    contained in the button box.

    \value WinLayout Use a policy appropriate for applications on Windows.
    \value MacLayout Use a policy appropriate for applications on Mac OS X.
    \value KdeLayout Use a policy appropriate for applications on KDE.
    \value GnomeLayout Use a policy appropriate for applications on GNOME.
*/

/*!
    \fn void QDialogButtonBox::clicked(QAbstractButton *button)

    This signal is emitted when a button inside the button box is clicked. The
    specific button that was pressed is specified by \a button.

    \sa accepted(), rejected() helpRequested()
*/

/*!
    \fn void QDialogButtonBox::accepted()

    This signal is emitted when a button inside the button box is clicked, as long
    as it was defined with the \l AcceptRole.

    \sa rejected(), clicked() helpRequested()
*/

/*!
    \fn void QDialogButtonBox::rejected()

    This signal is emitted when a button inside the button box is clicked, as long
    as it was defined with the \l RejectRole.

    \sa accepted() helpRequested() clicked()
*/

/*!
    \fn void QDialogButtonBox::helpRequested()

    This signal is emitted when a button inside the button box is clicked, as long
    as it was defined with the \l HelpRole.

    \sa accepted() rejected() clicked()
*/

/*!
    \property QDialogButtonBox::orientation
    \brief the orientation of the button box

    By default, the orientation is horizontal (i.e. the buttons are laid out
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
    d->resetLayout();
}

/*!
    Clears the button box, deleting all buttons within it.

    \sa removeButton(), addButton()
*/
void QDialogButtonBox::clear()
{
    Q_D(QDialogButtonBox);
    // Remove the created standard buttons, they should be in the other lists, which will
    // do the deletion
    d->standardButtonHash.clear();
    for (int i = 0; i < NRoles; ++i) {
        QList<QAbstractButton *> &list = d->buttonLists[i];
        while (list.count()) {
            QAbstractButton *button = list.takeAt(0);
            QObject::disconnect(button, SIGNAL(destroyed()), this, SLOT(_q_handleButtonDestroyed()));
            delete button;
        }
    }
}

/*!
    Returns a list of all the buttons that have been added to the button box.

    \sa buttonRole(), addButton(), removeButton()

*/
QList<QAbstractButton *> QDialogButtonBox::buttons() const
{
    Q_D(const QDialogButtonBox);
    QList<QAbstractButton *> finalList;
    for (int i = 0; i < NRoles; ++i) {
        const QList<QAbstractButton *> &list = d->buttonLists[i];
        for (int j = 0; j < list.count(); ++j)
            finalList.append(list.at(j));
    }
    return finalList;
}

/*!
    Returns the button role for the specified \a button. This function returns
    \l InvalidRole if \a button is 0 or has not been added to the button box.

    \sa buttons(), addButton()
*/
QDialogButtonBox::ButtonRole QDialogButtonBox::buttonRole(QAbstractButton *button) const
{
    Q_D(const QDialogButtonBox);
    for (int i = 0; i < NRoles; ++i) {
        const QList<QAbstractButton *> &list = d->buttonLists[i];
        for (int j = 0; j < list.count(); ++j) {
            if (list.at(j) == button)
                return ButtonRole(i);
        }
    }
    return InvalidRole;
}

/*!
    Removes \a button from the button box without deleting it. This is
    typically done when you want to change the parent of a button.

    \sa clear(), buttons(), addButton()
*/
void QDialogButtonBox::removeButton(QAbstractButton *button)
{
    Q_D(QDialogButtonBox);
    // Remove it from the standard button hash first and then from the roles
    if (QPushButton *pushButton = qobject_cast<QPushButton *>(button))
        d->standardButtonHash.remove(pushButton);
    for (int i = 0; i < NRoles; ++i) {
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
    Adds the given \a button to the button box with the specified \a role.
    If the role is invalid, the button is not added.

    If the button has already been added, it is removed and added again with the
    new role.

    \sa removeButton(), clear()
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
    Creates a push button with the given \a text, adds it to the button box for the
    specified \a role, and returns the corresponding push button. If \a role is
    invalid, no button is created, and zero is returned.

    \sa removeButton(), clear()
*/
QPushButton *QDialogButtonBox::addButton(const QString &text, ButtonRole role)
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
    Adds a standard \a button to the button box if it is valid to do so, and returns
    a push button. If \a button is invalid, it is not added to the button box, and
    zero is returned.

    \sa removeButton(), clear()
*/
QPushButton *QDialogButtonBox::addButton(StandardButton button)
{
    Q_D(QDialogButtonBox);
    return d->createButton(button);
}

/*!
    \property QDialogButtonBox::standardButtons
    \brief collection of standard buttons in the button box

    This property controls which standard buttons are used by the button box.

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
    StandardButtons standardButtons = NoButton;
    QHash<QPushButton *, StandardButton>::const_iterator it = d->standardButtonHash.constBegin();
    while (it != d->standardButtonHash.constEnd()) {
        standardButtons |= it.value();
        ++it;
    }
    return standardButtons;
}

/*!
    Returns a pointer corresponding to the standard button \a which,
    or 0 if the standard button doesn't exist in this button box.

    If there are several occurrences of \a which in the box (which
    happens if you call addButton(\a which) several times), it is not
    specified which button is returned.

    \sa standardButtons, buttons()
*/
QPushButton *QDialogButtonBox::button(StandardButton which) const
{
    Q_D(const QDialogButtonBox);
    return d->standardButtonHash.key(which);
}

void QDialogButtonBoxPrivate::_q_handleButtonClicked()
{
    Q_Q(QDialogButtonBox);
    if (QAbstractButton *button = qobject_cast<QAbstractButton *>(q->sender())) {
        emit q->clicked(button);
        switch (q->buttonRole(button)) {
        case AcceptRole:
            emit q->accepted();
            break;
        case RejectRole:
            emit q->rejected();
            break;
        case HelpRole:
            emit q->helpRequested();
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

/*!
    \reimp
*/
void QDialogButtonBox::changeEvent(QEvent *event)
{
    Q_D(QDialogButtonBox);

    switch (event->type()) {
    case QEvent::StyleChange:
        d->resetLayout();
        QWidget::changeEvent(event);
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

/*!
    \reimp
*/
bool QDialogButtonBox::event(QEvent *event)
{
    return QWidget::event(event);
}

#include "moc_qdialogbuttonbox.cpp"
