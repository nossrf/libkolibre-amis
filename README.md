What is Kolibre?
---------------------------------
Kolibre is a Finnish non-profit association whose purpose is to promote
information systems that aid people with reading disabilities. The software
which Kolibre develops is published under open source and made available to all
stakeholders at github.com/kolibre.

Kolibre is committed to broad cooperation between organizations, businesses and
individuals around the innovative development of custom information systems for
people with different needs. More information about Kolibres activities, association 
membership and contact information can be found at http://www.kolibre.org/


What is libkolibre-amis?
---------------------------------
Libkolibre-amis is a branch of the amis2 code base. The libkolibre-amis code base is 
refactored to suit the needs of the Kolibre client daisy navigator and parser. 
The libkolibre-amis component uses libkolibre-xmlreader for parsing XML files. 
Libkolibre-xmlreader provides useful functionality as caching remote resources 
and a tidy fallback for cleaning up messy HTML files.


What is AMIS?
--------------------------------
AMIS stands for Adaptive Multimedia Information System.

AMIS is a software program that you can use to read DAISY books. It is 
self-voicing, meaning that no specialized screen-reading software is needed in
order for it to be used by visually impaired people. AMIS is open source 
software and is provided free of charge.

Please check out amis homepage for more info:
http://www.daisy.org/amis/amis-daisy-2.02-daisy-3-playback-software


Documentation
---------------------------------
Kolibre client developer documentation is available at https://github.com/kolibre/libkolibre-builder/wiki

This library is documented using doxygen. Generate documentation by executing

    $ ./configure
    $ make doxygen-doc


Platforms
---------------------------------
Libkolibre-amis has been tested with Linux Debian Squeeze and can be built
using dev packages from apt repositories.


Dependencies
---------------------------------
Major dependencies for libkolibre-amis:

* libkolibre-xmlreader
* liblog4cxx
* libpthread


Building from source
---------------------------------
If building from GIT sources, first do a:

    $ autoreconf -fi

If building from a release tarball you can skip the above step.

    $ ./configure
    $ make
    $ make install

see INSTALL for detailed instructions.


Licensing
---------------------------------
Copyright (C) 2012 Kolibre

This file is part of libkolibre-amis.

Libkolibre-amis is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libkolibre-amis is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libkolibre-amis. If not, see <http://www.gnu.org/licenses/>.

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/5046b0b30dca9b42d2425307dc880908 "githalytics.com")](http://githalytics.com/kolibre/libkolibre-amis)
