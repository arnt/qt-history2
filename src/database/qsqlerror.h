#ifndef QSQLERROR_H
#define QSQLERROR_H

//#if !defined(Q_NO_SQL)

#include <qstring.h>

class QSqlError
{
public:
    enum Type {
	None,
	Connection,
	Statement,
	Transaction,
	Unknown
    };
    explicit QSqlError(  const QString& driverText = QString::null,
    		const QString& databaseText = QString::null,
		int type = QSqlError::None,
		int number = -1 );
    QSqlError(const QSqlError& n);
    QSqlError& operator=(const QSqlError& n);
    virtual ~QSqlError();
    QString 	driverText() const;
    void	setDriverText( const QString& driverText );
    QString 	databaseText() const;
    void	setDatabaseText( const QString& databaseText );
    int		type() const;
    void	setType( int type );
    int		number() const;
    void	setNumber( int number );
private:
    QString 	driverError;
    QString	databaseError;
    int		errorType;
    int 	errorNumber;
};

// #endif
#endif
