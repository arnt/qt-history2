#ifndef QATOMIC_H
#define QATOMIC_H

#if defined(__GNUC__) && defined(__i386__)

#include "qatomic_x86.h"

#elif defined(_MSC_VER)

#include "qatomic_win.h"

#else
#  error "Unknown compiler/CPU combination."
#endif

#endif
