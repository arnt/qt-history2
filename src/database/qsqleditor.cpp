#include "qsqleditor.h"

#ifndef QT_NO_SQL

/*!  Constructs an empty

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
    ed = new QLineEdit( this, name );
}

/*!  Constructs a SQL line edit

*/

QSqlLineEdit::QSqlLineEdit( const QString & contents, QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field, parent, name )
{
    ed = new QLineEdit( contents, this, name );
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

#endif


