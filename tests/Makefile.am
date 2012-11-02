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

AUTOMAKE_OPTIONS = foreign

SUBDIRS = HandlerTest DaisyTest JumpTest

check_PROGRAMS = md5test binsmilsearch navpointtest jumppagetest
TESTS = md5test binsmilsearch.sh navpointtest.sh jumppagetest.sh

md5test_SOURCES = md5test.cpp
md5test_CXXFLAGS = -I$(top_srcdir)/src/AmisCommon -g
md5test_LDFLAGS = $(top_builddir)/src/AmisCommon/md5.lo

binsmilsearch_SOURCES = BinSmilSearch.cpp
binsmilsearch_CPPFLAGS = -g @LOG4CXX_CFLAGS@ @LIBKOLIBREXMLREADER_CFLAGS@
binsmilsearch_LDADD = -L$(top_builddir)/src/ -lkolibre-amis @LOG4CXX_LIBS@ @LIBKOLIBREXMLREADER_LIBS@

navpointtest_SOURCES = NavPointTest.cpp
navpointtest_CPPFLAGS = -g @LOG4CXX_CFLAGS@ @LIBKOLIBREXMLREADER_CFLAGS@
navpointtest_LDADD = -L$(top_builddir)/src/ -lkolibre-amis @LOG4CXX_LIBS@ @LIBKOLIBREXMLREADER_LIBS@

jumppagetest_SOURCES = JumpPageTest.cpp
jumppagetest_CPPFLAGS = -g @LOG4CXX_CFLAGS@ @LIBKOLIBREXMLREADER_CFLAGS@
jumppagetest_LDADD = -L$(top_builddir)/src/ -lkolibre-amis @LOG4CXX_LIBS@ @LIBKOLIBREXMLREADER_LIBS@

INCLUDES = -I$(top_srcdir)/src/AmisCommon -I$(top_srcdir)/src/DaisyHandler -I$(top_srcdir)/src/NavParse -I$(top_srcdir)/src/SmilEngine

EXTRA_DIST = binsmilsearch.sh \
			 navpointtest.sh \
			 jumppagetest.sh \
			 setup_logging.h \
			 run \
			 data

clean-local: clean-local-check
.PHONY: clean-local-check

clean-local-check:
	-rm -rf *.log
