#include "qsqleditor.h"

#include "qlayout.h"
#include "qlineedit.h"
#include "qspinbox.h"

#ifndef QT_NO_SQL

/*!  Constructs an empty SQL editor.

*/

QSqlEditor::QSqlEditor( QSqlField& field, QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f ), fld(field)
{

}

/*! Destroys the object and frees any allocated resources.

*/

QSqlEditor::~QSqlEditor()
{

}

/*!
  Moves the contents of the QSqlField value into the editor.

*/

void QSqlEditor::syncToEditor()
{
    takeValue( fld.value() );
}


/*!
  Moves the contents of the editor into the QSqlField.

*/

void QSqlEditor::syncToField()
{
    fld.value() = editorValue();
}

///////////

/*!  Constructs a SQL line edit.

*/

QSqlLineEdit::QSqlLineEdit( QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field, parent, name )
{
    grid = new QGridLayout( this );
    ed = new QLineEdit( this, name );
    setSizePolicy( ed->sizePolicy() );
    setFocusProxy( ed );
    grid->addWidget( ed, 0, 0 );
}

/*!  Constructs a SQL line edit initialized to \a contents.

*/

QSqlLineEdit::QSqlLineEdit( const QString & contents, QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field, parent, name )
{
    grid = new QGridLayout( this );
    ed = new QLineEdit( contents, this, name );
    setSizePolicy( ed->sizePolicy() );
    setFocusProxy( ed );
    grid->addWidget( ed, 0, 0 );    
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlLineEdit::~QSqlLineEdit()
{
}


/*!
  Returns a pointer to the line edit.

*/

QLineEdit* QSqlLineEdit::lineEdit()
{
    return ed;
}

/*!
  \reimpl

*/

QVariant QSqlLineEdit::editorValue()
{
    return QVariant(ed->text());
}

/*!
  \reimpl

*/

void QSqlLineEdit::takeValue( QVariant& value )
{
    ed->setText( value.toString() );
}

/////////

/*!  Constructs a SQL spin box.

*/

QSqlSpinBox::QSqlSpinBox( QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field, parent, name )
{
    grid = new QGridLayout( this );
    ed = new QSpinBox( this, name );
    setSizePolicy( ed->sizePolicy() );
    setFocusProxy( ed );
    grid->addWidget( ed, 0, 0 );    
}

/*!  Constructs a SQL spin box

*/

QSqlSpinBox::QSqlSpinBox( int minValue, int maxValue, int step, QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field, parent, name )
{
    grid = new QGridLayout( this );
    ed = new QSpinBox( minValue, maxValue, step, this, name );
    setSizePolicy( ed->sizePolicy() );
    setFocusProxy( ed );
    grid->addWidget( ed, 0, 0 );    
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlSpinBox::~QSqlSpinBox()
{
    
}

/*!
  Returns a pointer to the spin box.

*/

QSpinBox* QSqlSpinBox::spinBox()
{
    return ed;
}

/*!
  \reimpl
*/

QVariant QSqlSpinBox::editorValue()
{
    return QVariant( ed->cleanText().toInt() );
}

/*!
  \reimpl

*/

void QSqlSpinBox::takeValue( QVariant& value )
{
    ed->setValue( value.toInt() );
}


#endif
