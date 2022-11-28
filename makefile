# ----------------------------
# Makefile Options
# ----------------------------

NAME         = PCAS
COMPRESSED   = YES
ICON         = iconc.png
DESCRIPTION  = "PineappleCAS"

CFLAGS       = -Wall -Oz
CXXFLAGS     = -Wall -Oz

include $(shell cedev-config --makefile)
