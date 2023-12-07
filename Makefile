# Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
# This project uses the MIT license, a copy should be included under /LICENSE */
#
# This trick is originally from BearSSL to distinguish between bmake/nmake
# ======================================================================
# The lines below are a horrible hack that nonetheless works. On a
# "make" utility compatible with Single Unix v4 (this includes GNU and
# BSD make), the '\' at the end of a command line counts as an escape
# for the newline character, so the next line is still a comment.
# However, Microsoft's nmake.exe (that comes with Visual Studio) does
# not interpret the final '\' that way in a comment. The end result is
# that when using nmake.exe, this will include "mk/NMake.mk", whereas
# GNU/BSD make will include "mk/BMake.mk".

!ifndef 0 # \
!include mk/nmake.mk # \
!else
.POSIX:
include mk/bmake.mk
# Extra hack for OpenBSD make.
ifndef: all
0: all
endif: all
# \
!endif
