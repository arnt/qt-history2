/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgl_p.h"

QT_BEGIN_NAMESPACE

bool qt_resolve_framebufferobject_extensions(QGLContext *ctx)
{
    if (glIsRenderbufferEXT != 0)
        return true;

    if (ctx == 0) {
        qWarning("QGLFramebufferObject: Unable to resolve framebuffer object extensions -"
                 " make sure there is a current context when creating the framebuffer object.");
        return false;
    }

    glIsRenderbufferEXT = (_glIsRenderbufferEXT) ctx->getProcAddress(QLatin1String("glIsRenderbufferEXT"));
    glBindRenderbufferEXT = (_glBindRenderbufferEXT) ctx->getProcAddress(QLatin1String("glBindRenderbufferEXT"));
    glDeleteRenderbuffersEXT = (_glDeleteRenderbuffersEXT) ctx->getProcAddress(QLatin1String("glDeleteRenderbuffersEXT"));
    glGenRenderbuffersEXT = (_glGenRenderbuffersEXT) ctx->getProcAddress(QLatin1String("glGenRenderbuffersEXT"));
    glRenderbufferStorageEXT = (_glRenderbufferStorageEXT) ctx->getProcAddress(QLatin1String("glRenderbufferStorageEXT"));
    glGetRenderbufferParameterivEXT =
        (_glGetRenderbufferParameterivEXT) ctx->getProcAddress(QLatin1String("glGetRenderbufferParameterivEXT"));
    glIsFramebufferEXT = (_glIsFramebufferEXT) ctx->getProcAddress(QLatin1String("glIsFramebufferEXT"));
    glBindFramebufferEXT = (_glBindFramebufferEXT) ctx->getProcAddress(QLatin1String("glBindFramebufferEXT"));
    glDeleteFramebuffersEXT = (_glDeleteFramebuffersEXT) ctx->getProcAddress(QLatin1String("glDeleteFramebuffersEXT"));
    glGenFramebuffersEXT = (_glGenFramebuffersEXT) ctx->getProcAddress(QLatin1String("glGenFramebuffersEXT"));
    glCheckFramebufferStatusEXT = (_glCheckFramebufferStatusEXT) ctx->getProcAddress(QLatin1String("glCheckFramebufferStatusEXT"));
    glFramebufferTexture1DEXT = (_glFramebufferTexture1DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture1DEXT"));
    glFramebufferTexture2DEXT = (_glFramebufferTexture2DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture2DEXT"));
    glFramebufferTexture3DEXT = (_glFramebufferTexture3DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture3DEXT"));
    glFramebufferRenderbufferEXT = (_glFramebufferRenderbufferEXT) ctx->getProcAddress(QLatin1String("glFramebufferRenderbufferEXT"));
    glGetFramebufferAttachmentParameterivEXT =
        (_glGetFramebufferAttachmentParameterivEXT) ctx->getProcAddress(QLatin1String("glGetFramebufferAttachmentParameterivEXT"));
    glGenerateMipmapEXT = (_glGenerateMipmapEXT) ctx->getProcAddress(QLatin1String("glGenerateMipmapEXT"));
    return glIsRenderbufferEXT;
}

bool qt_resolve_version_1_3_functions(QGLContext *ctx)
{
    if (glMultiTexCoord4f != 0)
        return true;

    QGLContext cx(QGLFormat::defaultFormat());
    glMultiTexCoord4f = (_glMultiTexCoord4f) ctx->getProcAddress(QLatin1String("glMultiTexCoord4f"));
    glActiveTexture = (_glActiveTexture) ctx->getProcAddress(QLatin1String("glActiveTexture"));

    return glMultiTexCoord4f && glActiveTexture;
}

bool qt_resolve_stencil_face_extension(QGLContext *ctx)
{
    if (glActiveStencilFaceEXT != 0)
        return true;

    QGLContext cx(QGLFormat::defaultFormat());
    glActiveStencilFaceEXT = (_glActiveStencilFaceEXT) ctx->getProcAddress(QLatin1String("glActiveStencilFaceEXT"));

    return glActiveStencilFaceEXT;
}

bool qt_resolve_frag_program_extensions(QGLContext *ctx)
{
    if (glProgramStringARB != 0)
        return true;

    // ARB_fragment_program
    glProgramStringARB = (_glProgramStringARB) ctx->getProcAddress(QLatin1String("glProgramStringARB"));
    glBindProgramARB = (_glBindProgramARB) ctx->getProcAddress(QLatin1String("glBindProgramARB"));
    glDeleteProgramsARB = (_glDeleteProgramsARB) ctx->getProcAddress(QLatin1String("glDeleteProgramsARB"));
    glGenProgramsARB = (_glGenProgramsARB) ctx->getProcAddress(QLatin1String("glGenProgramsARB"));
    glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) ctx->getProcAddress(QLatin1String("glProgramLocalParameter4fvARB"));

    return glProgramStringARB
        && glBindProgramARB
        && glDeleteProgramsARB
        && glGenProgramsARB
        && glProgramLocalParameter4fvARB;
}

QT_END_NAMESPACE
