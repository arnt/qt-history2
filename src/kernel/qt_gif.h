/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_gif.h#1 $
**
** To enable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 1".
**
** To disable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 0".
**
** WARNING:
**	Enabling this software to read GIF files, and hence allowing any
**	software using this software using the GIF format may require you
**	to make licensing arrangements with Unisys Corporation.
**	By making modifications to this file, you acknowledge that you
**	understand the consequences of those modifications.
**
*****************************************************************************/

#define QT_BUILTIN_GIF_READER 0

bool qt_builtin_gif_reader();
