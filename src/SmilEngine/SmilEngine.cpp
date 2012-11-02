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

/**
 * @class amis::SmilEngine
 *
 * @brief This class handles the SMIL playback for a Daisy book.
 *
 * @details
 * The SmilEngine is initialized with two things:
 *
 * 1. a spine file (ncc, master.smil, or opf)
 * 2. list of skippability options and their default states
 *
 * The Smil engine renders a single file at a time.  when a file reaches its end,
 * the next file is loaded from the spine object.
 *
 * Traversal functions (previous, next, loadPosition, first, last, escape) return
 * a media group object representing the data to be rendered as a result of this
 * traversal request.
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <cctype>
#include <algorithm>

//PROJECT INCLUDES
#include "FilePathTools.h"
#include "Spine.h"
#include "SpineBuilder.h"
#include "SmilTreeBuilder.h"
#include "SmilTree.h"
#include "SmilEngine.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSmilEngineLog(
        log4cxx::Logger::getLogger("kolibre.amis.smilengine"));

using namespace std;
using namespace amis;

SmilEngine* SmilEngine::pinstance = 0;

SmilEngine* SmilEngine::Instance()
{
    if (pinstance == 0) // is it the first call?
    {
        pinstance = new SmilEngine; // create sole instance
    }
    return pinstance; // address of sole instance
}

void SmilEngine::DestroyInstance()
{
    delete pinstance;
}

/**
 * Default constructor
 */
SmilEngine::SmilEngine()
{
    //initialize member variables
    mSpineBuilder = new SpineBuilder();
    mSmilTreeBuilder = new SmilTreeBuilder();
    //pointers are NULL
    mpSpine = NULL;
    mpSmilTree = NULL;
    mpOldSmilTree = NULL;

    //Daisy version is undefined (no book loaded right now)
    mDaisyVersion = NO_VERSION_SET;

    //we are not at the end of a Smil Tree
    mbEndOfTree = false;
    mbLoadId = false;

    mSpineBuildStatus = amis::NOT_INITIALIZED;
    mSmilTreeBuildStatus = amis::NOT_INITIALIZED;

}

/**
 * The destructor closes the book
 */
SmilEngine::~SmilEngine()
{
    closeBook();
    mBookFile = "";
    mIdTarget = "";
    mLastPosition = "";
    delete(mSpineBuilder);
    delete(mSmilTreeBuilder);
}

/**
 * Open a book from a full path to: "ncc.html", "master.smil", or "*.opf"
 *
 * @param filePath File path to the book to open
 * @param pMedia Media group to store tree in
 * @return Return status of operation
 * @return amis::OK if open book succeeded
 */
amis::AmisError SmilEngine::openBook(std::string filePath, SmilMediaGroup* pMedia)
{

    amis::AmisError err;
    string file_ext = amis::FilePathTools::getExtension(filePath);
    string smilpath;

    //reset
    mBookFile = "";
    mIdTarget = "";
    mLastPosition = "";

    //convert the string to lower case before doing a comparison
    std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(),
            (int (*)(int))tolower);

if(    file_ext.compare(EXT_OPF) == 0)
    {
        mDaisyVersion = DAISY3;
    }
    else
    {
        mDaisyVersion = DAISY202;
    }

    //prepare for a new book to be loaded
    closeBook();

    //set the daisy version 
    mSmilTreeBuilder->setDaisyVersion(mDaisyVersion);

    //make a new spine for this book
    mpSpine = new Spine();

    //give filepath to spine builder class and request that it builds the spine
    //the spine will be stored in our variable "mpSpine", which we initialized here
    err = mSpineBuilder->createSpine(mpSpine, filePath);

    mSpineBuildStatus = err.getCode();

    if (mSpineBuildStatus == amis::OK)
    {

        //get the first file from the spine
        string smil_file = mpSpine->getFirstFile();

        //set end of tree to false
        mbEndOfTree = false;

        err = createTreeFromFile(smil_file, pMedia);
    }
    else
    {
        delete mpSpine;
        mpSpine = NULL;
        mSpineBuildStatus = amis::NOT_INITIALIZED;
    }

    return err;

}

/**
 * this function deletes the spine and smil tree that a book comprises
 * it also deletes the skip options
 *
 */
void SmilEngine::closeBook()
{
    if (mSpineBuildStatus != amis::NOT_INITIALIZED)
    {
        LOG4CXX_DEBUG(amisSmilEngineLog, "deleting spine");
        delete mpSpine;
        mpSpine = NULL;
    }

    if (mSmilTreeBuildStatus != amis::NOT_INITIALIZED)
    {
        LOG4CXX_DEBUG(amisSmilEngineLog, "deleting smiltree");
        delete mpSmilTree;
        mpSmilTree = NULL;
    }

    clearAllSkipOptions();

    mSpineBuildStatus = amis::NOT_INITIALIZED;
    mSmilTreeBuildStatus = amis::NOT_INITIALIZED;

}

/**
 * build a tree from file and go to the first, last, or specified (by id) node
 *
 * @param filepath Filepath to the index file
 * @param pMedia Media group to store the tree in
 * @return amis::OK if the tree was successfully created
 */
amis::AmisError SmilEngine::createTreeFromFile(std::string filepath,
        SmilMediaGroup* pMedia)
{
    //local variables

    if (mpSmilTree != NULL)
    {
        if (mpOldSmilTree != NULL)
            delete mpOldSmilTree;
        mpOldSmilTree = mpSmilTree;
    }

    //make a new smil tree for this book
    mpSmilTree = new SmilTree();

    //build a smil tree from the selected Smil file
    LOG4CXX_DEBUG(amisSmilEngineLog, "opening " << filepath);
    mSmilTreeBuilder->setDaisyVersion(mDaisyVersion);
    amis::AmisError err = mSmilTreeBuilder->createSmilTree(mpSmilTree, filepath);

    mSmilTreeBuildStatus = err.getCode();

    // Recover the old tree if we failed to load the new one
    if (mSmilTreeBuildStatus != amis::OK)
    {
        if (mpOldSmilTree != NULL)
        {

            LOG4CXX_WARN(amisSmilEngineLog, "Recovering old tree");
            mSmilTreeBuildStatus = amis::OK;
            mpSmilTree = mpOldSmilTree;
            mpOldSmilTree = NULL;
            return err;

        }
        else
        {
            LOG4CXX_ERROR(amisSmilEngineLog, "Failed to recover old tree");
            return err;
        }

    }

    if (mSmilTreeBuildStatus != amis::OK)
        return err;

    //refresh the smil tree with the list of global skip options

    //loop through all global skip options and update the Smil Tree with the user's preference

    //add each skip option to the smil tree
    //the skip options came from outside the smil engine
    LOG4CXX_DEBUG(amisSmilEngineLog, "Setting skip options");
    mpSmilTree->setSkipOptionList(&mSkipOptions);

    //are we going to a specific element ID?
    if (mbLoadId == true)
    {
        LOG4CXX_DEBUG(amisSmilEngineLog, "Loading correct id " << mIdTarget);
        mbLoadId = false;
        err = mpSmilTree->goToId(mIdTarget, pMedia);
    }

    else
    {
        //if we should NOT go to the end of the tree (if we are entering the tree from the top)
        if (mbEndOfTree == false)
        {
            LOG4CXX_DEBUG(amisSmilEngineLog, "Going to first node");
            err = mpSmilTree->goFirst(pMedia);
        }
        //else, if we should go to the end of the tree (if we are entering the tree from the end)
        else
        {
            LOG4CXX_DEBUG(amisSmilEngineLog, "Going to last node");
            err = mpSmilTree->goLast(pMedia);
        }
    }

    //printTree();

    return err;
}

/**
 * Go to the next node in the smil tree.
 * if the next node is not found (for ex. if it is being skipped or we are at the
 * end of the tree), load the next smil file and try again.
 * repeat until found or end of book occurs
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] pMedia
 * points to an initialized object
 *
 * @return amis::OK if jump was successfull
 * @return amis::AT_END if the last node was reached
 */
amis::AmisError SmilEngine::next(SmilMediaGroup* pMedia)
{
    amis::AmisError err;

    if (mSpineBuildStatus != amis::OK || mSmilTreeBuildStatus != amis::OK)
    {
        err.setCode(amis::NOT_INITIALIZED);
        err.setMessage(MSG_BOOK_NOT_OPEN);
    }
    else
    {
        err = mpSmilTree->goNext(pMedia);

        //need a loop here
        //case:
        //skippable option set to not render
        //entire file is skippable
        //when you enter that file, it comes back as LIST_END
        //need to go to the next file and try again
        while (err.getCode() == amis::AT_END)
        {
            //spine-next
            string smil_path;

            //get the next smil file in the spine
            smil_path = mpSpine->getNextFile();

            //if this getNextFile call was successful
            if (mpSpine->getStatus() != amis::AT_END)
            {
                mbEndOfTree = false;
                LOG4CXX_WARN(amisSmilEngineLog, "opening " << smil_path);
                err = createTreeFromFile(smil_path, pMedia);
                recordPosition();
            }
            else
            {
                err.setCode(amis::AT_END);
                err.setMessage(MSG_END_OF_BOOK);
                break;
            }

            if (err.getCode() == amis::AT_END)
                LOG4CXX_WARN(amisSmilEngineLog,
                        "Got AT_END opening " << smil_path);
        }
        if (err.getCode() == amis::OK)
        {
            //here we are sending out node playback data, so record our current position
            recordPosition();
        }

    }

    err.setSourceModuleName(amis::module_SmilEngine);
    return err;
}

/**
 * Go to the previous node in the smil tree.
 * If the previous node is not found (for ex. if it is being skipped or we are
 * at the beginning of the tree), load the previous smil file and try again.
 * repeat until found or beginning of book occurs
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] pMedia
 * points to an initialized object

 * @return amis::OK on success
 * @return amis::AT_BEGINNING if we already is at first node
 */
amis::AmisError SmilEngine::previous(SmilMediaGroup* pMedia)
{
    amis::AmisError err;

    if (mSpineBuildStatus != amis::OK || mSmilTreeBuildStatus != amis::OK)
    {
        err.setCode(amis::NOT_INITIALIZED);
        err.setMessage(MSG_BOOK_NOT_OPEN);
    }
    else
    {
        err = mpSmilTree->goPrevious(pMedia);

        while (err.getCode() == amis::AT_BEGINNING)
        {
            //spine-next
            string smil_path;

            //get the previous smil file in the spine
            smil_path = mpSpine->getPreviousFile();

            //if this getPreviousFile call was successful
            if (mpSpine->getStatus() != amis::AT_BEGINNING)
            {
                mbEndOfTree = true;
                LOG4CXX_WARN(amisSmilEngineLog, "opening " << smil_path);
                err = createTreeFromFile(smil_path, pMedia);
                recordPosition();
            }
            else
            {
                err.setCode(amis::AT_BEGINNING);
                err.setMessage(MSG_BEGINNING_OF_BOOK);
                break;
            }

            if (err.getCode() == amis::AT_BEGINNING)
                LOG4CXX_WARN(amisSmilEngineLog,
                        "Got AT_BEGINNING opening " << smil_path);

        }
        if (err.getCode() == amis::OK)
        {
            //here we are sending out node playback data, so record our current position
            recordPosition();
        }

    }

    err.setSourceModuleName(amis::module_SmilEngine);
    return err;
}

/**
 * Go to the first node in the loaded smil tree
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] pMedia
 * points to an initialized object

 * @return amis::OK if the first node was opened
 * @return amis::NOT_INITIALIZED if book is not yet open
 */
amis::AmisError SmilEngine::first(SmilMediaGroup* pMedia)
{
    amis::AmisError err;

    if (mSpineBuildStatus != amis::OK || mSmilTreeBuildStatus != amis::OK)
    {
        err.setCode(amis::NOT_INITIALIZED);
        err.setMessage(MSG_BOOK_NOT_OPEN);
    }
    else
    {
        err = mpSmilTree->goFirst(pMedia);

        //here we are sending out node playback data, so record our current position
        recordPosition();
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

/**
 * Go to the last node in the loaded smil tree
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] pMedia
 * points to an initialized object
 *
 * @return amis::OK on success
 * @return amis::NOT_INITIALIZED if book is not open
 */
amis::AmisError SmilEngine::last(SmilMediaGroup* pMedia)
{
    amis::AmisError err;

    if (mSpineBuildStatus != amis::OK || mSmilTreeBuildStatus != amis::OK)
    {
        err.setCode(amis::NOT_INITIALIZED);
        err.setMessage(MSG_BOOK_NOT_OPEN);
    }
    else
    {
        err = mpSmilTree->goLast(pMedia);

        //here we are sending out node playback data, so record our current position
        recordPosition();
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

/**
 * Print a list of skip options
 */
void SmilEngine::printSkipOptions()
{
    unsigned int i;

    cerr << "Smil Engine Skip Options" << endl;

    for (i = 0; i < mSkipOptions.size(); i++)
    {
        cerr << "\tName = " << mSkipOptions[i]->getId() << endl;
        cerr << "\tWill play = " << mSkipOptions[i]->getCurrentState() << endl;
        cerr << "\tDefault = " << mSkipOptions[i]->getDefaultState() << endl;
        cerr << endl;
    }
}

/**
 * Add a skip option to the smil engine.
 *
 * @note
 * Call this function before opening a book
 *
 * @param pOption The skip option we wish to add
 */
void SmilEngine::addSkipOption(amis::CustomTest* pOption)
{

    //create a new custom test object and copy the data over
    //local variables
    amis::CustomTest* p_skip_option;

    p_skip_option = new amis::CustomTest();

    p_skip_option->setId(pOption->getId());
    p_skip_option->setCurrentState(pOption->getCurrentState());
    p_skip_option->setBookStruct(pOption->getBookStruct());
    p_skip_option->setDefaultState(pOption->getDefaultState());
    p_skip_option->setOverride(pOption->getOverride());

    //add the new entry to the skipOptions array
    mSkipOptions.push_back(p_skip_option);

}

/**
 * Change a skip option
 *
 * @param[in] id
 * id of the custom test which is being changed
 *
 * @param[in] newState
 * true = will play, false = will skip
 *
 * @return Returns true if the option was changed, false if not found
 */
bool SmilEngine::changeSkipOption(std::string id, bool newState)
{
    unsigned int i;
    bool b_found;

    b_found = false;
    //for-loop through the skip options array to see if this option already is present
    for (i = 0; i < mSkipOptions.size(); i++)
    {
        //if this option is present
        //if (mSkipOptions[i].mId.compare (option) == 0)
        if (mSkipOptions[i]->getId().compare(id) == 0)
        {
            //set its "willSkip" value equal to the user preference and break from the loop
            //mSkipOptions[i].mbWillSkip = value;
            mSkipOptions[i]->setCurrentState(newState);
            b_found = true;
            break;
        }
    }

    return b_found;
}


/**
 * Delete objects in the skip options collection
 */
void SmilEngine::clearAllSkipOptions()
{
    //delete the skip option vector
    int sz = this->mSkipOptions.size();
    amis::CustomTest* tmp_ptr;

    for (int i = sz - 1; i > 0; i--)
    {
        tmp_ptr = this->mSkipOptions[i];

        delete tmp_ptr;

        mSkipOptions.pop_back();

    }

    mSkipOptions.clear();
}

/**
 * if the next node outside of the current escapable structure is not found
 * (for ex. if it is being skipped or we are
 * at the end of the tree), load the next smil file and try again.
 * repeat until found or end of book occurs
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] pMedia
 * points to an initialized object
 *
 * @return amis::OK if everything succeded
 */
amis::AmisError SmilEngine::escapeCurrent(SmilMediaGroup* pMedia)
{
    amis::AmisError err;

    if (mSpineBuildStatus != amis::OK || mSmilTreeBuildStatus != amis::OK)
    {
        err.setCode(amis::NOT_INITIALIZED);
        err.setMessage(MSG_BOOK_NOT_OPEN);
    }
    else
    {
        err = mpSmilTree->escapeStructure(pMedia);

        while (err.getCode() == amis::AT_END)
        {
            //spine-next
            string smil_path;

            //get the next smil file in the spine
            smil_path = mpSpine->getNextFile();

            //if this getNextFile call was successful
            if (mpSpine->getStatus() != amis::AT_END)
            {
                LOG4CXX_WARN(amisSmilEngineLog, "opening " << smil_path);
                mbEndOfTree = false;
                err = createTreeFromFile(smil_path, pMedia);
                recordPosition();
            }
            else
            {
                err.setCode(amis::AT_END);
                err.setMessage(MSG_END_OF_BOOK);
                break;
            }

            if (err.getCode() == amis::AT_END)
                LOG4CXX_WARN(amisSmilEngineLog,
                        "Got AT_END opening " << smil_path);

        }
        if (err.getCode() == amis::OK)
        {
            //here we are sending out node playback data, so record our current position
            recordPosition();
        }
    }

    err.setSourceModuleName(amis::module_SmilEngine);

    return err;
}

/**
 * go to a specified position in the book.
 *
 * @param[out] pMedia
 * playback data delivered as a response to the request is stored here
 *
 * @param[in] positionUri
 * string of the position to load.  stated as a full path to the node, as in c:\\book\\file.smil#target
 *
 * @param[in] pMedia
 * points to an initialized object
 *
 * @return amis::OK if loading of position succeeded
 */
amis::AmisError SmilEngine::loadPosition(std::string positionUri,
        SmilMediaGroup* pMedia)
{
    amis::AmisError err;
    string err_msg;
    err_msg = "*" + positionUri + "* was not found.";

    err.setSourceModuleName(amis::module_SmilEngine);

    //the bookmark file
    string position = positionUri;

    LOG4CXX_DEBUG(amisSmilEngineLog, "loading " << positionUri);

    static bool recoverOnError = true;

    //save the target
    mIdTarget = amis::FilePathTools::getTarget(positionUri);

    //clear the target from the file
    positionUri = amis::FilePathTools::clearTarget(positionUri);

    if (mpSpine->goToFile(positionUri) == true)
    {
        //we are not at the end of the tree
        mbEndOfTree = false;
        mbLoadId = true;

        LOG4CXX_DEBUG(amisSmilEngineLog,
                "calling createTreeFromFile for " << positionUri);
        err = createTreeFromFile(positionUri, pMedia);

        //position not found is the likely error here
        //so load our last good position
        if (err.getCode() != amis::OK)
        {
            if (recoverOnError == true)
            {
                recoverOnError = false;
                LOG4CXX_DEBUG(amisSmilEngineLog,
                        "Calling loadPosition for old position " << mLastPosition);
                err = loadPosition(mLastPosition, pMedia);
                //there isn't a serious error but let the user know their request 
                //was invalid and that's why we didn't load it
                err.setCode(amis::NOT_FOUND);
                err.setMessage(err_msg);
            }
            else
            {
                LOG4CXX_DEBUG(amisSmilEngineLog,
                        "Not calling loadPosition for old position " << mLastPosition);
                return err;
            }
        }
        else
        {
            recordPosition();
        }

    }

    //else, this Smil file is not found
    //don't even try to change position
    else
    {
        err.setCode(amis::NOT_FOUND);
        err.setMessage(err_msg);
    }

    recoverOnError = true;

    return err;

}


/**
 * Record our current position
 *
 * @pre
 * this position was just delivered as output data
 */
void SmilEngine::recordPosition()
{
    //remember the last position so we can load it again if gotoId fails
    mLastPosition = mpSmilTree->getSmilFilePath();
    mLastPosition += "#" + mpSmilTree->getCurrentId();
}

/**
 * print the tree
 *
 * this function is useful in debugging
 */
void SmilEngine::printTree()
{
    if (mpSmilTree != NULL)
    {
        mpSmilTree->print();
    }
}

/**
 * get the source path for the currently open smil file
 *
 * @return
 */
string SmilEngine::getSmilSourcePath()
{
    if (mpSmilTree != NULL)
    {
        return this->mpSmilTree->getSmilFilePath();
    }
    else
    {
        return "";
    }
}

/**
 * Get meta data for currently open file
 *
 * @param metaname Name for meta data
 * @return Returns metadata from currently open smil file
 */
std::string SmilEngine::getMetadata(std::string metaname)
{
    if (mSmilTreeBuildStatus != amis::OK)
        return "";

    return mSmilTreeBuilder->getMetadata(metaname);
}

/**
 * Get daisy version
 *
 * @return Return daisy version from currently open book
 */
int SmilEngine::getDaisyVersion()
{
    return mDaisyVersion;
}

/**
 * Get smil file path
 *
 * @param id Index for node to get file path for
 * @return Returns smil file path from currently open smil file
 */
string SmilEngine::getSmilFilePath(int id)
{
    if (mpSpine == NULL)
        return "";

    return mpSpine->getSmilFilePath(id);
}

/**
 * Get the number of smil files
 *
 * @return Returs the number of smil files from currently open book
 */
int SmilEngine::getNumberOfSmilFiles()
{
    if (mpSpine == NULL)
        return 0;

    return mpSpine->getNumberOfSmilFiles();
}
