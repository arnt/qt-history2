/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qgif.h#8 $
**
** To enable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 1".
**
** To disable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 0".
**
** WARNING:
**      A separate license from Unisys may be required to use the gif
**      reader. See http://www.unisys.com/unisys/lzw/
**      for information from Unisys
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QT_H
#include <qfeatures.h>
#endif // QT_H

#ifndef QT_BUILTIN_GIF_READER
#define QT_BUILTIN_GIF_READER 1
#endif

bool qt_builtin_gif_reader();
