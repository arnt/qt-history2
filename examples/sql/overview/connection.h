/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Enter your connection info here

#define DB_SALES_DRIVER	    "QOCI8"
#define DB_SALES_DBNAME	    "AXDB"
#define DB_SALES_USER	    "test"
#define DB_SALES_PASSWD	    "test"
#define DB_SALES_HOST	    ""

#define DB_ORDERS_DRIVER    "QOCI8"
#define DB_ORDERS_DBNAME    "AXDB"
#define DB_ORDERS_USER	    "test"
#define DB_ORDERS_PASSWD    "test"
#define DB_ORDERS_HOST	    ""

bool createConnections();
