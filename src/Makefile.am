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

HDRS = AmisCommon/AmisCommon.h \
	   AmisCommon/AmisError.h \
	   DaisyHandler/DaisyHandler.h

lib_LTLIBRARIES = libkolibre-amis.la

# Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir = $(includedir)/libkolibre/amis-$(PACKAGE_VERSION)
library_include_HEADERS = $(HDRS)

SUBDIRS = AmisCommon NavParse SmilEngine DaisyHandler
DIST_SUBDIRS = $(SUBDIRS)

LIBADD = AmisCommon/libamiscommon.la \
	   DaisyHandler/libdaisyhandler.la \
	   NavParse/libnavparse.la \
	   SmilEngine/libsmilengine.la

libkolibre_amis_la_SOURCES = $(SRCS)
libkolibre_amis_la_LIBADD = $(LIBADD)
libkolibre_amis_la_LDFLAGS = -version-info $(VERSION_INFO) @LOG4CXX_LIBS@ @LIBKOLIBREXMLREADER_LIBS@
