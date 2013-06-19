## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-amis.
#
# Kolibre-amis is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-amis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-amis. If not, see <http://www.gnu.org/licenses/>.


# Process this file with autoconf to produce a configure script.

dnl -----------------------------------------------
dnl  Release instructions
dnl -----------------------------------------------
dnl
dnl  When you are about to release the software
dnl  either as a stable release or an unstable release
dnl  you should update the *_VERSION variables below
dnl  according to the following rules:
dnl
dnl  1. If any interfaces has been removed or changed,
dnl     increment MAJOR_VERSION and set others to 0
dnl  2. If any interface has been added,
dnl     increment MINOR_VERSION and set PATCH_VERSION to 0
dnl  3. If only the source code has changed,
dnl     increment PATCH_VERSION
dnl
dnl  If you are releasing a stable version,
dnl  set package version number in AC_INIT to
dnl  [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION]
dnl
dnl  If you are releasing an unstable version,
dnl  add a string (i.e -alpha or -beta or -rc1) to the
dnl  package version in AC_INIT after $PATCH_VERSION
dnl  e.g. [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION-rc1]


# Setup version here:
m4_define([MAJOR_VERSION], [0])
m4_define([MINOR_VERSION], [0])
m4_define([PATCH_VERSION], [1])
m4_define([EXTRA_VERSION], [])

AC_PREREQ([2.67])
AC_INIT([kolibre-amis],[MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION[]EXTRA_VERSION],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([tar-pax color-tests parallel-tests])

VERSION_INFO=m4_eval(MAJOR_VERSION + MINOR_VERSION):PATCH_VERSION:MINOR_VERSION
AC_SUBST(VERSION_INFO)

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
ACX_PTHREAD
LT_INIT

dnl -----------------------------------------------
dnl Doxygen settings
dnl -----------------------------------------------

DX_DOXYGEN_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(ON)
DX_INIT_DOXYGEN([kolibre-amis], doxygen.cfg, [doxygen-doc])

# Checks for libraries.

dnl -----------------------------------------------
dnl check for log4cxx
dnl -----------------------------------------------

PKG_CHECK_MODULES(LOG4CXX, liblog4cxx >= 0.10.0)

AC_SUBST(LOG4CXX_CFLAGS)
AC_SUBST(LOG4CXX_LIBS)

dnl -----------------------------------------------
dnl Check for libkolibre-player
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREPLAYER, libkolibre-player >= 0.0.1)

AC_SUBST(LIBKOLIBREPLAYER_LIBS)
AC_SUBST(LIBKOLIBREPLAYER_CFLAGS)

dnl -----------------------------------------------
dnl Check for libkolibre-xmlreader
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREXMLREADER, libkolibre-xmlreader >= 0.0.1)

AC_SUBST(LIBKOLIBREXMLREADER_LIBS)
AC_SUBST(LIBKOLIBREXMLREADER_CFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h locale.h malloc.h stdlib.h string.h sys/ioctl.h termio.h termios.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm localeconv memset mkdir select setenv setlocale strdup strerror strstr])

AC_CONFIG_FILES([Makefile
                 libkolibre-amis.pc
                 src/AmisCommon/Makefile
                 src/DaisyHandler/Makefile
                 src/Makefile
                 src/NavParse/Makefile
                 src/SmilEngine/Makefile
                 tests/DaisyTest/Makefile
                 tests/HandlerTest/Makefile
                 tests/JumpTest/Makefile
                 tests/Makefile])
AC_OUTPUT
