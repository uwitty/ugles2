#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libugles2], [0.1], [])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ugles2.h])
AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH(includes,
  AC_HELP_STRING(
    [--with-includes=PATH:PATH...], []),
    [
      case $with_includes in
        yes|no|"") ;;
        *) IFSorg=$IFS
           IFS=":"
           for p in $with_includes; do
             WITH_INCLUDES="${WITH_INCLUDES} -I$p"
           done
           IFS=$IFSorg ;;
      esac
    ])  
AC_SUBST(WITH_INCLUDES)
INCLUDES="$INCLUDES $WITH_INCLUDES"
CPPFLAGS="$CPPFLAGS $WITH_INCLUDES"

AC_ARG_WITH(libs,
  AC_HELP_STRING(
    [--with-libs=PATH:PATH...], []),
    [
      case $with_libs in
        yes|no|"") ;;
        *) IFSorg=$IFS
           IFS=":"
           for p in $with_libs; do
             WITH_LIBS="${WITH_LIBS} -L$p"
           done
           IFS=$IFSorg ;;
      esac
    ])  
AC_SUBST(WITH_LIBS)
LDFLAGS="$LDFLAGS $WITH_LIBS"


use_png=no
AC_ARG_ENABLE(png,
  AC_HELP_STRING(
    [--enagle-png], [Use libpng]),
    [
      use_png=yes
    ])

use_jpeg=no
AC_ARG_ENABLE(jpeg,
  AC_HELP_STRING(
    [--enagle-jpeg], [Use libjpeg]),
    [
      use_jpeg=yes
    ])

use_freetype=no
AC_ARG_ENABLE(freetype,
  AC_HELP_STRING(
    [--enagle-freetype], [Use libfreetype]),
    [
      use_freetype=yes
    ])


# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
AC_CHECK_HEADER([GLES2/gl2.h], [], [AC_MSG_ERROR([GLES2/gl2.h])])
AC_CHECK_HEADER([EGL/egl.h], [], [AC_MSG_ERROR([EGL/egl.h])])
case $use_png in
  yes)
    AC_CHECK_HEADER([png.h], [], [AC_MSG_ERROR([png.h])])
    CFLAGS="$CFLAGS -DUSE_PNG"
    ;;
  *) ;;
esac

case $use_jpeg in
  yes)
    AC_CHECK_HEADER([jpeglib.h], [], [AC_MSG_ERROR([jpeglib.h])])
    CFLAGS="$CFLAGS -DUSE_JPEG"
    ;;
  *) ;;
esac

case $use_freetype in
  yes)
    AC_CHECK_HEADER([ft2build.h], [], [AC_MSG_ERROR([ft2build.h])])
    CFLAGS="$CFLAGS -DUSE_FREETYPE"
    ;;
  *) ;;
esac

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
#AC_CONFIG_SUBDIRS([])
AC_OUTPUT
