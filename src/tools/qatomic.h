#ifndef QATOMIC_H
#define QATOMIC_H

#if defined(__GNUC__)
#  if defined(__i386__)
#    include "qatomic_x86.h"
#  elif defined(__ppc__)
#    include "qatomic_ppc.h"
#  endif
#elif defined(__INTEL_COMPILER)
#  include "qatomic_x86.h"
#elif defined(_MSC_VER)
#  include "qatomic_win.h"
#elif defined(__sun) || defined(sun)
#  include "qatomic_sun.h"
#else
#  error "Unknown compiler/CPU combination."
#endif

#endif
