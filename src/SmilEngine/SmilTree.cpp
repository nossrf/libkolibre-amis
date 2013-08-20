/*
 SMIL Engine: linear and skippable navigation of Daisy 2.02 and Daisy 3 SMIL contents

 Copyright (C) 2004  DAISY for All Project
 
 Copyright (C) 2012 Kolibre

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <vector>

//PROJECT INCLUDES
#include "SmilEngineConstants.h"
#include "SpineBuilder.h"
#include "SmilTreeBuilder.h"
#include "SmilEngine.h"

#include "Node.h"
#include "TimeContainerNode.h"
#include "ContentNode.h"
#include "ParNode.h"
#include "SeqNode.h"

#include "SmilTree.h"
#include <math.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSmilTreeLog(
        log4cxx::Logger::getLogger("kolibre.amis.smiltree"));

using namespace std;

//--------------------------------------------------
//Default constructor
//--------------------------------------------------
SmilTree::SmilTree()
{
    //initialize variables and clear the skip options list
    mpRoot = NULL;
    mTreeStatus = amis::OK;
    mpSkipOptions = NULL;
    mpEscapeNode = NULL;
    mbEscapeRequested = false;
    mbCouldEscape = false;
    mDuration = 0;
}

//--------------------------------------------------
//Destructor
//--------------------------------------------------
SmilTree::~SmilTree()
{
    //delete the root node
    delete mpRoot;
}

//--------------------------------------------------
//set the root node pointer
//--------------------------------------------------
void SmilTree::setRoot(SeqNode* pNewRoot)
{
    mpRoot = pNewRoot;
}

//--------------------------------------------------
//get the root node pointer
//--------------------------------------------------
Node* SmilTree::getRoot()
{
    return mpRoot;
}

//--------------------------------------------------
//go to the first set of parallel nodes in the tree
//--------------------------------------------------
amis::AmisError SmilTree::goFirst(amis::SmilMediaGroup* pMedia)
{
    LOG4CXX_TRACE(amisSmilTreeLog, "Constructor" );

    amis::AmisError err;
    //local variables
    amis::ErrorCode result;
    string playback_data;

    mbCouldEscape = false;

    //call setFirst on the root
    if (mustSkipOrEscapeNode(mpRoot) == false)
    {
        result = mpRoot->setFirst();
    }
    else
    {
        result = amis::AT_END;
    }

    //ensure it didn't raise an error
    if (result == amis::OK)
    {
        //record the tree status as OK
        mTreeStatus = amis::OK;

        this->mCurrentId = "";
        //gather the playback data and send it to the Smil Engine
        mpRoot->play(pMedia);
        pMedia->setId(this->mCurrentId);
        pMedia->setEscape(mbCouldEscape);
        err.setCode(amis::OK);

    }
    else
    {
        //set the tree status to amis::AT_END
        mTreeStatus = amis::AT_END;
        err.setCode(amis::AT_END);
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

//--------------------------------------------------
//go to the last set of parallel nodes in the tree
//--------------------------------------------------
amis::AmisError SmilTree::goLast(amis::SmilMediaGroup* pMedia)
{
    LOG4CXX_TRACE(amisSmilTreeLog, "goLast" );

    amis::AmisError err;

    //local variables
    amis::ErrorCode result;
    string playback_data;

    mbCouldEscape = false;

    if (mustSkipOrEscapeNode(mpRoot) == false)
    {
        //call setLast on the root
        result = mpRoot->setLast();
    }
    else
    {
        result = amis::AT_BEGINNING;
    }

    //ensure it didn't raise an error
    if (result == amis::OK)
    {
        //record the tree status as being OK
        mTreeStatus = amis::OK;

        this->mCurrentId = "";
        //gather the playback data and send it to the Smil Engine
        mpRoot->play(pMedia);
        pMedia->setEscape(mbCouldEscape);
        pMedia->setId(this->mCurrentId);
        err.setCode(amis::OK);
    }
    else
    {
        //set the tree status to amis::AT_END
        mTreeStatus = amis::AT_BEGINNING;
        err.setCode(amis::AT_BEGINNING);
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

//--------------------------------------------------
//go to the next set of parallel nodes in the smil tree
//--------------------------------------------------
amis::AmisError SmilTree::goNext(amis::SmilMediaGroup* pMedia)
{
    amis::AmisError err;
    //local variables
    amis::ErrorCode result;
    string playback_data;

    mbCouldEscape = false;

    LOG4CXX_TRACE(amisSmilTreeLog, "goNext" );

    //if the tree is at the beginning, we should go to the first node
    if (mTreeStatus == amis::AT_BEGINNING)
    {
        err = goFirst(pMedia);
        pMedia->setEscape(mbCouldEscape);
    }

    //else, the tree is somewhere in the middle
    else
    {
        if (mustSkipOrEscapeNode(mpRoot) == false)
        {
            //call setNext on the root
            result = mpRoot->setNext();
        }
        else
        {
            result = amis::AT_END;
        }

        //if we are not going to pass the end of the tree
        if (result == amis::OK)
        {
            //record the tree status as amis::OK
            mTreeStatus = amis::OK;

            this->mCurrentId = "";
            //gather the playback data and send it to the Smil Engine
            mpRoot->play(pMedia);
            pMedia->setEscape(mbCouldEscape);
            pMedia->setId(this->mCurrentId);
            err.setCode(amis::OK);
        }

        //else we have hit the end of the tree
        else
        {
            //set the tree status to amis::AT_END
            mTreeStatus = amis::AT_END;
            err.setCode(amis::AT_END);
        }

    } //end else: the tree is in the middle

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;

} //end SmilTree::goNext() function

//--------------------------------------------------
//Go to the previous set of parallel nodes in the Smil Tree
//--------------------------------------------------
amis::AmisError SmilTree::goPrevious(amis::SmilMediaGroup* pMedia)
{
    LOG4CXX_TRACE(amisSmilTreeLog, "goPrevious" );

    amis::AmisError err;

    //local variables
    amis::ErrorCode result;
    string playback_data;

    mbCouldEscape = false;

    //if the tree is at the end, we should go to the last node
    if (mTreeStatus == amis::AT_END)
    {
        err = goLast(pMedia);
        pMedia->setEscape(mbCouldEscape);
    }

    //else, the tree is somewhere in the middle
    else
    {
        if (mustSkipOrEscapeNode(mpRoot) == false)
        {
            //call setPrevious() on the root
            result = mpRoot->setPrevious();
        }
        else
        {
            result = amis::AT_BEGINNING;
        }

        //if we are not going to pass the beginning of the tree
        if (result == amis::OK)
        {
            //record the tree status as amis::OK
            mTreeStatus = amis::OK;

            this->mCurrentId = "";
            //gather the playback data and send it to the Smil Engine
            mpRoot->play(pMedia);
            pMedia->setId(this->mCurrentId);
            pMedia->setEscape(mbCouldEscape);
            err.setCode(amis::OK);
        }

        //else, we have hit the beginning of the tree
        else
        {
            //set the tree status to amis::AT_BEGINNING
            mTreeStatus = amis::AT_BEGINNING;
            err.setCode(amis::AT_BEGINNING);
        }

    } //end else: we are somewhere in the middle of the tree

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;

} //end SmilTree::goFirst() function

//--------------------------------------------------
//escape the current structure
//--------------------------------------------------
amis::AmisError SmilTree::escapeStructure(amis::SmilMediaGroup* pMedia)
{
    LOG4CXX_TRACE(amisSmilTreeLog, "escapeStructure" );

    amis::AmisError err;

    if (mbCouldEscape == true)
    {
        mbCouldEscape = false;
        mbEscapeRequested = true;
        err = goNext(pMedia);
        pMedia->setEscape(mbCouldEscape);
        mbEscapeRequested = false;
    }
    else
    {
        mbCouldEscape = false;
        err = goNext(pMedia);
        pMedia->setEscape(mbCouldEscape);
    }

    return err;

}

//--------------------------------------------------
//returns whether this tree is empty or not
//--------------------------------------------------
bool SmilTree::isEmpty()
{
    LOG4CXX_TRACE(amisSmilTreeLog, "isEmpty" );

    //test the root for NULL
    if (mpRoot == NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------
//print the tree
//--------------------------------------------------
void SmilTree::print()
{
    cout << endl << "TREE for *" << this->mSmilFilePath << "*" << endl;
    printNode(mpRoot, 0);
}

//--------------------------------------------------
/*!
 recursive function to print a node at a certain indentation level
 */
//--------------------------------------------------
void SmilTree::printNode(Node* pNode, int level)
{
    TimeContainerNode* p_time_container;

    int cnt;

    //print this node
    pNode->print(level);

    //if can have children
    if (pNode->getCategoryOfNode() == TIME_CONTAINER)
    {
        p_time_container = (TimeContainerNode*) pNode;

        //print the children
        for (cnt = 0; cnt < p_time_container->NumChildren(); cnt++)
        {
            printNode(p_time_container->getChild(cnt), level + 1);
        }

    }
}

//--------------------------------------------------
//assign skip options to the list
/*!
 this option list points to the list in the smil engine.  updates made there
 are automatically reflected here.
 */
//--------------------------------------------------
void SmilTree::setSkipOptionList(vector<amis::CustomTest*>* pSkipOptions)
{
    mpSkipOptions = pSkipOptions;

    //cout<<"added "<<mpSkipOptions->size()<<" skip options"<<endl;
}

//--------------------------------------------------
//add a content region to the list
//--------------------------------------------------
void SmilTree::addContentRegion(ContentRegionData region)
{
    //content regions are defined once at the top of the file, so there is no need to 
    //filter for duplicates
    mRegions.push_back(region);
}

//--------------------------------------------------
//return a pointer to the content regions list
//--------------------------------------------------
vector<ContentRegionData> SmilTree::getContentRegionList()
{
    return mRegions;
}

//--------------------------------------------------
/*!
 Based on the skip options and settings in the tree to which this node belongs, 
 should this node be played or not?
 */
//--------------------------------------------------
bool SmilTree::mustSkipOrEscapeNode(Node* pNode)
{
    LOG4CXX_TRACE(amisSmilTreeLog, "mustSkipOrEscapeNode" );

    //local variables
    int i;
    int len;
    bool return_value = false;
    string sys_skip_option;
    string node_skip_option;

    if (pNode == NULL)
        return return_value;

    len = mpSkipOptions->size();

    //test
    if (len == 0)
    {
        //	cerr<<"Warning!  no skip options for this tree"<<endl;
    }

    //if this is not a time container (seq or par) then we cannot escape or skip it
    if (pNode->getCategoryOfNode() != TIME_CONTAINER)
    {
        return_value = false;
    }

    //if this is our escapable node, and we have been asked to escape the current structure
    else if (pNode == mpEscapeNode && mbEscapeRequested == true)
    {
        return_value = true;
    }
    else
    {
        node_skip_option = ((TimeContainerNode*) pNode)->getSkipOption();

        for (i = 0; i < len; i++)
        {
            sys_skip_option = (*mpSkipOptions)[i]->getId();

            //if the id's match and willSkip is true, then this node must be skipped
            if (sys_skip_option.compare(node_skip_option) == 0
                    && (*mpSkipOptions)[i]->getCurrentState() == false)
            {
                return_value = true;
                break;
            }
        } //end of for-loop through skip options
    }

    if (return_value == true)
    {
        LOG4CXX_INFO(amisSmilTreeLog, "Skipping " << node_skip_option);
    }

    return return_value;
}

//--------------------------------------------------
/*!
 Remember that this node is elegible for being escaped
 */
//--------------------------------------------------
void SmilTree::setPotentialEscapeNode(Node* pNode)
{
    if (pNode == NULL)
    {
        mpEscapeNode = NULL;
    }
    else
    {
        //we can only set up a node for potential escapability if it is a seq or par
        if (pNode->getCategoryOfNode() == TIME_CONTAINER)
        {
            mpEscapeNode = (TimeContainerNode*) pNode;
            LOG4CXX_ERROR(amisSmilTreeLog, "setting up node for escaping " << pNode->getElementId() );
        }
    }

}

//--------------------------------------------------
//Go to a specific Id and start playback there
//--------------------------------------------------
amis::AmisError SmilTree::goToId(string id, amis::SmilMediaGroup* pMedia)
{

    LOG4CXX_TRACE(amisSmilTreeLog, "goToIdEmpty" );

    amis::AmisError err;
    err.setCode(amis::OK);

    string playback_data;

    if (mpRoot->getElementId().compare(id) == 0)
    {
        err = goFirst(pMedia);
        pMedia->setEscape(mbCouldEscape);
    }
    else
    {
        if (mpRoot->setAtId(id) == true)
        {
            this->mCurrentId = "";
            mpRoot->play(pMedia);
            pMedia->setId(this->mCurrentId);
            pMedia->setEscape(mbCouldEscape);
        }
        else
        {

            err.setCode(amis::NOT_FOUND);
            err.setMessage(
                    "The id *" + id + "* was not found in "
                            + getSmilFilePath());
        }
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

//--------------------------------------------------
//Set the smil file path that is the source for this tree
//--------------------------------------------------
void SmilTree::setSmilFilePath(string filepath)
{
    mSmilFilePath = filepath;
}

//--------------------------------------------------
//Get the smil file path that is the source for this tree
//--------------------------------------------------
string SmilTree::getSmilFilePath()
{
    return mSmilFilePath;
}

//--------------------------------------------------
/*!
 Get the current Id (the innermost nested id in the playback chain)
 */
//--------------------------------------------------
string SmilTree::getCurrentId()
{
    return mCurrentId;
}

//--------------------------------------------------
//Set the current Id
//--------------------------------------------------
void SmilTree::setCurrentId(string id)
{
    mCurrentId = id;
}

//--------------------------------------------------
//Get the most recent text Id 
//--------------------------------------------------
string SmilTree::getRecentTextId()
{
    return mRecentTextId;
}

//--------------------------------------------------
//Set the most recent text Id
//--------------------------------------------------
void SmilTree::setRecentTextId(string id)
{
    mRecentTextId = id;
}

//--------------------------------------------------
//return whether the current media data could be escaped
//--------------------------------------------------
bool SmilTree::getCouldEscape()
{
    return mbCouldEscape;
}

//--------------------------------------------------
//set whether the current media data could be escaped
//--------------------------------------------------
void SmilTree::setCouldEscape(bool escape)
{
    mbCouldEscape = escape;
}

//--------------------------------------------------
//get the smil duration
//--------------------------------------------------
unsigned int SmilTree::getSmilDuration()
{
    return mDuration;
}

//--------------------------------------------------
//set the smil duration from string
//--------------------------------------------------
void SmilTree::setSmilDuration(string durationStr)
{
    mDuration = stringToSeconds(durationStr);
}

//--------------------------------------------------
//set the smil duration from int
//--------------------------------------------------
void SmilTree::setSmilDuration(unsigned int duration)
{
    mDuration = duration;
}

// "0:0:0.5456" -> unsigned int
// "32.435" -> unsigned int
// "0:45:23" -> unsigned int
// "23:34:12.321" -> unsigned int
// "00:00:00" -> unsigned int
unsigned int SmilTree::stringToSeconds(string timeString)
{
    vector<string> numbers;
    istringstream ss(timeString);
    string numberStr;

    LOG4CXX_DEBUG( amisSmilTreeLog,
            "Trying to convert '" << timeString << "' to seconds");
    // Tokenize into string vector
    while (getline(ss, numberStr, ':'))
    {
        numbers.push_back(numberStr);
    }

    unsigned int duration = 0;
    double num;
    // Loop through the numbers and store them as seconds
    for (int i = 0; i < numbers.size(); i++)
    {
        istringstream ssnum(numbers[i]);
        ssnum >> num;
        duration += round(num * pow(60, numbers.size() - 1 - i));
    }

    LOG4CXX_DEBUG( amisSmilTreeLog,
            "Converted '" << timeString << "' to " << duration << " seconds");
    return duration;
}
