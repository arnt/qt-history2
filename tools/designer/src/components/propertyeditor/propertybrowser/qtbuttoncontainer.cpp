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

#include "qtbuttoncontainer.h"
#include <QToolButton>
#include <QVBoxLayout>
#include <QStyle>

class QtButtonContainerPrivate
{
    QtButtonContainer *q_ptr;
    Q_DECLARE_PUBLIC(QtButtonContainer)
public:

    QToolButton *m_button;
    QWidget *m_container;
    bool m_containerVisible;

    void slotToggled(bool checked);

    QVBoxLayout *m_layout;
};

void QtButtonContainerPrivate::slotToggled(bool checked)
{
    m_containerVisible = checked;
    if (m_container)
        m_container->setVisible(checked);
    emit q_ptr->containerVisibilityChanged(checked);
}

QtButtonContainer::QtButtonContainer(QWidget *parent)
    : QWidget(parent)
{
    d_ptr = new QtButtonContainerPrivate();
    d_ptr->q_ptr = this;

    d_ptr->m_containerVisible = false;
    d_ptr->m_layout = new QVBoxLayout(this);
    d_ptr->m_button = new QToolButton(this);
    d_ptr->m_button->setCheckable(true);
    d_ptr->m_button->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    d_ptr->m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QIcon icon;
    icon.addPixmap(style()->standardPixmap(QStyle::SP_ArrowDown), QIcon::Normal, QIcon::Off);
    icon.addPixmap(style()->standardPixmap(QStyle::SP_ArrowUp), QIcon::Normal, QIcon::On);
    d_ptr->m_button->setIcon(icon);
    d_ptr->m_layout->addWidget(d_ptr->m_button);
    d_ptr->m_layout->setMargin(0);
    d_ptr->m_layout->setSpacing(0);
    d_ptr->m_container = 0;

    connect(d_ptr->m_button, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
}

QtButtonContainer::~QtButtonContainer()
{
    delete d_ptr;
}

QString QtButtonContainer::title() const
{
    return d_ptr->m_button->text();
}

void QtButtonContainer::setTitle(const QString &text)
{
    d_ptr->m_button->setText(text);
}

QIcon QtButtonContainer::icon() const
{
    return d_ptr->m_button->icon();
}

void QtButtonContainer::setIcon(const QIcon &icon)
{
    d_ptr->m_button->setIcon(icon);
}

QWidget *QtButtonContainer::container() const
{
    return d_ptr->m_container;
}

void QtButtonContainer::setContainer(QWidget *container)
{
    if (d_ptr->m_container == container)
        return;

    bool updatesEna = updatesEnabled();
    setUpdatesEnabled(false);

    if (d_ptr->m_container) {
        d_ptr->m_container->setParent(0);
    }

    d_ptr->m_container = container;

    if (d_ptr->m_container) {
        d_ptr->m_layout->addWidget(d_ptr->m_container);
        d_ptr->m_container->setVisible(d_ptr->m_containerVisible);
    }

    setUpdatesEnabled(updatesEna);
}

bool QtButtonContainer::isContainerVisible() const
{
    return d_ptr->m_containerVisible;
}

void QtButtonContainer::setContainerVisible(bool visible)
{
    if (d_ptr->m_containerVisible == visible)
        return;

    d_ptr->m_button->setChecked(visible);
    d_ptr->m_containerVisible = visible;
    d_ptr->m_container->setVisible(visible);
}

#include "moc_qtbuttoncontainer.cpp"
