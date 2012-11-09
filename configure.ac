#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([SimScheduler], [1.0], [oteodoro@uci.edu])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

AC_ARG_ENABLE([generic-build],
  AS_HELP_STRING([--enable-generic-build],
    [enable generic build support])
)
AM_CONDITIONAL(GENERICBUILD, test "x$enable_generic_build" = "xyes")

AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug],
    [enable debugger support])
)
AM_CONDITIONAL(DEBUG, test "x$enable_debug" = "xyes")

AC_ARG_ENABLE([tests],
  AS_HELP_STRING([--enable-tests],
    [enable data structure testing])
)
AM_CONDITIONAL(TESTS, test "x$enable_tests" = "xyes")

AC_ARG_ENABLE([graphics],
  AS_HELP_STRING([--enable-graphics],
    [enable suppport for graphics])
)
AS_IF([test "x$enable_graphics" = "xyes"], [
  PKG_CHECK_MODULES([SDL],[sdl],[],AC_MSG_ERROR([missing sdl libraries and headers]))
  PKG_CHECK_MODULES([FTGL],[ftgl],[],AC_MSG_ERROR([missing ftgl libraries and headers]))
  PKG_CHECK_MODULES([ASSIMP],[assimp],[],AC_MSG_ERROR([missing assimp libraries and headers]))
  AC_DEFINE(GRAPHICS,[1],[graphics support])
])
AM_CONDITIONAL(GRAPHICS, test "x$enable_graphics" = "xyes")

AC_ARG_ENABLE([openmp],
  AS_HELP_STRING([--enable-openmp],
    [enable openmp multicore support])
)
AM_CONDITIONAL(OPENMP, test "x$enable_openmp" = "xyes")
AS_IF([test "x$enable_openmp" = "xyes"], [
  AC_SUBST([OPENMP_LIBS],[-lopenmp])
])


# Checks for libraries.


# Checks for header files.
AC_CHECK_HEADERS([stdio.h stdlib.h stdarg.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AM_INIT_AUTOMAKE

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
