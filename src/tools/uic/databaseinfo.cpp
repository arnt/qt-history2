/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "databaseinfo.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"

DatabaseInfo::DatabaseInfo(Driver *drv)
    : driver(drv)
{
}

void DatabaseInfo::accept(DomUI *node)
{
    m_connections.clear();
    m_cursors.clear();
    m_fields.clear();

    TreeWalker::accept(node);
}

void DatabaseInfo::accept(DomWidget *node)
{
    QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    DomProperty *frameworkCode = properties.value("frameworkCode", 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    DomProperty *db = properties.value("database", 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        QString connection = info.size() > 0 ? info.at(0) : QString();
        QString table = info.size() > 1 ? info.at(1) : QString();
        QString field = info.size() > 2 ? info.at(2) : QString();

        if (connection.size()) {
            m_connections.append(connection);
            if (table.size()) {
                m_cursors[connection].append(table);
                if (field.size()) {
                    m_fields[connection].append(field);
                }
            }
        }
    }

    TreeWalker::accept(node);
}

