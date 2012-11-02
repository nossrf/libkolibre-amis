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

SRCS = DaisyHandler.cpp HistoryRecorder.cpp

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src/AmisCommon -I$(top_srcdir)/src/SmilEngine -I$(top_srcdir)/src/NavParse @LIBKOLIBREXMLREADER_CFLAGS@

noinst_LTLIBRARIES= libdaisyhandler.la
libdaisyhandler_la_SOURCES= $(SRCS)

EXTRA_DIST = AmisConstants.h \
			 HistoryRecorder.h
