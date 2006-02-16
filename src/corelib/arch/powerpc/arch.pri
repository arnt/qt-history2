#
# PowerPC architecture
#
!*-g++* {
    *-64 {
        SOURCES += $$ARCH_CPP/qatomic64.s
    } else {
        SOURCES += $$ARCH_CPP/qatomic32.s
    }
}
