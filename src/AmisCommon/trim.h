/*
 Copyright (C) 2012 Kolibre
 
 This file is part of Kolibre-amis.
 
 Kolibre-amis is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 2.1 of the License, or
 (at your option) any later version.
 
 Kolibre-amis is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with Kolibre-amis.  If not, see <http://www.gnu.org/licenses/>.

*/

//: C03:Trim.h

#ifndef TRIM_H
#define TRIM_H
#include <string>


/*! General tool to strip spaces from both ends of a string*/
inline std::string trim(const std::string& s)
{
    if (s.length() == 0)
        return s;
    std::string::size_type beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
    std::string::size_type end = s.find_last_not_of(" \a\b\f\n\r\t\v");
    if (beg == std::string::npos) // No non-spaces
        return "";

    std::string trimmed = std::string(s, beg, end - beg + 1);

    // Remove all newlines

    std::string::size_type pos = trimmed.find("\n");
    while (pos != std::string::npos)
    {
        //LOG4CXX_TRACE(amisTrimLog, "removing endl at pos " << pos );
        trimmed = trimmed.replace(pos, 1, " ");
        pos = trimmed.find("\n");
    }

    // Remove all double spaces "  "
    pos = trimmed.find("  ");
    while (pos != std::string::npos)
    {
        //LOG4CXX_TRACE(amisTrimLog, "removing double space at pos " << pos );
        trimmed = trimmed.replace(pos, 2, " ");
        pos = trimmed.find("  ");
    }

    return trimmed;
    //return std::string(s, beg, end - beg + 1);
}

/*! General tool to strip spaces from both ends of a string*/
inline std::string strim(const std::string& s)
{
    if (s.length() == 0)
        return s;

    std::string trimmed = s;

    // Remove all newlines

    std::string::size_type pos = trimmed.find("\n");
    while (pos != std::string::npos)
    {
        //LOG4CXX_TRACE(amisTrimLog, "removing endl at pos " << pos );
        trimmed = trimmed.replace(pos, 1, " ");
        pos = trimmed.find("\n");
    }

    // Remove all double spaces "  "
    pos = trimmed.find("  ");
    while (pos != std::string::npos)
    {
        //LOG4CXX_TRACE(amisTrimLog, "removing double space at pos " << pos );
        trimmed = trimmed.replace(pos, 2, " ");
        pos = trimmed.find("  ");
    }

    return trimmed;
    //return std::string(s, beg, end - beg + 1);
}


#endif // TRIM_H ///:~
