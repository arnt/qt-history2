/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_gif.h#4 $
**
** To enable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 1".
**
** To disable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 0".
**
** WARNING:
**      A separate license from Unisys may be required to use the gif
**      reader. See http://corp2.unisys.com/LeadStory/lzwfaq.html
**      for information from Unisys
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#define QT_BUILTIN_GIF_READER 0 // MUST BE ZERO FOR ALL RELEASES

bool qt_builtin_gif_reader();
