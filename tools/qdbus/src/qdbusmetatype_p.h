/* qdbusmetatype_p.h
 *
 * Copyright (C) 2006 Trolltech AS. All rights reserved.
 *    Author: Thiago Macieira <thiago.macieira@trolltech.com>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef QDBUSMETATYPE_P_H
#define QDBUSMETATYPE_P_H

#include "qdbusmetatype.h"

struct QDBusMetaTypeId
{
    static int message;         // QDBusMessage
    static int argument;        // QDBusArgument
    static int variant;         // QDBusVariant
    static int objectpath;      // QDBusObjectPath
    static int signature;       // QDBusSignature

    static void init();
};

#endif
