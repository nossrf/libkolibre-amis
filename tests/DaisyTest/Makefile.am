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

check_PROGRAMS = daisytest
#TESTS = daisytest.sh

daisytest_SOURCES = DaisyTest.cpp
daisytest_LDADD = @LIBKOLIBREXMLREADER_LIBS@ @LOG4CXX_LIBS@ -L$(top_builddir)/src -lkolibre-amis -lxml2
daisytest_CPPFLAGS = @LOG4CXX_CFLAGS@ @LIBKOLIBREXMLREADER_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/src/AmisCommon -I$(top_srcdir)/src/SmilEngine -I$(top_srcdir)/src/NavParse -I$(top_srcdir)/src/DaisyHandler -I$(srcdir)/../ -g

EXTRA_DIST = DaisyTest.h \
			runbt \
			run \
			daisytest.sh
