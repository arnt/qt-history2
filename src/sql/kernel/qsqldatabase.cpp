/****************************************************************************
**
** Implementation of QSqlDatabase class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**

** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsqldatabase.h"

#ifndef QT_NO_SQL

#ifdef Q_OS_WIN32
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#endif

#ifdef QT_SQL_POSTGRES
#include "../drivers/psql/qsql_psql.h"
#endif
#ifdef QT_SQL_MYSQL
#include "../drivers/mysql/qsql_mysql.h"
#endif
#ifdef QT_SQL_ODBC
#include "../drivers/odbc/qsql_odbc.h"
#endif
#ifdef QT_SQL_OCI
#include "../drivers/oci/qsql_oci.h"
#endif
#ifdef QT_SQL_TDS
#include "../drivers/tds/qsql_tds.h"
#endif
#ifdef QT_SQL_DB2
#include "../drivers/db2/qsql_db2.h"
#endif
#ifdef QT_SQL_SQLITE
#include "../drivers/sqlite/qsql_sqlite.h"
#endif
#ifdef QT_SQL_IBASE
#include "../drivers/ibase/qsql_ibase.h"
#endif

#include "qcoreapplication.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverinterface_p.h"
#include "private/qpluginmanager_p.h"
#include "private/qsqlnulldriver_p.h"
#include <stdlib.h>

const char *QSqlDatabase::defaultConnection = "qt_sql_default_connection";

typedef QHash<QString, QSqlDriverCreatorBase*> DriverDict;
typedef QHash<QString, QSqlDatabase> ConnectionDict;

class QSqlDatabasePrivate
{
public:
    QSqlDatabasePrivate(QSqlDriver *dr = 0):
        driver(dr),
#ifndef QT_NO_COMPONENT
        plugIns(0),
#endif
        port(-1)
    {
        ref = 1;
    }
    QSqlDatabasePrivate(const QSqlDatabasePrivate &other);
    ~QSqlDatabasePrivate();
    void init(const QString& type);
    void copy(const QSqlDatabasePrivate *other);
    void disable();

    QAtomic ref;
    QSqlDriver* driver;
#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
#endif
    QString dbname;
    QString uname;
    QString pword;
    QString hname;
    QString drvName;
    int port;
    QString connOptions;

    static QSqlDatabasePrivate *shared_null();
    static QSqlDatabase database(const QString& name, bool open);
    static void addDatabase(const QSqlDatabase &db, const QString & name);
    static void removeDatabase(const QString& name);
    static DriverDict &driverDict();
    static ConnectionDict &dbDict();
    static void cleanConnections();
};

QSqlDatabasePrivate::QSqlDatabasePrivate(const QSqlDatabasePrivate &other)
{
    ref = 1;
    dbname = other.dbname;
    uname = other.uname;
    pword = other.pword;
    hname = other.hname;
    drvName = other.drvName;
    port = other.port;
    connOptions = other.connOptions;
    driver = other.driver;
#ifndef QT_NO_COMPONENT
    plugIns = other.plugIns;
#endif
}

QSqlDatabasePrivate::~QSqlDatabasePrivate()
{
    if (driver != shared_null()->driver) {
        delete driver;
#ifndef QT_NO_COMPONENT
        delete plugIns;
#endif
    }
}

void QSqlDatabasePrivate::cleanConnections()
{
    ConnectionDict &dict = dbDict();
    while (!dict.isEmpty())
        removeDatabase(dict.constBegin().key());
}

static bool qDriverDictInit = false;
static void cleanDriverDict()
{
    qDeleteAll(QSqlDatabasePrivate::driverDict());
    QSqlDatabasePrivate::cleanConnections();
    qDriverDictInit = false;
}

DriverDict &QSqlDatabasePrivate::driverDict()
{
    static DriverDict dict;
    if (!qDriverDictInit) {
        qDriverDictInit = true;
        qAddPostRoutine(cleanDriverDict);
    }
    return dict;
}

ConnectionDict &QSqlDatabasePrivate::dbDict()
{
    static ConnectionDict dict;
    if (!qDriverDictInit) {
        qDriverDictInit = true;
        qAddPostRoutine(cleanDriverDict);
    }
    return dict;
}

QSqlDatabasePrivate *QSqlDatabasePrivate::shared_null()
{
    static QSqlNullDriver dr;
    static QSqlDatabasePrivate n(&dr);
    return &n;
}

void QSqlDatabasePrivate::removeDatabase(const QString& name)
{
    if (!dbDict().contains(name))
        return;

    QSqlDatabase db = dbDict().take(name);
    if (db.d->ref != 1) {
        qWarning("QSqlDatabasePrivate::removeDatabase: connection '%s' is still in use, "
                 "all queries will cease to work.", name.local8Bit());
        db.d->disable();
    }
}

void QSqlDatabasePrivate::addDatabase(const QSqlDatabase &db, const QString & name)
{
    if (dbDict().contains(name)) {
        removeDatabase(name);
        qWarning("QSqlDatabasePrivate::addDatabase: duplicate connection name '%s', old "
                 "connection removed.", name.local8Bit());
    }
    dbDict().insert(name, db);
}

/*! \internal
 */
QSqlDatabase QSqlDatabasePrivate::database(const QString& name, bool open)
{
    QSqlDatabase db = dbDict().value(name);
    if (!db.isOpen() && open) {
        db.open();
        if (!db.isOpen())
            qWarning("QSqlDatabasePrivate::database: unable to open database: %s",
                     db.lastError().text().local8Bit());

    }
    return db;
}


/*! \internal
    Copies the connection data from \a other
*/
void QSqlDatabasePrivate::copy(const QSqlDatabasePrivate *other)
{
    dbname = other->dbname;
    uname = other->uname;
    pword = other->pword;
    hname = other->hname;
    drvName = other->drvName;
    port = other->port;
    connOptions = other->connOptions;
}

void QSqlDatabasePrivate::disable()
{
    if (driver != shared_null()->driver) {
        delete driver;
        driver = shared_null()->driver;
    }
}

/*!
    \class QSqlDatabase qsqldatabase.h
    \brief The QSqlDatabase class is used to create SQL database
    connections and to provide transaction handling.

    \ingroup database
    \mainclass
    \module sql

    Note that transaction handling is not supported by every SQL
    database. You can find out whether transactions are supported
    using QSqlDriver::hasFeature().

    The QSqlDatabase class provides an abstract interface for
    accessing many types of database backends. Database-specific
    drivers are used internally to actually access and manipulate
    data, (see QSqlDriver). Result set objects provide the interface
    for executing and manipulating SQL queries (see QSqlQuery).
*/

/*!
    Adds a database to the list of database connections using the
    driver \a type and the connection name \a connectionName.

    The database connection is referred to by \a connectionName. The
    newly added database connection is returned.

    If \a connectionName is not specified, the newly added database
    connection becomes the default database connection for the
    application, and subsequent calls to database() (without a
    database name parameter) will return a reference to it. If \a
    connectionName is given, use \link QSqlDatabase::database()
    database(connectionName)\endlink to retrieve a pointer to the
    database connection.

    \sa database() removeDatabase()
*/
QSqlDatabase QSqlDatabase::addDatabase(const QString& type, const QString& connectionName)
{
    QSqlDatabase db(type);
    QSqlDatabasePrivate::addDatabase(db, connectionName);
    return db;
}

/*!
    Returns the database connection called \a connectionName. The
    database connection must have been previously added with
    addDatabase(). If \a open is true (the default) and the database
    connection is not already open it is opened now. If no \a
    connectionName is specified the default connection is used. If \a
    connectionName does not exist in the list of databases, an invalid
    connection is returned.
*/

QSqlDatabase QSqlDatabase::database(const QString& connectionName, bool open)
{
    return QSqlDatabasePrivate::database(connectionName, open);
}

/*!
    Removes the database connection \a connectionName from the list of
    database connections.

    \warning There should be no open queries on the database
    connection when this function is called, otherwise a resource leak
    will occur.
*/

void QSqlDatabase::removeDatabase(const QString& connectionName)
{
    QSqlDatabasePrivate::removeDatabase(connectionName);
}

/*!
    Returns a list of all the available database drivers.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = QSqlDatabase::drivers();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/

QStringList QSqlDatabase::drivers()
{
    QStringList l;

#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
    plugIns = new QPluginManager<QSqlDriverFactoryInterface>(IID_QSqlDriverFactory,
                    QCoreApplication::libraryPaths(), "/sqldrivers");

    l = plugIns->featureList();
    delete plugIns;
#endif

    DriverDict dict = QSqlDatabasePrivate::driverDict();
    for (DriverDict::ConstIterator itd = dict.constBegin(); itd != dict.constEnd(); ++itd) {
        if (!l.contains(itd.key()))
            l << itd.key();
    }

#ifdef QT_SQL_POSTGRES
    if (!l.contains("QPSQL7"))
        l << "QPSQL7";
#endif
#ifdef QT_SQL_MYSQL
    if (!l.contains("QMYSQL3"))
        l << "QMYSQL3";
#endif
#ifdef QT_SQL_ODBC
    if (!l.contains("QODBC3"))
        l << "QODBC3";
#endif
#ifdef QT_SQL_OCI
    if (!l.contains("QOCI8"))
        l << "QOCI8";
#endif
#ifdef QT_SQL_TDS
    if (!l.contains("QTDS7"))
        l << "QTDS7";
#endif
#ifdef QT_SQL_DB2
    if (!l.contains("QDB2"))
        l << "QDB2";
#endif
#ifdef QT_SQL_SQLITE
    if (!l.contains("QSQLITE"))
        l << "QSQLITE";
#endif
#ifdef QT_SQL_IBASE
    if (!l.contains("QIBASE"))
        l << "QIBASE";
#endif

    return l;
}

/*!
    This function registers a new SQL driver called \a name, within
    the SQL framework. This is useful if you have a custom SQL driver
    and don't want to compile it as a plugin.

    Example usage:

    \code
    QSqlDatabase::registerSqlDriver("MYDRIVER", new QSqlDriverCreator<MyDatabaseDriver>);
    QSqlDatabase* db = QSqlDatabase::addDatabase("MYDRIVER");
    ...
    \endcode

    \warning The framework takes ownership of the \a creator pointer,
    so it should not be deleted.
*/
void QSqlDatabase::registerSqlDriver(const QString& name, QSqlDriverCreatorBase* creator)
{
    delete QSqlDatabasePrivate::driverDict().take(name);
    if (creator)
        QSqlDatabasePrivate::driverDict().insert(name, creator);
}

/*!
    Returns true if the list of database connections contains \a
    connectionName; otherwise returns false.
*/

bool QSqlDatabase::contains(const QString& connectionName)
{
    return QSqlDatabasePrivate::dbDict().contains(connectionName);
}

/*!
    \overload

    Creates a QSqlDatabase connection called \a name that uses the
    driver referred to by \a type, with the parent \a parent. If the
    \a type is not recognized, the database connection will have no
    functionality.

    The currently available drivers are:

    \table
    \header \i Driver Type \i Description
    \row \i QODBC3 \i ODBC Driver (includes Microsoft SQL Server)
    \row \i QOCI8 \i Oracle Call Interface Driver
    \row \i QPSQL7 \i PostgreSQL v6.x and v7.x Driver
    \row \i QTDS7 \i Sybase Adaptive Server
    \row \i QMYSQL3 \i MySQL Driver
    \row \i QDB2 \i IBM DB2, v7.1 and higher
    \row \i QSQLITE \i SQLite Driver
    \row \i QIBASE \i Borland Interbase Driver
    \endtable

    Additional third party drivers, including your own custom drivers,
    can be loaded dynamically.

    \sa registerSqlDriver()
*/

QSqlDatabase::QSqlDatabase(const QString &type)
{
    d = new QSqlDatabasePrivate();
    d->init(type);
}

/*!
    \overload

     Creates a database connection using the driver \a driver and with
     the parent \a parent.
*/

QSqlDatabase::QSqlDatabase(QSqlDriver *driver)
{
    d = new QSqlDatabasePrivate(driver);
}

/*!
    Creates an empty, invalid QSqlDatabase object. Note that you should
    never create QSqlDatabase objects on your own, but rather use
    addDatabase(), removeDatabase() and database() to get
    QSqlDatabase objects.
 */
QSqlDatabase::QSqlDatabase()
{
    d = QSqlDatabasePrivate::shared_null();
    ++d->ref;
}

/*!
  Creates a copy of \a other.
 */
QSqlDatabase::QSqlDatabase(const QSqlDatabase &other)
{
    d = other.d;
    ++d->ref;
}

/*!
  Assigns \a other to this object.
 */
QSqlDatabase &QSqlDatabase::operator=(const QSqlDatabase &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
  \internal

  Create the actual driver instance \a type.
*/

void QSqlDatabasePrivate::init(const QString& type)
{
    drvName = type;

    if (!driver) {

#ifdef QT_SQL_POSTGRES
        if (type == "QPSQL7")
            driver = new QPSQLDriver();
#endif

#ifdef QT_SQL_MYSQL
        if (type == "QMYSQL3")
            driver = new QMYSQLDriver();
#endif

#ifdef QT_SQL_ODBC
        if (type == "QODBC3")
            driver = new QODBCDriver();
#endif

#ifdef QT_SQL_OCI
        if (type == "QOCI8")
            driver = new QOCIDriver();
#endif

#ifdef QT_SQL_TDS
        if (type == "QTDS7")
            driver = new QTDSDriver();
#endif

#ifdef QT_SQL_DB2
        if (type == "QDB2")
            driver = new QDB2Driver();
#endif

#ifdef QT_SQL_SQLITE
        if (type == "QSQLITE")
            driver = new QSQLiteDriver();
#endif

#ifdef QT_SQL_IBASE
        if (type == "QIBASE")
            driver = new QIBaseDriver();
#endif

    }

    if (!driver) {
        DriverDict dict = QSqlDatabasePrivate::driverDict();
        for (DriverDict::ConstIterator it = dict.constBegin();
             it != dict.constEnd() && !driver; ++it) {
            if (type == it.key()) {
                driver = ((QSqlDriverCreatorBase*)(*it))->createObject();
            }
        }
    }

#ifndef QT_NO_COMPONENT
    if (!driver) {
        plugIns = new QPluginManager<QSqlDriverFactoryInterface>(IID_QSqlDriverFactory,
                            QCoreApplication::libraryPaths(), "/sqldrivers");

        QInterfacePtr<QSqlDriverFactoryInterface> iface = 0;
        plugIns->queryInterface(type, &iface);
        if(iface)
            driver = iface->create(type);
    }
#endif

    if (!driver) {

        qWarning("QSqlDatabase: %s driver not loaded", type.latin1());
        qWarning("QSqlDatabase: available drivers: %s", QSqlDatabase::drivers().join(" ").latin1());
        driver = shared_null()->driver;
    }
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlDatabase::~QSqlDatabase()
{
    if (!--d->ref) {
        close();
        delete d;
    }
}

/*!
    Executes a SQL statement (e.g. an \c INSERT, \c UPDATE or \c
    DELETE statement) on the database, and returns a QSqlQuery object.
    Use lastError() to retrieve error information. If \a query is
    empty, an empty, invalid query is returned and lastError()
    is not affected.

    \sa QSqlQuery lastError()
*/

QSqlQuery QSqlDatabase::exec(const QString & query) const
{
    QSqlQuery r = d->driver->createQuery();
    if (!query.isEmpty()) {
        r.exec(query);
        d->driver->setLastError(r.lastError());
    }
    return r;
}

/*!
    Opens the database connection using the current connection values.
    Returns true on success; otherwise returns false. Error
    information can be retrieved using the lastError() function.

    \sa lastError()
*/

bool QSqlDatabase::open()
{
    return d->driver->open(d->dbname, d->uname, d->pword, d->hname,
                            d->port, d->connOptions);
}

/*!
    \overload

    Opens the database connection using the given \a user name and \a
    password. Returns true on success; otherwise returns false. Error
    information can be retrieved using the lastError() function.

    This function does not store the password it is given. Instead,
    the password is passed directly to the driver for opening a
    connection and is then discarded.

    \sa lastError()
*/

bool QSqlDatabase::open(const QString& user, const QString& password)
{
    setUserName(user);
    return d->driver->open(d->dbname, user, password, d->hname,
                            d->port, d->connOptions);
}

/*!
    Closes the database connection, freeing any resources acquired.

    \sa removeDatabase()
*/

void QSqlDatabase::close()
{
    d->driver->close();
}

/*!
    Returns true if the database connection is currently open;
    otherwise returns false.
*/

bool QSqlDatabase::isOpen() const
{
    return d->driver->isOpen();
}

/*!
    Returns true if there was an error opening the database
    connection; otherwise returns false. Error information can be
    retrieved using the lastError() function.
*/

bool QSqlDatabase::isOpenError() const
{
    return d->driver->isOpenError();
}

/*!
    Begins a transaction on the database if the driver supports
    transactions. Returns true if the operation succeeded; otherwise
    returns false.

    \sa QSqlDriver::hasFeature() commit() rollback()
*/

bool QSqlDatabase::transaction()
{
    if (!d->driver->hasFeature(QSqlDriver::Transactions))
        return false;
    return d->driver->beginTransaction();
}

/*!
    Commits a transaction to the database if the driver supports
    transactions. Returns true if the operation succeeded; otherwise
    returns false.

    \sa QSqlDriver::hasFeature() rollback()
*/

bool QSqlDatabase::commit()
{
    if (!d->driver->hasFeature(QSqlDriver::Transactions))
        return false;
    return d->driver->commitTransaction();
}

/*!
    Rolls a transaction back on the database if the driver supports
    transactions. Returns true if the operation succeeded; otherwise
    returns false.

    \sa QSqlDriver::hasFeature() commit() transaction()
*/

bool QSqlDatabase::rollback()
{
    if (!d->driver->hasFeature(QSqlDriver::Transactions))
        return false;
    return d->driver->rollbackTransaction();
}

/*!
    \property QSqlDatabase::databaseName
    \brief the name of the database

    Note that the database name is the TNS Service Name for the QOCI8
    (Oracle) driver.

    For the QODBC3 driver it can either be a DSN, a DSN filename (the
    file must have a \c .dsn extension), or a connection string. MS
    Access users can for example use the following connection string
    to open a \c .mdb file directly, instead of having to create a DSN
    entry in the ODBC manager:

    \code
    ...
    db = QSqlDatabase::addDatabase("QODBC3");
    db->setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=myaccessfile.mdb");
    if (db->open()) {
        // success!
    }
    ...
    \endcode
    ("FIL" is the required spelling in Microsoft's API.)

    There is no default value.
*/

void QSqlDatabase::setDatabaseName(const QString& name)
{
    d->dbname = name;
}

/*!
    \property QSqlDatabase::userName
    \brief the user name connected to the database

    There is no default value.
*/

void QSqlDatabase::setUserName(const QString& name)
{
    d->uname = name;
}

/*!
    \property QSqlDatabase::password
    \brief the password used to connect to the database

    There is no default value.

    \warning This function stores the password in plain text within
    Qt. Use the open() call that takes a password as parameter to
    avoid this behavior.

    \sa open()
*/

void QSqlDatabase::setPassword(const QString& password)
{
    d->pword = password;
}

/*!
    \property QSqlDatabase::hostName
    \brief the host name where the database resides

    There is no default value.
*/

void QSqlDatabase::setHostName(const QString& host)
{
    d->hname = host;
}

/*!
    \property QSqlDatabase::port
    \brief the port used to connect to the database

    There is no default value.
*/

void QSqlDatabase::setPort(int p)
{
    d->port = p;
}

QString QSqlDatabase::databaseName() const
{
    return d->dbname;
}

QString QSqlDatabase::userName() const
{
    return d->uname;
}

QString QSqlDatabase::password() const
{
    return d->pword;
}

QString QSqlDatabase::hostName() const
{
    return d->hname;
}

/*!
    Returns the name of the driver used by the database connection.
*/
QString QSqlDatabase::driverName() const
{
    return d->drvName;
}

int QSqlDatabase::port() const
{
    return d->port;
}

/*!
    Returns the database driver used to access the database
    connection.
*/

QSqlDriver* QSqlDatabase::driver() const
{
    return d->driver;
}

/*!
    Returns information about the last error that occurred on the
    database. See QSqlError for more information.
*/

QSqlError QSqlDatabase::lastError() const
{
    return d->driver->lastError();
}


/*!
    Returns a list of the database's tables, system tables and views,
    as specified by the parameter \a type.
*/

QStringList QSqlDatabase::tables(QSql::TableType type) const
{
    return d->driver->tables(type);
}

/*!
    Returns the primary index for table \a tablename. If no primary
    index exists an empty QSqlIndex will be returned.
*/

QSqlIndex QSqlDatabase::primaryIndex(const QString& tablename) const
{
    return d->driver->primaryIndex(tablename);
}


/*!
    Returns a QSqlRecord populated with the names of all the fields in
    the table (or view) called \a tablename. The order in which the
    fields appear in the record is undefined. If no such table (or
    view) exists, an empty record is returned.
*/

QSqlRecord QSqlDatabase::record(const QString& tablename) const
{
    return d->driver->record(tablename);
}


/*! \fn QSqlRecord QSqlDatabase::record(const QSqlQuery& query) const
    \obsolete use QSqlQuery::record() instead

*/

/*! \fn QSqlRecord QSqlDatabase::recordInfo(const QString& tablename) const
    \obsolete use QSqlRecord::record instead
*/

/*! \fn QSqlRecord QSqlDatabase::recordInfo(const QSqlQuery& query) const
    \obsolete use QSqlQuery::record() instead
*/

/*!
    \property QSqlDatabase::connectOptions
    \brief the database connect options

    The format of the options string is a semi-colon separated list of
    option names or option = value pairs. The options depend on the
    database client used:

    \table
    \header \i ODBC \i MySQL \i PostgreSQL
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \i SQL_ATTR_CONNECTION_TIMEOUT
    \i SQL_ATTR_CURRENT_CATALOG
    \i SQL_ATTR_METADATA_ID
    \i SQL_ATTR_PACKET_SIZE
    \i SQL_ATTR_TRACEFILE
    \i SQL_ATTR_TRACE
    \i SQL_ATTR_CONNECTION_POOLING
    \endlist

    \i
    \list
    \i CLIENT_COMPRESS
    \i CLIENT_FOUND_ROWS
    \i CLIENT_IGNORE_SPACE
    \i CLIENT_SSL
    \i CLIENT_ODBC
    \i CLIENT_NO_SCHEMA
    \i CLIENT_INTERACTIVE
    \endlist

    \i
    \list
    \i connect_timeout
    \i options
    \i tty
    \i requiressl
    \i service
    \endlist

    \header \i DB2 \i OCI \i TDS
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \endlist

    \i
    \e none

    \i
    \e none

    \endtable

    Example of usage:
    \code
    ...
    // MySQL connection
    db.setConnectOptions("CLIENT_SSL;CLIENT_IGNORE_SPACE"); // use an SSL connection to the server
    if (!db.open()) {
        db.setConnectOptions(); // clears the connect option string
        ...
    }
    ...
    // PostgreSQL connection
    db.setConnectOptions("requiressl=1"); // enable PostgreSQL SSL connections
    if (!db.open()) {
        db.setConnectOptions(); // clear options
        ...
    }
    ...
    // ODBC connection
    db.setConnectOptions("SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_ONLY;SQL_ATTR_TRACE=SQL_OPT_TRACE_ON"); // set ODBC options
    if (!db.open()) {
        db.setConnectOptions(); // don't try to set this option
        ...
    }
    \endcode

    Please refer to the client library documentation for more
    information about the different options. The options will be set
    prior to opening the database connection. Setting new options
    without re-opening the connection does nothing.

    \sa connectOptions()
*/

void QSqlDatabase::setConnectOptions(const QString& options)
{
    d->connOptions = options;
}

/*! Returns the connection options for for this connection

    \sa setConnectOptions()
 */
QString QSqlDatabase::connectOptions() const
{
    return d->connOptions;
}

/*!
    Returns true if a driver called \a name is available; otherwise
    returns false.

    \sa drivers()
*/

bool QSqlDatabase::isDriverAvailable(const QString& name)
{
    return drivers().contains(name);
}

/*! \overload

    This function is useful if you need to set up the database
    connection and instantiate the driver yourself. If you do this, it
    is recommended that you include the driver code in your own
    application. For example, setting up a custom PostgreSQL
    connection and instantiating the QPSQL7 driver can be done the
    following way:

    \code
    #include "qtdir/src/sql/drivers/psql/qsql_psql.cpp"
    \endcode
    (We assume that \c qtdir is the directory where Qt is installed.)
    This will pull in the code that is needed to use the PostgreSQL
    client library and to instantiate a QPSQLDriver object, assuming
    that you have the PostgreSQL headers somewhere in your include
    search path.

    \code
    PGconn* con = PQconnectdb("host=server user=bart password=simpson dbname=springfield");
    QPSQLDriver* drv =  new QPSQLDriver(con);
    QSqlDatabase db = QSqlDatabase::addDatabase(drv); // becomes the new default connection
    QSqlQuery q;
    q.exec("SELECT * FROM people");
    ...
    \endcode

    The above code sets up a PostgreSQL connection and instantiates a
    QPSQLDriver object. Next, addDatabase() is called to add the
    connection to the known connections so that it can be used by the
    Qt SQL classes. When a driver is instantiated with a connection
    handle (or set of handles), Qt assumes that you have already
    opened the database connection.

    Remember that you must link your application against the database
    client library as well. The simplest way to do this is to add
    lines like those below to your \c .pro file:

    \code
    unix:LIBS += -lpq
    win32:LIBS += libpqdll.lib
    \endcode

    You will need to have the client library in your linker's search
    path.

    The method described above will work for all the drivers, the only
    difference is the arguments the driver constructors take. Below is
    an overview of the drivers and their constructor arguments.

    \table
    \header \i Driver \i Class name \i Constructor arguments \i File to include
    \row
    \i QPSQL7
    \i QPSQLDriver
    \i PGconn* connection
    \i \c qsql_psql.cpp
    \row
    \i QMYSQL3
    \i QMYSQLDriver
    \i MYSQL* connection
    \i \c qsql_mysql.cpp
    \row
    \i QOCI8
    \i QOCIDriver
    \i OCIEnv* environment, OCIError* error, OCISvcCtx* serviceContext
    \i \c qsql_oci.cpp
    \row
    \i QODBC3
    \i QODBCDriver
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_odbc.cpp
    \row
    \i QDB2
    \i QDB2
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_db2.cpp
    \row
    \i QTDS7
    \i QTDSDriver
    \i LOGINREC* loginRecord, DBPROCESS* dbProcess, const QString& hostName
    \i \c qsql_tds.cpp
    \row
    \i QSQLITE
    \i QSQLiteDriver
    \i sqlite* connection
    \i \c qsql_sqlite.cpp
    \row
    \i QIBASE
    \i QIBaseDriver
    \i isc_db_handle connection
    \i \c qsql_ibase.cpp
    \endtable

    Note: The host name (or service name) is needed when constructing
    the QTDSDriver for creating new connections for internal
    queries. This is to prevent the simultaneous usage of several
    QSqlQuery/\l{QSqlCursor} objects from blocking each other.

    \warning The SQL framework takes ownership of the \a driver pointer,
    and it should not be deleted. If you want to
    explicitly remove the connection, use removeDatabase()

    \sa drivers()
*/

QSqlDatabase QSqlDatabase::addDatabase(QSqlDriver* driver, const QString& connectionName)
{
    QSqlDatabase db(driver);
    QSqlDatabasePrivate::addDatabase(db, connectionName);
    return db;
}

/*!
   Returns true if the QSqlDatabase has a valid driver.
 */
bool QSqlDatabase::isValid() const
{
    return d->driver && d->driver != d->shared_null()->driver;
}

#ifdef QT_COMPAT
QSqlRecord QSqlDatabase::record(const QSqlQuery& query) const
{ return query.record(); }

QSqlRecord QSqlDatabase::recordInfo(const QSqlQuery& query) const
{ return query.record(); }
#endif

/*!
   Clones the database connection \a other and and stores it as \a connectionName.
   Does nothing if \a other is an invalid database.
   Returns the newly created database connection. Note that the connection is not opened,
   to use it, it is neccessary to call open() first.
 */
QSqlDatabase QSqlDatabase::cloneDatabase(const QSqlDatabase &other, const QString &connectionName)
{
    if (!other.isValid())
        return QSqlDatabase();

    QSqlDatabase db(other.driver());
    db.d->copy(other.d);
    QSqlDatabasePrivate::addDatabase(db, connectionName);
    return db;
}
#endif // QT_NO_SQL
