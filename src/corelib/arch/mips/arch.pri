#
# MIPS 3/4 architecture
#
*-64:SOURCES += $$ARCH_CPP/qatomic64.s
else:SOURCES += $$ARCH_CPP/qatomic32.s
