#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([image-id], [2.1.0], [calvin.walton@kepstin.ca])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([image_id.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99

# Checks for libraries.
PKG_CHECK_MODULES([IMAGE_ID], [libdiscid libmirage >= 2.1.0])
AC_MSG_CHECKING([if libMirage is 3.0 or later])
PKG_CHECK_EXISTS([libmirage >= 3.0.0],
		[libmirage_3_0_api=yes],
		[libmirage_3_0_api=no])
if test $libmirage_3_0_api = yes
then
	AC_DEFINE([LIBMIRAGE_AT_LEAST_3_0], [1],
				[libMirage API is 3.0 or newer])
fi
AC_MSG_RESULT([$libmirage_3_0_api])


# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
