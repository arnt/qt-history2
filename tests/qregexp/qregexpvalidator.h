/*
  qregexpvalidator.h
*/

#ifndef QT_H
#include "qregexp.h"
#include "qvalidator.h" // ### ### this will appear in validator.h
#endif

class Q_EXPORT QRegExpValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY( QRegExp regExp READ regExp WRITE setRegExp )

public:
    QRegExpValidator( QWidget *parent, const char *name = 0 );
    QRegExpValidator( const QRegExp& rx, QWidget *parent,
		      const char *name = 0 );
    ~QRegExpValidator();

    virtual QValidator::State validate( QString& input, int& pos ) const;

    void setRegExp( const QRegExp& rx );
    const QRegExp& regExp() const { return r; }

private:
    QRegExp r;

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRegExpValidator( const QRegExpValidator& );
    QRegExpValidator& operator=( const QRegExpValidator& );
#endif
};
