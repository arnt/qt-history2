#include "qsqleditor.h"

#ifndef QT_NO_SQL


/*!  Constructs a SQL editor that operates on field \a field.

*/

QSqlEditor::QSqlEditor( QSqlField& field )
    : fld(field)
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
    : QSqlEditor( field ), QLineEdit( parent, name )
{
    
}

/*!  Constructs a SQL line edit

*/

QSqlLineEdit::QSqlLineEdit( const QString & contents, QWidget * parent, QSqlField& field, const char * name )
    : QSqlEditor( field ), QLineEdit( contents, parent, name )    
{
    
}

/*! 
  \reimpl

*/

QVariant QSqlLineEdit::editorValue()
{
    return QVariant(text());
}

/*! 
  \reimpl

*/

void QSqlLineEdit::takeValue( QVariant& value )
{
    setText( value.toString() );
}

#endif
