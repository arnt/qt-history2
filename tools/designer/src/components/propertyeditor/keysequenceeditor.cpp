
#include "keysequenceeditor.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>
#include <QPixmap>
#include <QIconSet>

KeySequenceEditor::KeySequenceEditor(QWidget *parent)
    : QHBoxWidget(parent),
      mouseEnter(true)
{
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->installEventFilter(this);
    m_resetButton = new QToolButton(this);
    m_resetButton->setIcon(QPixmap(":/trolltech/formeditor/images/designer_resetproperty.png"));
    
    setFocusProxy(m_lineEdit);
    m_resetButton->setFocusPolicy(Qt::NoFocus);
    
    reset();

    connect(m_lineEdit, SIGNAL(textChanged(const QString&)),
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

int KeySequenceEditor::translateModifiers(Qt::ButtonState state)
{
    int result = 0;
    if (state & Qt::ShiftButton)
        result |= Qt::SHIFT;
    if (state & Qt::ControlButton)
        result |= Qt::CTRL;
    if (state & Qt::MetaButton)
        result |= Qt::META;
    if (state & Qt::AltButton)
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

