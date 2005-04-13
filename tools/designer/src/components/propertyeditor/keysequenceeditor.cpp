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

#include "keysequenceeditor.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>
#include <QPixmap>
#include <QIconSet>
#include <QHBoxLayout>

KeySequenceEditor::KeySequenceEditor(QWidget *parent)
    : QWidget(parent),
      mouseEnter(true)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    m_lineEdit = new QLineEdit(this);
    hbox->addWidget(m_lineEdit);
    m_lineEdit->installEventFilter(this);
    m_resetButton = new QToolButton(this);
    hbox->addWidget(m_resetButton);
    m_resetButton->setIcon(QIcon(QString::fromUtf8(":/trolltech/formeditor/images/resetproperty.png")));

    setFocusProxy(m_lineEdit);
    m_resetButton->setFocusPolicy(Qt::NoFocus);

    reset();

    connect(m_lineEdit, SIGNAL(textChanged(QString)),
        this, SIGNAL(changed()));
    connect(m_resetButton, SIGNAL(clicked()),
        this, SLOT(reset()));
}

KeySequenceEditor::~KeySequenceEditor()
{
}

bool KeySequenceEditor::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *k = static_cast<QKeyEvent*>(e);
        if ( !mouseEnter &&
            (k->key() == Qt::Key_Up ||
             k->key() == Qt::Key_Down) )
            return false;
        handleKeyEvent( k );
        return true;
    } else if ( (e->type() == QEvent::FocusIn) ||
                (e->type() == QEvent::MouseButtonPress) ) {
//        mouseEnter = ( listview->lastEvent() == QEvent::MouseEvent ) ||
//                     (e->type() == QEvent::MouseButtonPress);
        return true;
    }

    // Lets eat accelerators now..
    if ( e->type() == QEvent::Shortcut ||
         e->type() == QEvent::ShortcutOverride  ||
         e->type() == QEvent::KeyRelease )
        return true;

    return false;
}

void KeySequenceEditor::handleKeyEvent(QKeyEvent *e)
{
    int nextKey = e->key();

    if ( num > 3 ||
         nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift ||
         nextKey == Qt::Key_Meta ||
         nextKey == Qt::Key_Alt )
         return;

    nextKey |= translateModifiers(e->modifiers());
    switch( num ) {
        case 0:
            k1 = nextKey;
            break;
        case 1:
            k2 = nextKey;
            break;
        case 2:
            k3 = nextKey;
            break;
        case 3:
            k4 = nextKey;
            break;
        default:
            break;
    }
    num++;
    QKeySequence ks( k1, k2, k3, k4 );
    lineEdit()->setText( ks );
    e->accept();
}

QKeySequence KeySequenceEditor::keySequence() const
{
    return QKeySequence(k1, k2, k3, k4);
}

void KeySequenceEditor::setKeySequence(const QKeySequence &ks)
{
    num = ks.count();
    k1 = ks[0];
    k2 = ks[1];
    k3 = ks[2];
    k4 = ks[3];
    lineEdit()->setText(keySequence());
}

int KeySequenceEditor::translateModifiers(Qt::KeyboardModifiers modifier)
{
    int result = 0;
    if (modifier & Qt::ShiftModifier)
        result |= Qt::SHIFT;
    if (modifier & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (modifier & Qt::MetaModifier)
        result |= Qt::META;
    if (modifier & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

QToolButton *KeySequenceEditor::resetButton() const
{
    return m_resetButton;
}

QLineEdit *KeySequenceEditor::lineEdit() const
{
    return m_lineEdit;
}

void KeySequenceEditor::reset()
{
    setKeySequence(QKeySequence());
}
