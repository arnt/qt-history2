/****************************************************************************
** $Id$
**
** Implementation of Qt/Embedded Qnx mouse drivers
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifdef Q_OS_QNX4

#include "qwsmouse_qnx4.h"

QQnxMouseHandlerPrivate::QQnxMouseHandlerPrivate(MouseProtocol &protocol,QString dev) : QWSMouseHandler() {
}

QQnxMouseHandlerPrivate::~QQnxMouseHandlerPrivate(){};
void QQnxMouseHandlerPrivate::clearCalibration(){};
void QQnxMouseHandlerPrivate::calibrate(){};
void QQnxMouseHandlerPrivate::getCalibration(QWSPointerCalibrationData *){};

void QQnxMouseHandlerPrivate::getData() {
}

void QQnxMouseHandlerPrivate::readMouseData(int fd) {
}

#endif
