# Components
#
COMPONENT_SRCDIRS = core demo

# Treat all warnings as fatal
#
CFLAGS += -Wall -Wextra -Werror

# Inhibit spurious errors from -Waddress
#
CFLAGS += -Wno-address

# Include common header file
#
CFLAGS += -include compiler.h
