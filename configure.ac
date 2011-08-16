#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(reevengi-tools, 0.5, patmandin@gmail.com)
AC_CONFIG_SRCDIR([src/pak2tim.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR([autotools])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strrchr pow])

# Checks for libraries.

# Math

if test "x$ac_cv_func_pow" = "xno"; then
	LIBS="$LIBS -lm"
fi

# SDL

AM_PATH_SDL( 1.2.0 , :, AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 vs2005/Makefile])
AC_OUTPUT
