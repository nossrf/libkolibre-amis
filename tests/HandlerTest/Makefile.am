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

check_PROGRAMS = handlertest handlertestunit
TESTS = handlertestunit.sh

handlertest_SOURCES = HandlerTest.cpp FakePlayer.cpp
handlertest_LDADD = @LIBKOLIBREXMLREADER_LIBS@ $(top_builddir)/src/libkolibre-amis.la

handlertestunit_SOURCES = HandlerTestUnit.cpp FakePlayer.cpp
handlertestunit_LDADD = @LIBKOLIBREXMLREADER_LIBS@ $(top_builddir)/src/libkolibre-amis.la

AM_LDFLAGS = -lpthread @LOG4CXX_LIBS@
AM_CPPFLAGS = @LIBKOLIBREXMLREADER_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/src/AmisCommon -I$(top_srcdir)/src/SmilEngine -I$(top_srcdir)/src/NavParse -I$(top_srcdir)/src/DaisyHandler -g 

EXTRA_DIST = FakePlayer.h \
			 runbt \
			 run \
			 handlertest.sh \
			 handlertestunit.sh 
