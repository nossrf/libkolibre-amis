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

#include "BinarySmilSearch.h"
#include "SpineBuilder.h"
#include "SmilEngine.h"

#include <sstream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisBinarySmilSearchLog(
        log4cxx::Logger::getLogger("kolibre.amis.binarysmilsearch"));
using namespace std;
using namespace amis;

// "00:00:00" -> unsigned int
unsigned int stringToSeconds(string timeString)
{
    // TODO: put some iss.good() in there to make sure the parsing worked.
    unsigned int seconds;
    {
        int number;
        char colon;
        istringstream iss(timeString);
        iss >> number;
        seconds = number * 60 * 60;
        iss >> colon;
        iss >> number;
        seconds += number * 60;
        iss >> colon;
        iss >> number;
        seconds += number;
    }
    return seconds;
}

SmilTreeBuilder* BinarySmilSearch::buildTree(int id)
{
    LOG4CXX_DEBUG(amisBinarySmilSearchLog, "Trying smil id: " << id);

    currentSmilPath = SmilEngine::Instance()->getSmilFilePath(id);
    SmilTreeBuilder builder;
    currentTreeBuilder = builder; // Clear the treeBuilder
    //Share daisyversion between files
    currentTreeBuilder.setDaisyVersion(
            SmilEngine::Instance()->getDaisyVersion());
    currentSmilTree = SmilTree();
    if (amis::OK
            == currentTreeBuilder.createSmilTree(&currentSmilTree,
                    currentSmilPath).getCode())
    {
        return &currentTreeBuilder;
    }

    LOG4CXX_ERROR(amisBinarySmilSearchLog, "Failed to build smil tree for " << currentSmilPath);
    return NULL;
}

bool BinarySmilSearch::currentSmilIsBeyond(unsigned int seconds)
{
    string smilStartingAt;
    switch (currentTreeBuilder.getDaisyVersion())
    {
    case DAISY3:
        smilStartingAt = currentTreeBuilder.getMetadata("dtb:totalelapsedtime");
        break;
    case DAISY202:
        smilStartingAt = currentTreeBuilder.getMetadata("ncc:totalelapsedtime");
        break;
    default:
        LOG4CXX_ERROR( amisBinarySmilSearchLog,
                "Unsupported daisy format: " << currentTreeBuilder.getDaisyVersion());
        break;
    }
    if (smilStartingAt.empty())
        throw 0;

    unsigned int comparedTo = stringToSeconds(smilStartingAt);

    LOG4CXX_DEBUG( amisBinarySmilSearchLog,
            "Looking for : " << seconds << ", current smil starts at: " << comparedTo);

    if (comparedTo > seconds)
        return true;

    return false;
}

bool BinarySmilSearch::currentSmilContains(unsigned int seconds)
{
    if (currentSmilIsBeyond(seconds))
        return false;

    unsigned int timeInThisSmilSeconds = currentSmilTree.getSmilDuration();
    if (timeInThisSmilSeconds == 0)
    {
        string timeInThisSmil = currentTreeBuilder.getMetadata(
                "ncc:timeinthissmil");
        //Check if the timeinthissmil was defined
        if (timeInThisSmil.empty())
        {
            throw 0;
        }
        timeInThisSmilSeconds = stringToSeconds(timeInThisSmil);
    }

    string smilStartingAt;
    switch (currentTreeBuilder.getDaisyVersion())
    {
    case DAISY3:
        smilStartingAt = currentTreeBuilder.getMetadata("dtb:totalelapsedtime");
        break;
    case DAISY202:
        smilStartingAt = currentTreeBuilder.getMetadata("ncc:totalelapsedtime");
        break;
    default:
        LOG4CXX_ERROR( amisBinarySmilSearchLog,
                "Unsupported daisy format: " << currentTreeBuilder.getDaisyVersion());
        break;
    }
    if (smilStartingAt.empty())
        throw 0;

    unsigned int smilStartingAtSeconds = stringToSeconds(smilStartingAt);

    LOG4CXX_DEBUG( amisBinarySmilSearchLog,
            "Looking for : " << seconds << ", current smil contains: " << timeInThisSmilSeconds + smilStartingAtSeconds);
    if ((timeInThisSmilSeconds + smilStartingAtSeconds) < seconds) // seconds is beyond this smil file
        return false;

    return true;
}

SmilTreeBuilder* BinarySmilSearch::begin()
{
    if (not SmilEngine::Instance())
    {
        LOG4CXX_ERROR( amisBinarySmilSearchLog,
                "Smilengine is not setup correctly");
        return NULL;
    }

    LOG4CXX_DEBUG(amisBinarySmilSearchLog, "Begin binary serach for smil file");

    upperSmilIdx = SmilEngine::Instance()->getNumberOfSmilFiles();
    lowerSmilIdx = 0;

    // Get the smil file in the middle.
    currentSmilIdx = lowerSmilIdx + (upperSmilIdx - lowerSmilIdx) / 2;

    LOG4CXX_DEBUG(amisBinarySmilSearchLog, "Starting in the middle");
    LOG4CXX_DEBUG(amisBinarySmilSearchLog, "current: " << currentSmilIdx << " upper: " << upperSmilIdx << " lower: " << lowerSmilIdx);

    // Man kan ändra på upper och lower ifall man har information
    // om vilka smil filer som går att söka mellan. Alltså om man
    // har läst halva boken så kan man sätta lower till upper/2
    // och current mittemellan lower och upper som vanligt.

    return buildTree(currentSmilIdx);
}

SmilTreeBuilder* BinarySmilSearch::next(searchDirection direction)
{
    if (lowerSmilIdx == upperSmilIdx)
        return NULL;

    if (direction == UP)
    {
        LOG4CXX_DEBUG(amisBinarySmilSearchLog, "Searching next in direction UP");
        lowerSmilIdx = currentSmilIdx;
        currentSmilIdx = (lowerSmilIdx + upperSmilIdx) / 2;
        if (currentSmilIdx == lowerSmilIdx)
            return NULL;
    }
    else
    {
        LOG4CXX_DEBUG(amisBinarySmilSearchLog, "Searching next in direction DOWN");
        upperSmilIdx = currentSmilIdx;
        currentSmilIdx = (lowerSmilIdx + upperSmilIdx) / 2;
        if (upperSmilIdx == currentSmilIdx)
            return NULL;
    }

    return buildTree(currentSmilIdx);
}

string BinarySmilSearch::getCurrentSmilPath()
{
    return currentSmilPath;
}

SmilTree* BinarySmilSearch::getCurrentSmilTree()
{
    return &currentSmilTree;
}
