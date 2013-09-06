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

/**
 * @class amis::DaisyHandler
 *
 * @brief This class encapsulates the Daisy book handling functions
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

// AmisCommon
#include "Bookmarks.h"
#include "BookmarksReader.h"
#include "BookmarksWriter.h"
#include "FilePathTools.h"
#include "Media.h"
#include "Metadata.h"
#include "OpfItemExtract.h"
#include "TitleAuthorParse.h"

// DaisyHandler
#include "AmisConstants.h"
#include "DaisyHandler.h"
#include "HistoryRecorder.h"

// NavParse
#include "NavParse.h"

// SmilEngine
#include "BinarySmilSearch.h"
#include "ContentNode.h"
#include "SmilEngine.h"
#include "SmilTreeBuilder.h"
#include "SpineBuilder.h"

#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <iterator>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisDaisyHandlerLog(
        log4cxx::Logger::getLogger("kolibre.amis.daisyhandler"));

// Enable or disable a navi-history level
#define NAVIHISTORY_LEVEL_ENABLE 0

#define AUTONAVI_DELAY_SECONDS 30

using namespace amis;
using namespace std;

std::string stringReplaceAll(string haystack, string needle, string replace)
{
    int pos = 0;

    while (pos > -1)
    {
        pos = haystack.find(needle);

        if (pos > -1)
        {
            haystack.replace(pos, needle.length(), replace);
        }
    }

    return haystack;
}

amis::DaisyHandler* DaisyHandler::pinstance = 0;

void *open_thread(void *handler);

// Local helper functions
int currentSection(NavPoint* root, const int currentPlayOrder, bool& found);
int countSections(NavPoint* root);
int currentNavContainerIdx(NavContainer* list, const int currentPlayOrder);
long parseTime(string str);

/**
 * Method for fetching the DaisyHandler instance
 *
 * @return The instance of DaisyHandler
 */
DaisyHandler* DaisyHandler::Instance()
{
    if (pinstance == 0) // is it the first call?
    {
        pinstance = new DaisyHandler; // create sole instance
    }
    return pinstance; // address of sole instance
}

/**
 * Method for Destroing the singleton instance
 */
void DaisyHandler::DestroyInstance()
{
    delete pinstance;
}

/**
 * DaisyHandler constructor
 */
DaisyHandler::DaisyHandler()
{
    mpBmk = NULL;
    mBmkPath = "";
    mCurrentBookmark = -1;
    mCurrentPage = "";

    mpHst = NULL;
    mpCurrentMedia = NULL;
    currentPos = new amis::PositionData();
    mpTitle = NULL;

    mbStartAtLastmark = true;

    naviDirection = FORWARD;

    // callback stuff
    PlayFunction = NULL;
    OOPlayFunction = NULL;
    OOPlayFunctionData = NULL;

    handlerMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(handlerMutex, NULL);

    setState(HANDLER_CLOSED);

    handlerThreadActive = false;
    currentNaviLevel = PHRASE;
}

/**
 * DaisyHandler destructor
 */
DaisyHandler::~DaisyHandler()
{
    // NavParse will take care of destroying mpTitle
    /*
    if (mpTitle != NULL)
    {
        mpTitle->destroyContents();
        delete mpTitle;
    }
    */

    if (mpCurrentMedia != NULL)
    {
        delete mpCurrentMedia;
        mpCurrentMedia = NULL;
    }

    if (mpBmk != NULL)
    {
        delete mpBmk;
        mpBmk = NULL;
    }

    if (mpHst != NULL)
    {
        delete mpHst;
        mpHst = NULL;
    }

    free(handlerMutex);

    //destroy objects!
    //LOG4CXX_DEBUG(amisDaisyHandlerLog, "destorying smilengine");
    SmilEngine::Instance()->DestroyInstance();

    //LOG4CXX_DEBUG(amisDaisyHandlerLog, "destroying navparse");
    NavParse::Instance()->DestroyInstance();

    //LOG4CXX_DEBUG(amisDaisyHandlerLog, "destorying metadata");
    amis::Metadata::Instance()->DestroyInstance();
    delete currentPos;
}

/**
 * Set the PlayFunction callback
 *
 * @param ptr A pointer to a play function
 */
void DaisyHandler::setPlayFunction(bool (*ptr)(std::string, long long, long long))
{
    PlayFunction = ptr;
}

/**
 * Set the PlayFunction callback with data pointer
 *
 * @param ptr A pointer to a play function
 * @param data A data pointer
 */
void DaisyHandler::setPlayFunction(
        bool (*ptr)(std::string, long long, long long, void*), void *data)
{
    OOPlayFunction = ptr;
    OOPlayFunctionData = data;
}

/**
 * Call the play function
 *
 * @param filename File to play
 * @param startms Start time of range
 * @param stopms Stop
 * @param stopms NULL Null for play to EOF
 * @return Returns true on success
 */
bool DaisyHandler::callPlayFunction(std::string filename, long long startms,
        long long stopms)
{
    if (PlayFunction != NULL)
        return PlayFunction(filename, startms, stopms);
    return false;
}

/**
 * Call the play function with data pointer in context
 *
 * @param filename File to play
 * @param startms Start time of range
 * @param stopms Stop time
 * @return Returns true on success
 */
bool DaisyHandler::callOOPlayFunction(std::string filename, long long startms,
        long long stopms)
{
    if (OOPlayFunction != NULL)
        return OOPlayFunction(filename, startms, stopms, OOPlayFunctionData);
    return false;
}

/**
 * Set a new state for the handler
 *
 * @param state The new state
 */
void DaisyHandler::setState(HandlerState state)
{
    pthread_mutex_lock(handlerMutex);
    handlerState = state;
    pthread_mutex_unlock(handlerMutex);
}

/**
 * Get the state of the handler
 *
 * @return Returns the current state
 */
DaisyHandler::HandlerState DaisyHandler::getState() const
{
    HandlerState currentState = HANDLER_CLOSED;
    pthread_mutex_lock(handlerMutex);
    currentState = handlerState;
    pthread_mutex_unlock(handlerMutex);
    return currentState;
}

/**
 * Get the file path of current file
 *
 * @return Returns the file path set in the daisyhandler
 */
std::string DaisyHandler::getFilePath() const
{
	std::string filename = "";
    pthread_mutex_lock(handlerMutex);
    filename = mFilePath;
    pthread_mutex_unlock(handlerMutex);
    return filename;
}

/**
 * Close the current media
 */
void DaisyHandler::closeBook()
{
    if (mpCurrentMedia != NULL)
    {
        delete mpCurrentMedia;
        mpCurrentMedia = NULL;
    }

    if (mpBmk != NULL)
    {
        delete mpBmk;
        mpBmk = NULL;
    }

    if (mpHst != NULL)
    {
        delete mpHst;
        mpHst = NULL;
    }

    // Close book
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "closing SmilEngine");
    SmilEngine::Instance()->closeBook();
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "closing NavParse");
    NavParse::Instance()->close();
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "closing Metadata");
    Metadata::Instance()->close();

    setState(HANDLER_CLOSED);
}

/**
 * Open new media
 *
 * @param url URL to the media
 * @return Returns true on success
 */
bool DaisyHandler::openBook(std::string url)
{
    HandlerState currentState = getState();
    amis::AmisError err;

    switch (currentState)
    {
    case HANDLER_OPENING:
        err.setCode(amis::UNDEFINED_ERROR);
        err.setMessage("Open thread already running");
        reportGeneralError(err);
        return false;
    case HANDLER_OPEN:
        // check if the requested url is the same as the current one
        if (mFilePath.compare(url) == 0)
        {
            err.setCode(amis::UNDEFINED_ERROR);
            err.setMessage("Book already open");
            reportGeneralError(err);
            return false;
        }
    case HANDLER_READY:
        closeBook();
        break;
    case HANDLER_ERROR:
        closeBook();
        break;

    default:
        break;
    }

    mFilePath = url;
    mBookInfo.mUri = url;

    //delete old bookmarks
    if (mpBmk != NULL)
    {
        delete mpBmk;
        mpBmk = NULL;
    }

    //delete old history
    if (mpHst != NULL)
    {
        delete mpHst;
        mpHst = NULL;
    }

    mpHst = new HistoryRecorder();

    // check if url is empty
    if (url == "")
    {
        err.setCode(amis::NOT_FOUND);
        err.setMessage("File not found: " + url);
        reportGeneralError(err);
        setState(HANDLER_ERROR);
        return false;
    }

    setState(HANDLER_OPENING);

    if (pthread_create(&handlerThread, NULL, open_thread, this) == 0)
    {
        handlerThreadActive = true;
#ifdef WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
        return true;
    }

    setState(HANDLER_ERROR);
    err.setCode(amis::UNDEFINED_ERROR);
    err.setMessage("Failed to start open thread");
    reportGeneralError(err);
    return false;
}

/**
 * Open a book in a new thread
 *
 * @param handler A handler pointer
 */
void *open_thread(void *handler)
{
    DaisyHandler *h = (DaisyHandler *) handler;

    std::string filename = h->getFilePath();

    amis::AmisError err;
    SmilMediaGroup* pMedia = NULL;
    pMedia = new SmilMediaGroup();

    // load the smil tree
    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "openthread: opening " << filename << " in smilengine");
    err = SmilEngine::Instance()->openBook(filename, pMedia);

    if (err.getCode() != amis::OK)
    {
        h->setState(DaisyHandler::HANDLER_ERROR);
        h->reportGeneralError(err);
        pthread_exit(NULL);

        return NULL;
    }


    // get the navigationurl from the opf file if we are opening a DAISY3 book
    std::string ext = amis::FilePathTools::getExtension(filename);
    if (ext.compare("opf") == 0)
    {
        amis::OpfItemExtract opf_extr;
        filename = opf_extr.getItemHref(filename, "ncx");
        if (filename.size() == 0)
            filename = opf_extr.getItemHref(filename, "NCX");
    }

    // load the navigation structure
    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "openthread: opening " << filename << " in navparse");
    err = NavParse::Instance()->open(filename);

    if (err.getCode() != amis::OK)
    {
        h->setState(DaisyHandler::HANDLER_ERROR);
        h->reportGeneralError(err);

        pthread_exit(NULL);
        return NULL;

    }

    h->setState(DaisyHandler::HANDLER_OPEN);

    pthread_exit(NULL);
    return NULL;
}

/**
 * Wait for child threads to join
 */
void DaisyHandler::join_threads()
{
    // Check if we need to join the open thread
    if (handlerThreadActive)
    {
        // Join the open thread
        LOG4CXX_DEBUG(amisDaisyHandlerLog, "Joining open thread");
        pthread_join(handlerThread, NULL);
        handlerThreadActive = false;
    }
}

/**
 * Wait for previous books to close and setup new Book
 *
 * @return Returns true on success
 */
bool DaisyHandler::setupBook()
{
    HandlerState currentState = getState();
    bool ret = false;

    join_threads();
    if (currentState != HANDLER_OPEN)
        return false;

    AmisError err;
    SmilMediaGroup* pMedia = new SmilMediaGroup;
    SmilEngine::Instance()->first(pMedia);
    // Skip the title (should always be the first element)
    //SmilEngine::Instance()->next(pMedia);

    // Load the book metadata	
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "loading metadata..");
    err = Metadata::Instance()->openFile(getFilePath());

    if (err.getCode() != amis::OK)
    {
        reportGeneralError(err);
        setState(DaisyHandler::HANDLER_ERROR);
    }

    //look for a bookmarks file, create one if does not exist
    string uid = Metadata::Instance()->getMetadata("dc:Identifier");

    if (uid.size() == 0)
    {
        uid = Metadata::Instance()->getMetadata("dc:identifier");
    }
    if (uid.size() == 0)
    {
        uid = Metadata::Instance()->getMetadata("ncc:identifier");
    }
    if (uid.size() == 0)
    {
        uid = "unknown";
    }

    string checksum = Metadata::Instance()->getChecksum();

    //only record bookmarks if this book has a UID
    //set uid to checksum in case the book does not have a uid
    if (uid.size() == 0)
        uid = checksum;

    //otherwise there's no consistent way to track the bookmark files
    if (uid.size() == 0)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "Bookmark setup not possible, unable to determine unique identifier for book");
        err.setCode(amis::NOT_FOUND);
        err.setMessage("Failed to find UID of book");
        err.setSourceModuleName(amis::module_DaisyHandler);
        reportGeneralError(err);
    }
    else
    {
        //this function also initializes the mpBmk object
        setupBookmarks(uid, checksum);
    }

    LOG4CXX_DEBUG(amisDaisyHandlerLog, "getting navmodel..");
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    //sync the skip options between nav and smil
    if (p_nav_model != NULL)
    {
        for (unsigned int i = 0; i < p_nav_model->getNumberOfCustomTests(); i++)
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "adding skipoption: " << p_nav_model->getCustomTest(i)->getId() << " - " << ((p_nav_model->getCustomTest(i)->getDefaultState() == true) ? "render" : "skip"));
            SmilEngine::Instance()->addSkipOption(
                    p_nav_model->getCustomTest(i));
        }
    }

    LOG4CXX_INFO(amisDaisyHandlerLog, "getting title..");

    if (NavParse::Instance()->getNavModel()->getDocTitle() != NULL)
    {
        mpTitle = NavParse::Instance()->getNavModel()->getDocTitle();
        if (mpTitle->getText() != NULL)
        {
            mBookInfo.mTitle = mpTitle->getText()->getTextString();
            LOG4CXX_INFO(amisDaisyHandlerLog, "Book title: " << mBookInfo.mTitle);
        }
    }

    //load lastmark
    if (mpBmk != NULL && mbStartAtLastmark == true)
    {
        PositionData* pos_data = NULL;
        pos_data = mpBmk->getLastmark();

        if (pos_data != NULL)
        {
            if (pos_data->mUri.size() > 0)
            {
                string pos = FilePathTools::goRelativePath(this->mFilePath,
                        pos_data->mUri);
                string ncxref = pos_data->mNcxRef;
                string audioref = pos_data->mAudioRef;
                int playorder = pos_data->mPlayOrder;

                LOG4CXX_INFO(amisDaisyHandlerLog,
                        "jumping to lastmark pos:" << pos << " ref:" << audioref);

                ret = loadSmilContent(pos, audioref);

                if (ret == true)
                {
                    // Locate the correct position in the navmap
                    syncNavModel(ncxref, playorder);
                }

            }
        }
        else
        {
            playMediaGroup(pMedia);
            ret = false;
        }
    }
    else
    {
        playMediaGroup(pMedia);
        ret = false;
    }

    currentNaviLevel = PHRASE;
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    readBookInfo();
    readPageNavPoints();
    readSectionNavPoints();

    // returns true if we have a lastmark, false if we don't
    return ret;

}

/**
 * Jump forward in history
 *
 * @return Returns true on success
 */
bool DaisyHandler::nextHistory()
{
    if (mpHst == NULL)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "mpHst == NULL");
        return false;
    }

    // Remember the current navi direction
    naviDirection = FORWARD;

    PositionData *p_pos = NULL;
    bool ret1 = mpHst->getNext(&p_pos);

    if (p_pos != NULL)
    {
        string content_url = amis::FilePathTools::goRelativePath(mFilePath,
                p_pos->mUri);
        string audioRef = p_pos->mAudioRef;
        bool ret2 = loadSmilContent(content_url, audioRef);

        if (ret2 == true)
        {
            // Locate the correct position in the navmap
            syncNavModel(p_pos->mNcxRef, p_pos->mPlayOrder);
        }

        return (ret1 && ret2);
    }
    return false;
}

/**
 * Jump backwards in history. This can be used to trace a path of
 * user interaction.
 *
 * @return Returns true on success
 */
bool DaisyHandler::previousHistory()
{
    if (mpHst == NULL)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "mpHst == NULL");
        return false;
    }

    // Remember the current navi direction
    naviDirection = BACKWARD;

    PositionData *p_pos = NULL;
    bool ret1 = mpHst->getPrevious(&p_pos);

    if (p_pos != NULL)
    {
        string content_url = amis::FilePathTools::goRelativePath(mFilePath,
                p_pos->mUri);
        string audioRef = p_pos->mAudioRef;
        bool ret2 = loadSmilContent(content_url, audioRef);

        if (ret2 == true)
        {
            // Locate the correct position in the navmap
            syncNavModel(p_pos->mNcxRef, p_pos->mPlayOrder);
        }

        return (ret1 && ret2);
    }
    return false;
}

/**
 * Restore last history
 *
 * @return Returns true on success
 */
bool DaisyHandler::loadLastHistory()
{
    if (mpHst == NULL)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "mpHst == NULL");
        return false;
    }

    PositionData *p_pos = NULL;
    bool ret1 = mpHst->getLast(&p_pos);

    if (p_pos != NULL)
    {
        string content_url = amis::FilePathTools::goRelativePath(mFilePath,
                p_pos->mUri);
        string audioRef = p_pos->mAudioRef;
        bool ret2 = loadSmilContent(content_url, audioRef);

        if (ret2 == true)
        {
            // Locate the correct position in the navmap
            syncNavModel(p_pos->mNcxRef, p_pos->mPlayOrder);
        }

        return (ret1 && ret2);
    }
    return false;
}

/**
 * Print the recorded history
 */
void DaisyHandler::printHistory()
{
    if (mpHst == NULL)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "mpHst == NULL");
        return;
    }

    mpHst->printItems();
}

/**
 * Get the number of custom tests configured
 *
 * @return Returns the number of custom tests found
 * @retval -1 Nav model not instatiated
 */
int DaisyHandler::numCustomTests()
{
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int num = -1;

    // list the skip options, return count
    if (p_nav_model != NULL)
    {
        if (p_nav_model->getNumberOfCustomTests() != 0)
            num = p_nav_model->getNumberOfCustomTests();
    }

    return num;
}

/**
 * Get custom test from index
 *
 * @param idx The index of wanted custom test
 * @return Returns the string
 */
std::string DaisyHandler::getCustomTestId(unsigned int idx)
{
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    string name = "<unknown>";

    // return name of customtest with number idx
    if (p_nav_model != NULL)
    {
        if (idx >= 0 && idx < p_nav_model->getNumberOfCustomTests())
            name = p_nav_model->getCustomTest(idx)->getId();
    }

    return name;
}

/**
 * Get custom test state for id
 *
 * @param idx Id of wanted custom test
 * @return Returns the state id
 * @return -1 if setting does not exist
 * @return 0 for false
 * @return 1 for true
 */
int DaisyHandler::getCustomTestState(unsigned int idx)
{
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int currentState = -1;

    // return state of customtest with number idx
    if (p_nav_model != NULL)
    {
        if (idx >= 0 && idx < p_nav_model->getNumberOfCustomTests())
            currentState =
                    p_nav_model->getCustomTest(idx)->getCurrentState() == true ?
                            1 : 0;
    }

    return currentState;
}

/**
 * Get custom test state for string id
 *
 * @param id
 * @return
 * @return -1 if setting does not exist
 * @return 0 for false
 * @return 1 for true
 */
int DaisyHandler::getCustomTestState(std::string id)
{
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int currentState = -1;

    // return state of customtest with number idx
    if (p_nav_model != NULL)
    {
        for (unsigned int i = 0; i < p_nav_model->getNumberOfCustomTests(); i++)
            if (p_nav_model->getCustomTest(i)->getId().compare(id))
                currentState =
                        p_nav_model->getCustomTest(i)->getCurrentState()
                                == true ? 1 : 0;
    }

    return currentState;
}

/**
 * Set custom test state
 *
 * @param idx
 * @param state
 * @return
 * @return -1 if setting does not exist
 * @return 0 for false
 * @return 1 for true
 */
int DaisyHandler::setCustomTestState(unsigned int idx, bool state)
{
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int currentState = -1;

    // set state of customtest with number idx
    if (p_nav_model != NULL)
    {
        if (idx >= 0 && idx < p_nav_model->getNumberOfCustomTests())
        {

            string id = p_nav_model->getCustomTest(idx)->getId();

            if (SmilEngine::Instance()->changeSkipOption(id, state))
            {
                p_nav_model->getCustomTest(idx)->setCurrentState(state);
                currentState =
                        p_nav_model->getCustomTest(idx)->getCurrentState()
                                == true ? 1 : 0;
            }
        }

    }

    return currentState;
}

/**
 * Set the path for storing bookmarks
 *
 * @param path The target path to store bookmarks in
 * @return Returns true on success
 */
bool DaisyHandler::setBookmarkPath(std::string path)
{
    mBmkPath = path;
    return true;
}

/**
 * Set up bookmarks, either loads an existing bookmark or creates a new one
 *
 * @param uid A unique identifier for the book
 * @param checksum A MD5 checksum for the book
 */
void DaisyHandler::setupBookmarks(std::string uid, std::string checksum)
{
    string filesafe_uid = uid + "_" + checksum;

    //look for a file in the bookmark directory which looks like "uid.bmk"
    //if not found, create a new one
    //record the book's title

    string bookmark_file;
    if (mBmkPath != "")
    {
        bookmark_file = mBmkPath;
    }
    else if (getenv("BOOKMARK_DIR") != NULL)
    {
        bookmark_file = getenv("BOOKMARK_DIR");
    }
    else if (getenv("KOLIBRE_DATA_PATH") != NULL)
    {
        bookmark_file = getenv("KOLIBRE_DATA_PATH");
    }
    else
    {
        bookmark_file = DEFAULT_BOOKMARK_PATH;
    }

    bookmark_file = amis::FilePathTools::convertSlashesFwd(bookmark_file);


    if (bookmark_file[bookmark_file.size() - 1] != '/')
    {
        bookmark_file.append("/");
    }

    //replace all / \ , ; : (special chars) with '__' (double underscore)

    for (unsigned int i = 0; i < filesafe_uid.length(); i++)
    {
        if (filesafe_uid[i] == '\\')
        {
            filesafe_uid.replace(i, 1, "__");
        }

        else if (filesafe_uid[i] == '/')
        {
            filesafe_uid.replace(i, 1, "__");
        }

        else if (filesafe_uid[i] == ',')
        {
            filesafe_uid.replace(i, 1, "__");
        }
        else if (filesafe_uid[i] == ';')
        {
            filesafe_uid.replace(i, 1, "__");
        }

        else if (filesafe_uid[i] == ':')
        {
            filesafe_uid.replace(i, 1, "__");
        }

        else if (filesafe_uid[i] == ' ')
        {
            filesafe_uid.replace(i, 1, "__");
        }

        else
        {
            //empty
        }
    }

    bookmark_file.append(filesafe_uid);
    bookmark_file.append(".bmk");

    if (amis::FilePathTools::fileIsReadable(bookmark_file))
    {
        //create a new bookmark file object
        amis::BookmarkFile* p_bmk = NULL;
        p_bmk = new amis::BookmarkFile();

        amis::BookmarksReader reader;
        AmisError result = reader.openFile(bookmark_file, p_bmk);

        if (result.getCode() == amis::OK)
        {
            // Set the current bookmark to the last one added
            mCurrentBookmark = p_bmk->getNumberOfItems() - 1;
            p_bmk->setUid(uid);

            // Set mBmkFilePath and mpBmk pointer and return
            mBmkFilePath = bookmark_file;
            mpBmk = p_bmk;

            return;
        }

        if (result.getCode() != amis::NOT_FOUND)
        {
            std::string bookmark_file_backup = bookmark_file;
            bookmark_file_backup.append(".corrupt");

            if (not amis::FilePathTools::renameFile(bookmark_file, bookmark_file_backup))
            {
                LOG4CXX_WARN(amisDaisyHandlerLog, "Error creating bookmark file backup");
            }
        }
    }

    LOG4CXX_INFO(amisDaisyHandlerLog, "Creating a new bookmark file");

    //create a new bookmark file object
    amis::BookmarkFile* p_bmk = NULL;
    p_bmk = new amis::BookmarkFile();

    //read the title data from our current book and add it to the file
    amis::TitleAuthorParse title_parse;
    amis::MediaGroup* p_title = NULL;

    AmisError err = title_parse.openFile(this->mFilePath);
    if (err.getCode() == amis::OK)
    {
        p_title = title_parse.getTitleInfo();
    }
    else
    {
        p_title = new amis::MediaGroup();

        amis::TextNode* p_text = NULL;
        p_text = new amis::TextNode();

        p_text->setTextString("no title");
    }

    p_bmk->setTitle(p_title);
    p_bmk->setUid(uid);

    amis::BookmarksWriter writer;
    if (not writer.saveFile(bookmark_file, p_bmk))
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "Failed to save bookmark file");
        delete p_bmk;
        return;
    }

    // Set mBmkFilePath and mpBmk pointer and return
    mBmkFilePath = bookmark_file;
    mpBmk = p_bmk;
}

/**
 * Add a bookmark from current position
 *
 * @return Returns true on success
 */
bool DaisyHandler::addBookmark()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("BookmarkFile not open");
        reportGeneralError(err);
        return false;
    }

    amis::Bookmark* p_bmk = NULL;
    amis::PositionData* p_pos = NULL;
    amis::MediaGroup* p_note = NULL;
    amis::TextNode* p_note_text = NULL;

    p_bmk = new amis::Bookmark();
    p_pos = new amis::PositionData;
    p_note = new amis::MediaGroup();
    p_note_text = new amis::TextNode();

    p_pos->mUri = mLastmarkUri;
    p_bmk->mId = mpBmk->getMaxId() + 1;

    // get the audioref
    if (mpCurrentMedia->getNumberOfAudioClips() > 0)
    {
        AudioNode* p_audio = NULL;
        for (unsigned int i = 0; i < mpCurrentMedia->getNumberOfAudioClips();
                i++)
        {
            p_audio = mpCurrentMedia->getAudio(i);
            p_pos->mAudioRef = p_audio->getId();
        }
    }

    // get the textref
    if (mpCurrentMedia->hasText())
    {
        p_pos->mTextRef = mpCurrentMedia->getText()->getId();
    }

    NavNode* p_node = NULL;
    string bookmarktext = "";
    p_node = NavParse::Instance()->getNavModel()->getNavMap()->current();
    if (p_node != NULL)
    {
        p_pos->mNcxRef = p_node->getId();
        p_pos->mPlayOrder = p_node->getPlayOrder();

        // Try to get the text of the current navmapitem
        MediaGroup *mpLabel = NULL;
        mpLabel = p_node->getLabel();
        if (mpLabel != NULL)
            if (mpLabel->getText() != NULL)
                bookmarktext = mpLabel->getText()->getTextString();
    }

    // If we couldn't find a bookmarktext, create a timestamp text for the bookmark
    if (bookmarktext == "")
    {
        time_t rawtime;
        struct tm *timeinfo;
        char buffer[80];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, 80, "%c", timeinfo);

        bookmarktext.assign(buffer);
    }

    // only use 20 first characters
    bookmarktext = bookmarktext.substr(0, 30);

    p_note_text->setTextString(bookmarktext);

    p_note->setText(p_note_text);
    amis::AudioNode* p_audio = NULL;
    amis::AudioNode* p_temp_audio = NULL;

    p_temp_audio = mpCurrentMedia->getAudio(0);

    if (p_temp_audio != NULL)
    {
        //copy the properties of this audio node
        //because it will disappear as soon as a new SMIL file is loaded
        p_audio = new amis::AudioNode();
        p_audio->setSrc(p_temp_audio->getSrc());
        p_audio->setId(p_temp_audio->getId());
        p_audio->setClipBegin(p_temp_audio->getClipBegin());
        p_audio->setClipEnd(p_temp_audio->getClipEnd());
        p_audio->setHref(p_temp_audio->getHref());
        p_audio->setClass(p_temp_audio->getClass());
        p_audio->setMediaNodeType(p_temp_audio->getMediaNodeType());
        p_audio->setMediaType(p_temp_audio->getMediaType());
        p_audio->setLangCode(p_temp_audio->getLangCode());

        p_note->addAudioClip(p_audio);
    }

    p_bmk->mpStart = p_pos;
    p_bmk->mpNote = p_note;
    p_bmk->mbHasNote = true;
    p_bmk->mType = amis::PositionMark::BOOKMARK;

    mpBmk->addBookmark(p_bmk);
    //mpBmk->print();

    // Set the current bookmark to the last one added
    mCurrentBookmark = mpBmk->getNumberOfItems() - 1;

    amis::BookmarksWriter writer;
    if (not writer.saveFile(mBmkFilePath, mpBmk))
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "Failed to save bookmark file");
        err.setCode(amis::UNDEFINED_ERROR);
        err.setMessage("Failed to save bookmark file");
        reportGeneralError(err);
        return false;
    }

    return true;
}

/**
 * Delete current bookmark
 *
 * @return Returns true on success
 */
bool DaisyHandler::deleteCurrentBookmark()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    int idx = mCurrentBookmark;

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return false;
    }

    if (idx >= 0 && idx < mpBmk->getNumberOfItems())
    {
        mpBmk->deleteItem(idx);
        amis::BookmarksWriter writer;
        if (not writer.saveFile(mBmkFilePath, mpBmk))
        {
            LOG4CXX_ERROR(amisDaisyHandlerLog, "Failed to save bookmark file");
            err.setCode(amis::UNDEFINED_ERROR);
            err.setMessage("Failed to save bookmark file");
            reportGeneralError(err);
            return false;
        }

        if (mCurrentBookmark >= mpBmk->getNumberOfItems())
            mCurrentBookmark = mpBmk->getNumberOfItems() - 1;

        return true;
    }
    else
    {
        err.setCode(NOT_FOUND);
        err.setMessage("Bookmark not found, idx " + idx);
        reportGeneralError(err);
    }

    return false;
}

/**
 * Deletes all bookmarks in media
 *
 * @return Return true on success
 * @return false Returns false if bookmarks are not supported
 */
bool DaisyHandler::deleteAllBookmarks()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return false;
    }

    while (mpBmk->getNumberOfItems())
    {
        mpBmk->deleteItem(0);
    }

    amis::BookmarksWriter writer;
    if (not writer.saveFile(mBmkFilePath, mpBmk))
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "Failed to save bookmark file");
        err.setCode(amis::UNDEFINED_ERROR);
        err.setMessage("Failed to save bookmark file");
        reportGeneralError(err);
        return false;
    }

    mCurrentBookmark = -1;

    return true;
}

bool DaisyHandler::selectBookmark(int idx)
{
    amis::PositionMark* p_pos = NULL;
    string content_url;

    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return 0;
    }

    if (idx >= 0 && idx < mpBmk->getNumberOfItems())
    {
        p_pos = mpBmk->getItem(idx);

        if (p_pos != NULL)
        {
            content_url = amis::FilePathTools::goRelativePath(mFilePath,
                    p_pos->mpStart->mUri);

            string audioRef = p_pos->mpStart->mAudioRef;
            bool ret = loadSmilContent(content_url, audioRef);

            if (ret == true)
            {
                // Locate the correct position in the navmap
                syncNavModel(p_pos->mpStart->mNcxRef,
                        p_pos->mpStart->mPlayOrder);
            }
            return ret;

        }
    }

    err.setCode(NOT_FOUND);
    err.setMessage("could not go to bookmark idx: " + idx);
    reportGeneralError(err);

    return false;
}

/**
 * Get the number of bookmarks stored
 *
 * @return Returns the number of bookmarks found
 * @return 0 Returns 0 if bookmarks are not supported or none are found
 */
unsigned int DaisyHandler::getNumberOfBookmarks()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return 0;
    }
    return mpBmk->getNumberOfItems();
}

/**
 * Select the next bookmark
 *
 * @return Returns true on success
 */
bool DaisyHandler::nextBookmark()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return false;
    }

    if (mpBmk->getNumberOfItems() == 0)
    {
        err.setCode(NOT_FOUND);
        err.setMessage("No bookmarks found");
        reportGeneralError(err);
        return false;
    }

    // Remember the current navi direction
    naviDirection = FORWARD;

    // Remember when we last changed bookmarks manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    if (mCurrentBookmark < mpBmk->getNumberOfItems() - 1)
    {
        mCurrentBookmark++;
        return selectBookmark(mCurrentBookmark);
    }
    else
    {
        err.setCode(NOT_FOUND);
        err.setMessage("Could not go to next bookmark");
        reportGeneralError(err);

        return selectBookmark(mCurrentBookmark);
    }

    return false;

}

/**
 * Select previous bookmark
 *
 * @return Returns true on success
 */
bool DaisyHandler::previousBookmark()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return false;
    }

    if (mpBmk->getNumberOfItems() == 0)
    {
        err.setCode(NOT_FOUND);
        err.setMessage("No bookmarks found");
        reportGeneralError(err);
        return false;
    }

    if (mCurrentBookmark == -1)
        mCurrentBookmark = 0;

    // Remember the current navi direction
    naviDirection = BACKWARD;

    // Remember when we last changed bookmarks manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    if (mCurrentBookmark > 0)
    {
        mCurrentBookmark--;
        return selectBookmark(mCurrentBookmark);
    }
    else
    {
        err.setCode(NOT_FOUND);
        err.setMessage("Could not go to previous bookmark");
        reportGeneralError(err);

        return selectBookmark(mCurrentBookmark);
    }

    return false;
}

/**
 * Get the id of current bookmark
 *
 * @return Returns the id of current bookmark
 * @return -1 Returns -1 on error or if no bookmark is loaded
 */
int DaisyHandler::getCurrentBookmarkId()
{
    amis::PositionMark* p_pos = NULL;
    amis::Bookmark* p_bmk = NULL;

    int id = -1;

    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return -1;
    }

    if (mCurrentBookmark >= 0 && mCurrentBookmark < mpBmk->getNumberOfItems())
    {
        p_pos = mpBmk->getItem(mCurrentBookmark);

        if (p_pos != NULL && p_pos->mType == PositionMark::BOOKMARK)
            p_bmk = (amis::Bookmark*) p_pos;

        if (p_bmk != NULL)
        {

            id = p_bmk->mId;
            return id;

        }
    }

    err.setCode(NOT_FOUND);
    err.setMessage("could not go to bookmark idx: " + mCurrentBookmark);
    reportGeneralError(err);

    return -1;
}

/**
 * Get the id of next bookmark
 *
 * @return Returns the id of next bookmark
 * @return -1 Returns -1 if bookmarks are not supported
 */
int DaisyHandler::getNextBookmarkId()
{
    AmisError err;
    err.setSourceModuleName(module_DaisyHandler);

    if (mpBmk == NULL)
    {
        err.setCode(NOT_SUPPORTED);
        err.setMessage("Bookmarkfile not open");
        reportGeneralError(err);
        return -1;
    }

    return mpBmk->getMaxId() + 1;
}

/**
 * Increase the level granularity for navigating the book
 *
 * @return Returns true on success
 */
bool DaisyHandler::increaseNaviLevel()
{
    // Update the last level change time
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int maxDepth = 0;
    bool hasPages = false;
    bool hasBookmarks = false;
    bool hasHistory = false;

    if (p_nav_model != NULL)
    {
        maxDepth = p_nav_model->getNavMap()->getMaxDepth();
        hasPages = p_nav_model->hasPages();
    }

    if (getNumberOfBookmarks())
    {
        hasBookmarks = true;
    }

    if (mpHst != NULL && mpHst->getNumberOfItems())
    {
        hasHistory = true;
    }

    switch (currentNaviLevel)
    {
    case PHRASE:
        if (hasPages)
        {
            currentNaviLevel = PAGE;
            break;
        }
    case PAGE:
        if (maxDepth >= 6)
        {
            currentNaviLevel = H6;
            break;
        }
    case H6:
        if (maxDepth >= 5)
        {
            currentNaviLevel = H5;
            break;
        }
    case H5:
        if (maxDepth >= 4)
        {
            currentNaviLevel = H4;
            break;
        }
    case H4:
        if (maxDepth >= 3)
        {
            currentNaviLevel = H3;
            break;
        }
    case H3:
        if (maxDepth >= 2)
        {
            currentNaviLevel = H2;
            break;
        }
    case H2:
        if (maxDepth >= 1)
        {
            currentNaviLevel = H1;
            break;
        }
    case H1:
        if (hasHistory && NAVIHISTORY_LEVEL_ENABLE)
        {
            currentNaviLevel = HISTORY;
            break;
        }

    case HISTORY:
        if (hasBookmarks)
        {
            currentNaviLevel = BOOKMARK;
            break;
        }

    case BOOKMARK:
        currentNaviLevel = BEGEND;
        break;

    case BEGEND:
        currentNaviLevel = TOPLEVEL;
        break;

    case TOPLEVEL:
        currentNaviLevel = PHRASE;
        break;
    }

    return true;
}

/**
 * Decrease the level granularity for navigating the book
 *
 * @return Returns true on success
 */
bool DaisyHandler::decreaseNaviLevel()
{
    // Update the last level change time
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;
    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    int maxDepth = 0;
    bool hasPages = false;
    bool hasBookmarks = false;
    bool hasHistory = false;

    if (p_nav_model != NULL)
    {
        maxDepth = p_nav_model->getNavMap()->getMaxDepth();
        hasPages = p_nav_model->hasPages();
    }

    if (getNumberOfBookmarks())
    {
        hasBookmarks = true;
    }

    if (mpHst != NULL && mpHst->getNumberOfItems())
    {
        hasHistory = true;
    }

    switch (currentNaviLevel)
    {
    case TOPLEVEL:
        currentNaviLevel = BEGEND;
        break;

    case BEGEND:
        if (hasBookmarks)
        {
            currentNaviLevel = BOOKMARK;
            break;
        }
    case BOOKMARK:
        if (hasHistory && NAVIHISTORY_LEVEL_ENABLE)
        {
            currentNaviLevel = HISTORY;
            break;
        }
    case HISTORY:
        if (maxDepth >= 1)
        {
            currentNaviLevel = H1;
            break;
        }
    case H1:
        if (maxDepth >= 2)
        {
            currentNaviLevel = H2;
            break;
        }
    case H2:
        if (maxDepth >= 3)
        {
            currentNaviLevel = H3;
            break;
        }
    case H3:
        if (maxDepth >= 4)
        {
            currentNaviLevel = H4;
            break;
        }
    case H4:
        if (maxDepth >= 5)
        {
            currentNaviLevel = H5;
            break;
        }
    case H5:
        if (maxDepth >= 6)
        {
            currentNaviLevel = H6;
            break;
        }
    case H6:
        if (hasPages)
        {
            currentNaviLevel = PAGE;
            break;
        }
    case PAGE:
        currentNaviLevel = PHRASE;
        break;

    case PHRASE:
        currentNaviLevel = TOPLEVEL;
        break;

    }

    return true;
}

/**
 * return the active NaviLevel
 *
 * @return Returns the current navi level
 */
DaisyHandler::NaviLevel DaisyHandler::getNaviLevel()
{
    return currentNaviLevel;
}

/**
 * Get the current navi level as string
 *
 * @return Returns the current navi level as string
 */
string DaisyHandler::getNaviLevelStr()
{
    switch (currentNaviLevel)
    {
    case TOPLEVEL:
        return "TOPLEVEL";
        break;
    case BEGEND:
        return "BEGEND";
        break;
    case BOOKMARK:
        return "BOOKMARK";
        break;
    case HISTORY:
        return "HISTORY";
        break;
    case H1:
        return "H1";
        break;
    case H2:
        return "H2";
        break;
    case H3:
        return "H3";
        break;
    case H4:
        return "H4";
        break;
    case H5:
        return "H5";
        break;
    case H6:
        return "H6";
        break;
    case PAGE:
        return "PAGE";
        break;
    case PHRASE:
        return "PHRASE";
        break;
    default:
        return "UNKNOWN";
        break;
    }
    return "ERROR";
}

/**
 * Print the current navi level to console
 * Used for debugging
 */
void DaisyHandler::printNaviLevel()
{
    LOG4CXX_INFO(amisDaisyHandlerLog,
            "currentNaviLevel: " << getNaviLevelStr());
}

/**
 * Print the current navi position to console
 * Used for debugging
 */
void DaisyHandler::printNaviPos()
{
    static string lastUri = "";
    static string lastNcxRef = "";
    static string lastTextRef = "";
    static string lastAudioRef = "";
    static int lastPlayOrder = 0;

    std::ostringstream o;
    if (currentPos->mUri != lastUri)
    {
        o << "URI:" + currentPos->mUri;
        lastUri = currentPos->mUri;
    }

    if (currentPos->mPlayOrder != lastPlayOrder)
    {
        if (o.str() != "")
            o << "  ";
        o << "PO:" << currentPos->mPlayOrder;
        lastPlayOrder = currentPos->mPlayOrder;
    }

    if (currentPos->mNcxRef != lastNcxRef)
    {
        if (o.str() != "")
            o << "  ";
        o << "NCX:" + currentPos->mNcxRef;
        lastNcxRef = currentPos->mNcxRef;
    }

    if (currentPos->mTextRef != lastTextRef)
    {
        if (o.str() != "")
            o << "  ";
        o << "TR:" + currentPos->mTextRef;
        lastTextRef = currentPos->mTextRef;
    }

    if (currentPos->mAudioRef != lastAudioRef)
    {
        if (o.str() != "")
            o << "  ";
        o << "AR:" + currentPos->mAudioRef;
        lastAudioRef = currentPos->mAudioRef;
    }

    if (o.str() != "")
        LOG4CXX_DEBUG(amisDaisyHandlerLog, o.str());
}

/**
 * Set the navigation level granularity
 *
 * @param newLevel The new level to be set
 * @return Returns true on success
 */
bool DaisyHandler::updateNaviLevel(NaviLevel newLevel)
{
    NaviLevel updatedLevel = currentNaviLevel;

    switch (currentNaviLevel)
    {
    case H1:
        switch (newLevel)
        {
        case H2:
            updatedLevel = H2;
            break;
        case H3:
            updatedLevel = H3;
            break;
        case H4:
            updatedLevel = H4;
            break;
        case H5:
            updatedLevel = H5;
            break;
        case H6:
            updatedLevel = H6;
            break;
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case H2:
        switch (newLevel)
        {
        case H3:
            updatedLevel = H3;
            break;
        case H4:
            updatedLevel = H4;
            break;
        case H5:
            updatedLevel = H5;
            break;
        case H6:
            updatedLevel = H6;
            break;
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case H3:
        switch (newLevel)
        {
        case H4:
            updatedLevel = H4;
            break;
        case H5:
            updatedLevel = H5;
            break;
        case H6:
            updatedLevel = H6;
            break;
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case H4:
        switch (newLevel)
        {
        case H5:
            updatedLevel = H5;
            break;
        case H6:
            updatedLevel = H6;
            break;
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case H5:
        switch (newLevel)
        {
        case H6:
            updatedLevel = H6;
            break;
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case H6:
        switch (newLevel)
        {
        case PAGE:
            updatedLevel = PAGE;
            break;
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    case PAGE:
        switch (newLevel)
        {
        case PHRASE:
            updatedLevel = PHRASE;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    if (updatedLevel != currentNaviLevel)
    {
        // Check if we are currently navigating manually
        if (autonaviStartTime < time(NULL))
        {
            currentNaviLevel = updatedLevel;
            LOG4CXX_INFO(amisDaisyHandlerLog, "AutoNavi updating naviLevel");
            printNaviLevel();
            return true;
        }
    }

    return false;
}

/**
 * Jump to first section optionally skip title
 *
 * @param skipTitle Option to jump over title also
 * @return Retuns true on success
 */
bool DaisyHandler::firstSection(bool skipTitle)
{
    NavPoint* p_node = NULL;
    NavModel* p_nav_model = NULL;

    AmisError err;

    // Remember the current navi direction
    naviDirection = BACKWARD;

    // Remember when we last changed sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    p_nav_model = NavParse::Instance()->getNavModel();
    if (p_nav_model == NULL)
        return false;

    // Get the first node
    p_node = (NavPoint*) p_nav_model->getNavMap()->first();

    if (p_node != NULL)
    {
        //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Got node");
        string content_url = p_node->getContent();
        return loadSmilContent(content_url);
    }

    // Preset the error code in case we fail to find next node
    err.setCode(amis::UNDEFINED_ERROR);
    err.setMessage("could not go to first section of book");

    // Store the error
    reportGeneralError(err);

    return false;

}

/**
 * Jump to the last section
 *
 * @return Returns true on success
 */
bool DaisyHandler::lastSection()
{
    NavPoint* p_node = NULL;
    NavModel* p_nav_model = NULL;

    AmisError err;

    // Remember the current navi direction
    naviDirection = FORWARD;

    // Remember when we last changed sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    p_nav_model = NavParse::Instance()->getNavModel();
    if (p_nav_model == NULL)
        return false;

    // Get the first node
    p_node = (NavPoint*) p_nav_model->getNavMap()->last();

    if (p_node != NULL)
    {
        //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Got node");
        string content_url = p_node->getContent();
        return loadSmilContent(content_url);
    }

    // Preset the error code in case we fail to find next node
    err.setCode(amis::UNDEFINED_ERROR);
    err.setMessage("could not go to last section of book");

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the next phrase
 *
 * @param rewindWhenEndOfBook Option to rewind at the end of book
 * @return Returns true on success
 */
bool DaisyHandler::nextPhrase(bool rewindWhenEndOfBook)
{
    SmilMediaGroup* pMedia = NULL;
    pMedia = new SmilMediaGroup();

    // Remember the current navi direction
    naviDirection = FORWARD;

    AmisError err;

    err = SmilEngine::Instance()->next(pMedia);

    if (err.getCode() != OK && err.getCode() != AT_END)
    {
        err.setMessage("Error going to nextPhrase: " + err.getMessage());
        reportGeneralError(err);
    }
    else if (err.getCode() == AT_END)
    {
        if (rewindWhenEndOfBook)
        {

            SmilMediaGroup* pMediaZ = NULL;
            pMediaZ = new SmilMediaGroup();

            AmisError errZ;
            errZ =
                    SmilEngine::Instance()->loadPosition(
                            NavParse::Instance()->getNavModel()->getNavMap()->first()->getContent(),
                            pMediaZ);

            if (errZ.getCode() == OK)
            {
                return playMediaGroup(pMediaZ);
            }
        }

        // Store the error
        reportGeneralError(err);

        delete pMedia;
        return false;
    }
    else
    {
        // If we have a weird node try the next one
        return playMediaGroup(pMedia);
    }

    delete pMedia;
    return false;
}

/**
 * Jump to previous phrase
 *
 * @return Returns true on success, and false on fail or if we reached beginning of book
 */
bool DaisyHandler::previousPhrase()
{
    SmilMediaGroup* pMedia = NULL;
    pMedia = new SmilMediaGroup();

    // Remember the current navi direction
    naviDirection = BACKWARD;

    AmisError err;
    string currentSmilFile = SmilEngine::Instance()->getSmilSourcePath();
    err = SmilEngine::Instance()->previous(pMedia);

    if (err.getCode() != OK && err.getCode() != AT_BEGINNING)
    {
        err.setMessage("Error going to prevPhase: " + err.getMessage());
        reportGeneralError(err);
    }
    else if (err.getCode() == AT_BEGINNING)
    {
        // Store the error
        reportGeneralError(err);
    }
    else
    {
        //In case we navigate backwards on phrase level and we change name on the
        //smil file then resync position info
        if (currentSmilFile != SmilEngine::Instance()->getSmilSourcePath())
        {
            syncPosInfo();
        }
        return playMediaGroup(pMedia);
    }

    delete pMedia;
    return false;
}

/**
 * Jump to the next section at current navi level or higher
 *
 * @return Returns true on success
 */
bool DaisyHandler::nextSection()
{

    NavPoint* p_node = NULL;
    NavModel* p_nav_model = NULL;

    int mMaxDepth = 0;
    int mCurrentDepth = 0;
    int mCurrentPlayOrder = 0;

    // Remember the current navi direction
    naviDirection = FORWARD;

    // Remember when we last changed sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    switch (currentNaviLevel)
    {
    case H1:
        mCurrentDepth = 1;
        break;
    case H2:
        mCurrentDepth = 2;
        break;
    case H3:
        mCurrentDepth = 3;
        break;
    case H4:
        mCurrentDepth = 4;
        break;
    case H5:
        mCurrentDepth = 5;
        break;
    case H6:
        mCurrentDepth = 6;
        break;
    default:
        LOG4CXX_ERROR(amisDaisyHandlerLog, "Could not get current depth");
        return false;
    }

    p_nav_model = NavParse::Instance()->getNavModel();
    if (p_nav_model == NULL)
        return false;

    // Get the maximum depth
    mMaxDepth = p_nav_model->getNavMap()->getMaxDepth();

    // Get the current playorder
    mCurrentPlayOrder = p_nav_model->getPlayOrder();

    // Adjust the depth
    if (mCurrentDepth > mMaxDepth)
        mCurrentDepth = mMaxDepth;

    //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Parsing navnodes, currentDepth = " << mCurrentDepth << ", maxdepth = " << mMaxDepth << ", playorder = " << mCurrentPlayOrder);

    // Get the first node
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "Getting first navmap node");
    p_node = (NavPoint*) p_nav_model->getNavMap()->first();

    // Go trough each navnode until we find one we want
    while (p_node != NULL)
    {
        //p_node->print(p_node->getLevel());

        if (p_node->getLevel() <= mCurrentDepth
                && p_node->getPlayOrder() > mCurrentPlayOrder)
        {
            string content_url = p_node->getContent();
            LOG4CXX_DEBUG(amisDaisyHandlerLog, "Got node " << content_url);

            return loadSmilContent(content_url);
        }

        LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to next node");
        p_node =
                (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->next();

        // Fixes bug where next-prev-next-prev does not return correct positions
        /*if(p_node != NULL) {
         string content_url = p_node->getContent();
         
         SmilMediaGroup* pMedia = NULL;
         pMedia = new SmilMediaGroup();

         AmisError err = SmilEngine::Instance()->loadPosition(content_url, pMedia);
         
         if (err.getCode() != OK)
         LOG4CXX_ERROR(amisDaisyHandlerLog, "Could not load audionode " << content_url);

         delete pMedia;
         }*/

    }

    AmisError err;
    err.setCode(amis::AT_END);
    err.setMessage("could not go to next section");

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the previous section at current navi level or higher
 *
 * @return Returns true on success
 */
bool DaisyHandler::previousSection()
{

    NavPoint* p_node = NULL;
    NavPoint* p_prev_node = NULL;
    NavPoint* p_current_node = NULL;
    NavModel* p_nav_model = NULL;

    // Remember the current navi direction
    naviDirection = BACKWARD;

    // Remember when we last changes sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    int mMaxDepth = 0;
    int mCurrentDepth = 0;
    int mCurrentPlayOrder = 0;

    switch (currentNaviLevel)
    {
    case H1:
        mCurrentDepth = 1;
        break;
    case H2:
        mCurrentDepth = 2;
        break;
    case H3:
        mCurrentDepth = 3;
        break;
    case H4:
        mCurrentDepth = 4;
        break;
    case H5:
        mCurrentDepth = 5;
        break;
    case H6:
        mCurrentDepth = 6;
        break;
    default:
        LOG4CXX_ERROR(amisDaisyHandlerLog, "Could not get current depth");
        return false;
    }

    p_nav_model = NavParse::Instance()->getNavModel();
    if (p_nav_model == NULL)
        return false;

    // Get the maximum depth
    mMaxDepth = p_nav_model->getNavMap()->getMaxDepth();

    // Get the current playorder
    mCurrentPlayOrder = p_nav_model->getPlayOrder();

    // Adjust the depth
    if (mCurrentDepth > mMaxDepth)
        mCurrentDepth = mMaxDepth;

    //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Parsing navnodes, currentDepth = " << mCurrentDepth << ", maxdepth = " << mMaxDepth << ", playorder = " << mCurrentPlayOrder);

    // Get the current node
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "Getting current navmap node");
    p_current_node = (NavPoint*) p_nav_model->getNavMap()->current();

    // Get the first node
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "Getting first navmap node");
    p_node = (NavPoint*) p_nav_model->getNavMap()->first();

    // Go trough each navnode until we find one larger than the current playorder
    while (p_node != NULL)
    {
        //p_node->print(p_node->getLevel());

        if (p_node->getPlayOrder() >= mCurrentPlayOrder)
        {
            // See if we have a previous node in store
            if (p_prev_node != NULL)
            {
                string content_url = p_prev_node->getContent();
                bool result = loadSmilContent(content_url);

                LOG4CXX_DEBUG(amisDaisyHandlerLog, "Got node " << content_url);

                if (p_prev_node != p_current_node)
                    return result;
                else
                    break;

            }
            else
            {
                LOG4CXX_WARN(amisDaisyHandlerLog, "p_prev_node was NULL");
            }
        }

        if (p_node->getLevel() <= mCurrentDepth)
        {
            p_prev_node = p_node;
        }

        LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to next node");
        p_node = (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->next();

    }

    // Return the last node found
    if (p_prev_node != NULL)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog,
                "could not go to prev section, returning last node");
        string content_url = p_prev_node->getContent();
        loadSmilContent(content_url);
    }

    AmisError err;
    err.setCode(amis::AT_BEGINNING);
    err.setMessage("could not go to previous section");
    err.setSourceModuleName(module_DaisyHandler);

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the next section after index
 *
 * @param idx The index of the section before wanted node
 * @return Returns true on success
 */
bool DaisyHandler::nextInNavList(int idx)
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    AmisError err;

    // Remember the current navi direction
    naviDirection = FORWARD;

    // Remember when we last changes sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    int num_lists = p_model->getNumberOfNavLists();

    // Preset the error code in case we fail to find next node
    err.setCode(amis::AT_END);
    err.setMessage("could not go to next in nav list " + idx);

    if (idx >= 0 && idx < num_lists)
    {
        NavList* p_list = NULL;
        p_list = p_model->getNavList(idx);

        NavTarget* p_navt = NULL;
        p_navt = (NavTarget*) p_list->next();

        if (p_navt != NULL)
        {
            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavList(idx, p_navt);

            string content_url = p_navt->getContent();
            p_model->updatePlayOrder(p_navt->getPlayOrder());
            return loadSmilContent(content_url);
        }

    }
    else
    {
        err.setCode(amis::UNDEFINED_ERROR);
        err.setMessage("navlist out of range " + idx);
    }
    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the previous section before index
 *
 * @param idx The index of the section after wanted node
 * @return Returns true on success
 */
bool DaisyHandler::prevInNavList(int idx)
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    AmisError err;

    // Remember the current navi direction
    naviDirection = BACKWARD;

    // Remember when we last changes sections manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    int num_lists = p_model->getNumberOfNavLists();

    // Preset the error code in case we fail to find next node
    err.setCode(amis::AT_BEGINNING);
    err.setMessage("could not go to prev in nav list " + idx);

    if (idx >= 0 && idx < num_lists)
    {
        NavList* p_list = NULL;
        p_list = p_model->getNavList(idx);

        NavTarget* p_navt = NULL;
        p_navt = (NavTarget*) p_list->previous();

        if (p_navt != NULL)
        {
            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavList(idx, p_navt);

            string content_url = p_navt->getContent();
            p_model->updatePlayOrder(p_navt->getPlayOrder());
            return loadSmilContent(content_url);
        }

    }
    else
    {
        err.setCode(amis::UNDEFINED_ERROR);
        err.setMessage("navlist out of range " + idx);
    }

    // Store the error
    reportGeneralError(err);

    return false;

}

/**
 * Jump to next page
 *
 * @return Returns true if the page was opened
 */
bool DaisyHandler::nextPage()
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    // Remember if we hit the last page on previous call
    static bool gotlastpageonlastcall = false;
    if (naviDirection != FORWARD)
        gotlastpageonlastcall = false;

    AmisError err;
    err.setSourceModuleName(amis::module_DaisyHandler);

    // Remember the current navi direction
    naviDirection = FORWARD;

    // Remember when we last changed page manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    // Preset the error code in case we fail to find node
    err.setCode(amis::AT_END);
    err.setMessage("could not go to next page");

    if (p_model != NULL && p_model->hasPages() == true)
    {
        PageList* p_list = NULL;
        p_list = p_model->getPageList();

        PageTarget *p_last_page = NULL;
        PageTarget* p_page = NULL;

        p_last_page = (PageTarget*) p_list->last();
        p_page = (PageTarget*) p_list->nextBasedOnPlayOrder(p_model->getPlayOrder());

        if (p_page != NULL)
        {
            string content_url = p_page->getContent();
            LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to next page " << p_page->getPlayOrder());
            p_model->updatePlayOrder(p_page->getPlayOrder());
            bool ret = loadSmilContent(content_url);

            if (ret == false)
                return ret;

            // check if this is the last page
            if (p_last_page != NULL && p_page == p_last_page)
            {
                if (gotlastpageonlastcall)
                {
                    err.setCode(amis::AT_END);
                    err.setMessage("no more pages");
                    reportGeneralError(err);
                    ret = false;
                }
                gotlastpageonlastcall = true;
            }
            else
                gotlastpageonlastcall = false;

            return ret;

        }
        else
        {
            err.setCode(amis::NOT_FOUND);
            err.setMessage("failed to go to next page");

        }
    }
    else
    {
        err.setCode(amis::NOT_SUPPORTED);
        err.setMessage("book does not have pages");
    }

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the previous page
 *
 * @return Returns true if the page was opened
 */
bool DaisyHandler::previousPage()
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    // Remember if we hit the first page on previous call
    static bool gotfirstpageonlastcall = false;
    if (naviDirection != BACKWARD)
        gotfirstpageonlastcall = false;

    AmisError err;
    err.setSourceModuleName(amis::module_DaisyHandler);

    // Remember the current navi direction
    naviDirection = BACKWARD;

    // Remember when we last changed page manually
    autonaviStartTime = time(NULL) + AUTONAVI_DELAY_SECONDS;

    // Preset the error code in case we fail to find node
    err.setCode(amis::AT_BEGINNING);
    err.setMessage("could not go to previous page");

    if (p_model != NULL && p_model->hasPages() == true)
    {
        PageList* p_list = NULL;
        p_list = p_model->getPageList();

        PageTarget* p_page = NULL;
        PageTarget* p_first_page = NULL;

        p_first_page = (PageTarget*) p_list->first();
        p_page = (PageTarget*) p_list->previousBasedOnPlayOrder(p_model->getPlayOrder());

        if (p_page != NULL)
        {
            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncPageList(p_page);

            string content_url = p_page->getContent();
            LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to previous page " << p_page->getPlayOrder());
            p_model->updatePlayOrder(p_page->getPlayOrder());
            bool ret = loadSmilContent(content_url);

            if (ret == false)
                return ret;

            // check if this is the last page
            if (p_first_page != NULL && p_page == p_first_page)
            {
                if (gotfirstpageonlastcall)
                {
                    err.setCode(amis::AT_BEGINNING);
                    err.setMessage("no more pages");
                    reportGeneralError(err);
                    ret = false;
                }
                gotfirstpageonlastcall = true;
            }
            else
                gotfirstpageonlastcall = false;

            return ret;
        }
        else
        {
            err.setCode(amis::NOT_FOUND);
            err.setMessage("failed to go to previous page");
        }
    }
    else
    {
        err.setCode(amis::NOT_SUPPORTED);
        err.setMessage("book does not have pages");
    }

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Go to a node with id
 *
 * @param id the id of the wanted node
 * @return Returns true if the node was opened
 */
bool DaisyHandler::goToId(std::string id)
{
    NavModel* p_model = NavParse::Instance()->getNavModel();
    if (p_model == NULL)
        return false;

    NavPoint* navPoint;
    AmisError err = p_model->goToId(id, navPoint);
    if (err.getCode() != amis::OK)
        return false;

    bool success = loadSmilContent(navPoint->getContent());
    if(!success)
        LOG4CXX_WARN(amisDaisyHandlerLog, "Failed to load " << navPoint->getContent());

    return success;
}

/**
 * Go to a page with the given label
 *
 * @param page_name The label of the page
 * @return Returns true if the page was opened
 */
bool DaisyHandler::goToPage(std::string page_name)
{
    AmisError err;

    // Preset the error code in case we fail to find node
    err.setCode(amis::NOT_FOUND);
    err.setMessage("page not found (" + page_name + ")");

    if (page_name.size() > 0)
    {
        NavModel* p_model = NULL;
        p_model = NavParse::Instance()->getNavModel();

        if (p_model != NULL && p_model->hasPages() == true)
        {
            PageList* p_list = NULL;
            p_list = p_model->getPageList();

            PageTarget* p_page = NULL;

            p_page = (PageTarget*) p_list->findPage(page_name);

            if (p_page != NULL)
            {
                //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncPageList(p_page);

                string content_url = p_page->getContent();
                p_model->updatePlayOrder(p_page->getPlayOrder());
                return loadSmilContent(content_url);

            }
        }
        else
        {
            err.setCode(amis::NOT_SUPPORTED);
            err.setMessage("book does not have pages");
        }
    }

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the first page
 *
 * @return Returns true if the page was opened
 */
bool DaisyHandler::firstPage()
{
    AmisError err;

    // Preset the error code in case we fail to find node
    err.setCode(amis::NOT_FOUND);
    err.setMessage("error when jumping to first page");

    NavModel* p_model = NavParse::Instance()->getNavModel();
    if (p_model != NULL && p_model->hasPages() == true)
    {
        PageList* pageList = p_model->getPageList();
        if (pageList != NULL)
        {
            PageTarget* pageTarget = (PageTarget*) pageList->first();
            if (pageTarget != NULL)
            {
                LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to first page " << pageTarget->getPlayOrder());
                p_model->updatePlayOrder(pageTarget->getPlayOrder());
                return loadSmilContent(pageTarget->getContent());
            }
        }
    }
    else
    {
        err.setCode(amis::NOT_SUPPORTED);
        err.setMessage("book does not have pages");
    }

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Jump to the last page
 *
 * @return Returns true if the page was opened
 */
bool DaisyHandler::lastPage()
{
    AmisError err;

    // Preset the error code in case we fail to find node
    err.setCode(amis::NOT_FOUND);
    err.setMessage("error when jumping to last page");

    NavModel* p_model = NavParse::Instance()->getNavModel();
    if (p_model != NULL && p_model->hasPages() == true)
    {
        PageList* pageList = p_model->getPageList();
        if (pageList != NULL)
        {
            PageTarget* pageTarget = (PageTarget*) pageList->last();
            if (pageTarget != NULL)
            {
                LOG4CXX_DEBUG(amisDaisyHandlerLog, "Going to last page " << pageTarget->getPlayOrder());
                p_model->updatePlayOrder(pageTarget->getPlayOrder());
                return loadSmilContent(pageTarget->getContent());
            }
        }
    }
    else
    {
        err.setCode(amis::NOT_SUPPORTED);
        err.setMessage("book does not have pages");
    }

    // Store the error
    reportGeneralError(err);

    return false;
}

/**
 * Get the index of current page
 *
 * @return Returns the index of current page
 */
int DaisyHandler::currentPage()
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();
    PageTarget* p_current_page = NULL;

    AmisError err;

    if (p_model != NULL && p_model->hasPages() == true)
    {
        int mCurrentPlayOrder = p_model->getPlayOrder();

        PageList* p_list = NULL;
        p_list = p_model->getPageList();

        PageTarget* p_page = NULL;

        p_page = (PageTarget*) p_list->first();
        p_current_page = (PageTarget*) p_list->previousBasedOnPlayOrder(p_model->getPlayOrder());

        while (p_page != NULL)
        {
            string type = "unknown";
            string text = "";
            switch (p_page->getType())
            {
            case PageTarget::PAGE_FRONT:
                type = "PAGE_FRONT";
                break;
            case PageTarget::PAGE_NORMAL:
                type = "PAGE_NORMAL";
                break;
            case PageTarget::PAGE_SPECIAL:
                type = "PAGE_SPECIAL";
                break;
            }

            MediaGroup *p_label = p_page->getLabel();
            if (p_label != NULL && p_label->hasText())
                if (p_label->getText()->getTextString().length())
                {
                    text = p_label->getText()->getTextString();
                }

            LOG4CXX_DEBUG(amisDaisyHandlerLog,
                    "text: '" << text << "' " << "type: " << type << " id: " << p_page->getId() << " playorder: " << p_page->getPlayOrder());

            // Remember the current page
            if (p_page->getPlayOrder() <= mCurrentPlayOrder)
                p_current_page = p_page;

            p_page = (PageTarget*) p_list->next();
        }
    }
    else
    {
        return -1;
    }

    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "Current page id" << p_current_page->getId());

    return 0;
}

/**
 * Get the current page
 *
 * @return name of the current page
 */
string DaisyHandler::getCurrentPage()
{
    return mCurrentPage;
}

/**
 * return pointer to a NavNode referenced a by page number
 */
amis::NavNode* getNavNode(int pageNumber)
{
    amis::NavModel* navModel = NavParse::Instance()->getNavModel();
    if(navModel != NULL && navModel->hasPages())
    {
        amis::PageList* pageList = navModel->getPageList();
        if(pageList != NULL)
        {
            amis::NavNode* navNode = pageList->first();
            int pageCount = 0;
            while(navNode != NULL)
            {
                if(pageCount == pageNumber)
                {
                    return navNode;
                }
                navNode = pageList->next();
                pageCount++;
            }
        }
    }
    return NULL;
}

/**
 * Get the id for a page referenced by a page number
 *
 * @return page id on success, empty string on failure
 */
std::string DaisyHandler::getPageId(int pageNumber)
{
    amis::NavNode* navNode = getNavNode(pageNumber);
    if(navNode != NULL)
    {
        return navNode->getId();
    }
    return "";
}

/**
 * Get the label for a page referenced by a page number
 *
 * @return page name on success, empty string on failure
 */
std::string DaisyHandler::getPageLabel(int pageNumber)
{
    amis::NavNode* navNode = getNavNode(pageNumber);
    if(navNode != NULL)
    {
        if(navNode->getLabel() != NULL && navNode->getLabel()->hasText())
        {
            return navNode->getLabel()->getText()->getTextString();
        }
    }
    return "";
}

/**
 * Get bookinfo structure for opened book
 *
 * @return Returns the BookInfo for the current book
 */
DaisyHandler::BookInfo * DaisyHandler::getBookInfo()
{
    return &mBookInfo;
}

/**
 * Read book info into memory
 */
void DaisyHandler::readBookInfo()
{
    string tmp;
    NavModel *p_nav_model = NavParse::Instance()->getNavModel();

    int maxDepth = 0;
    bool hasPages = false;
    int currentPlayOrder = 0;

    if (p_nav_model != NULL)
    {
        maxDepth = p_nav_model->getNavMap()->getMaxDepth();
        hasPages = p_nav_model->hasPages();
        currentPlayOrder = p_nav_model->getPlayOrder();
    }

    LOG4CXX_INFO(amisDaisyHandlerLog, "Getting book information");

    // daisy type
    mBookInfo.mDaisyType = -1;
    mBookInfo.hasDaisyType = false;
    tmp = Metadata::Instance()->getMetadata("dc:format");
    if (tmp.length() != 0)
    {
        LOG4CXX_INFO(amisDaisyHandlerLog, "Daisy format '" << tmp << "'");

        // Convert value to lowercase letters
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                (int (*)(int))tolower);

        mBookInfo.hasDaisyType = true;

        if (tmp == "daisy 2.02") // Official dc:format tag
            mBookInfo.mDaisyType = 202;

        else if (tmp == "daisy 2.0") // Official dc:format tag
            mBookInfo.mDaisyType = 200;

        else if (tmp == "daisy 2.00") //?
            mBookInfo.mDaisyType = 200;

        else if (tmp == "ansi/niso z39.86-2002") //?
            mBookInfo.mDaisyType = 2002;

        else if (tmp == "nimas 1.0") //?
            mBookInfo.mDaisyType = 2002;

        else if (tmp == "ansi/niso z39.86-2005") // Official dc:format tag
            mBookInfo.mDaisyType = 2005;
        else
        {
            mBookInfo.hasDaisyType = false;
            LOG4CXX_WARN(amisDaisyHandlerLog, "Failed to parse dc:format");
        }
    }

    tmp = Metadata::Instance()->getMetadata("ncc:multimediaType");
    if (tmp.length() == 0)
        tmp = Metadata::Instance()->getMetadata("dtb:multimediaType");

    mBookInfo.mContentType = -1;
    if (tmp.length() != 0)
    {
        LOG4CXX_INFO(amisDaisyHandlerLog,
                "Daisy multimediatype '" << tmp << "'");

        // Convert value to lowercase letters
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                (int (*)(int))tolower);

if(        tmp == "audioonly")
        {
            mBookInfo.mContentType = 1;
        }
        else if(tmp == "audioncc" || tmp == "audioncx")
        {
            mBookInfo.mContentType = 2;
        }
        else if(tmp == "audioparttext")
        {
            mBookInfo.mContentType = 3;
        }
        else if(tmp == "audiofulltext")
        {
            mBookInfo.mContentType = 4;
        }
        else if(tmp == "textpartaudio")
        {
            mBookInfo.mContentType = 5;
        }
        else if(tmp == "textncc" || tmp == "textncx")
        {
            mBookInfo.mContentType = 6;
        }
        else if(tmp == "textonly")
        {
            mBookInfo.mContentType = 7;
        }

    }

    LOG4CXX_INFO(amisDaisyHandlerLog, "1. Getting SET information");
    tmp = Metadata::Instance()->getMetadata("ncc:setinfo");
    mBookInfo.hasSetInfo = false;

    if (tmp.length() != 0)
    {
        // Convert value to lowercase letters
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                (int (*)(int))tolower);

if                (sscanf(tmp.c_str(), "%d of %d", &mBookInfo.mCurrentSet, &mBookInfo.mMaxSet) == 2)
                {
                    mBookInfo.hasSetInfo = true;
                    LOG4CXX_INFO(amisDaisyHandlerLog, "Daisy parsed setInfo '" << mBookInfo.mCurrentSet << " of " << mBookInfo.mMaxSet << "'");
                }
                else
                {
                    LOG4CXX_WARN(amisDaisyHandlerLog, "Failed to parse setInfo: " << tmp);
                }
            }

    LOG4CXX_INFO(amisDaisyHandlerLog, "2. Getting NCC information");
    mBookInfo.mLevels = maxDepth;
    NaviLevel level = getNaviLevel();
    switch (level)
    {
    case H1:
        mBookInfo.mCurrentLevel = 1;
        break;
    case H2:
        mBookInfo.mCurrentLevel = 2;
        break;
    case H3:
        mBookInfo.mCurrentLevel = 3;
        break;
    case H4:
        mBookInfo.mCurrentLevel = 4;
        break;
    case H5:
        mBookInfo.mCurrentLevel = 5;
        break;
    case H6:
        mBookInfo.mCurrentLevel = 6;
        break;
    default:
        mBookInfo.mCurrentLevel = -1;
        break;
    }

    mBookInfo.mTocItems = -1;
    tmp = Metadata::Instance()->getMetadata("ncc:tocitems");
    mBookInfo.mTocItems = convertToInt(tmp);

    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "Level " << mBookInfo.mCurrentLevel << "/" << mBookInfo.mLevels << " TocItems: " << mBookInfo.mTocItems);

    mBookInfo.hasPages = false;
    LOG4CXX_INFO(amisDaisyHandlerLog, "3. Getting PAGE information");
    if (hasPages)
    {
        tmp = Metadata::Instance()->getMetadata("ncc:pageFront");
        if (tmp.length() == 0)
            tmp = Metadata::Instance()->getMetadata("ncc:page-front");
        mBookInfo.mFrontPages = convertToInt(tmp);

        tmp = Metadata::Instance()->getMetadata("ncc:pageNormal");
        if (tmp.length() == 0)
            tmp = Metadata::Instance()->getMetadata("ncc:page-normal");
        mBookInfo.mNormalPages = convertToInt(tmp);

        tmp = Metadata::Instance()->getMetadata("ncc:pageSpecial");
        if (tmp.length() == 0)
            tmp = Metadata::Instance()->getMetadata("ncc:page-special");
        mBookInfo.mSpecialPages = convertToInt(tmp);

        // Count the actual pages in navmodel
        PageTarget *p_current_page = NULL;
        AmisError err;
        int frontPages = 0, normalPages = 0, specialPages = 0;
        long minPage = 99999, maxPage = 0;

        if (p_nav_model != NULL)
        {
            PageList* p_list = NULL;
            p_list = p_nav_model->getPageList();

            PageTarget* p_page = NULL;

            p_page = (PageTarget*) p_list->first();
            p_current_page = (PageTarget*) p_list->previousBasedOnPlayOrder(p_nav_model->getPlayOrder());

            while (p_page != NULL)
            {
                string pageText = "";
                MediaGroup *p_label = p_page->getLabel();
                if (p_label != NULL && p_label->hasText())
                    if (p_label->getText()->getTextString().length())
                    {
                        pageText = p_label->getText()->getTextString();
                    }

                switch (p_page->getType())
                {
                case PageTarget::PAGE_FRONT:
                    frontPages++;
                    break;
                case PageTarget::PAGE_NORMAL:
                    normalPages++;
                    // get min and max pagenumbers in PAGE_NORMAL class
                    if (convertToInt(pageText) < minPage)
                        minPage = convertToInt(pageText);
                    if (convertToInt(pageText) > maxPage)
                        maxPage = convertToInt(pageText);

                    break;
                case PageTarget::PAGE_SPECIAL:
                    specialPages++;
                    break;
                default:
                    LOG4CXX_ERROR(amisDaisyHandlerLog,
                            "Unknown pagetype detected..?");
                    break;
                }

                p_page = (PageTarget*) p_list->next();
            }

            mBookInfo.hasPages = true;

        }

        if (frontPages != mBookInfo.mFrontPages)
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "ncc:pageFront (" << mBookInfo.mFrontPages << ") differs from parsed frontPages (" << frontPages << ")");
            mBookInfo.mFrontPages = frontPages;
        }

        LOG4CXX_INFO(amisDaisyHandlerLog,
                "ncc:pageFront: " << mBookInfo.mFrontPages);

        if (normalPages != mBookInfo.mNormalPages)
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "ncc:pageNormal (" << mBookInfo.mNormalPages << ") differs from parsed normalPages (" << normalPages << ")");
            mBookInfo.mNormalPages = normalPages;
        }

        if (maxPage > minPage && (maxPage != 99999 || minPage != 0))
        {
            mBookInfo.mMinPageNum = minPage;
            mBookInfo.mMaxPageNum = maxPage;
        }

        LOG4CXX_INFO(amisDaisyHandlerLog,
                "ncc:pageNormal: " << mBookInfo.mNormalPages << " numbering " << minPage << " -> " << maxPage);

        if (specialPages != mBookInfo.mSpecialPages)
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "ncc:pageSpecial (" << mBookInfo.mSpecialPages << ") differs from parsed specialPages (" << specialPages << ")");
            mBookInfo.mSpecialPages = specialPages;
        }

        LOG4CXX_INFO(amisDaisyHandlerLog,
                "ncc:pageSpecial: " << mBookInfo.mSpecialPages);

    }

    mBookInfo.hasTime = false;
    mPosInfo.hasCurrentTime = false;
    LOG4CXX_INFO(amisDaisyHandlerLog, "4. Getting TIME information");

    long nodestartms = 0;

    // check how far into the smil file we are (this will not work 100% of the time, 
    // for example if smil file has multiple mp3's but it will do for now
    if (mpCurrentMedia != NULL)
    {
        if (mpCurrentMedia->getNumberOfAudioClips() > 0)
        {
            for (unsigned int i = 0;
                    i < mpCurrentMedia->getNumberOfAudioClips(); i++)
            {
                string clipBegin = stringReplaceAll(
                        mpCurrentMedia->getAudio(i)->getClipBegin(), "npt=",
                        "");

                nodestartms = parseTime(clipBegin);

                //AudioNode *p_audio = mpCurrentMedia->getAudio(i);
                //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Got node " << p_audio->getId() << " clipBegin:" << startms);

            }
        }
    }

    tmp = Metadata::Instance()->getMetadata("ncc:totaltime");
    if (tmp.length() == 0)
        tmp = Metadata::Instance()->getMetadata("dtb:totaltime");
    LOG4CXX_DEBUG(amisDaisyHandlerLog, "totaltime = " << tmp);
    long totalms = parseTime(tmp);
    if (totalms != -1)
    {
        mBookInfo.hasTime = true;
        int seconds = totalms / 1000;
        mBookInfo.mTotalTime.tm_hour = seconds / 3600;
        mBookInfo.mTotalTime.tm_min = (seconds % 3600) / 60;
        mBookInfo.mTotalTime.tm_sec = (seconds % 60);
    }

    long elapsedms = 0;
    tmp = SmilEngine::Instance()->getMetadata("ncc:totalelapsedtime");
    if (tmp.length() == 0)
        tmp = SmilEngine::Instance()->getMetadata("dtb:totalelapsedtime");
    if (tmp.length() == 0)
        tmp = SmilEngine::Instance()->getMetadata("ncc:total-elapsed-time");
    elapsedms = parseTime(tmp);

    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "totalelapsedtime = " << tmp << " nodestartms = " << nodestartms/1000 << "s");

    if (elapsedms != -1)
    {
        mPosInfo.hasCurrentTime = true;
        int seconds = (elapsedms + nodestartms) / 1000;
        mPosInfo.mCurrentTime.tm_hour = seconds / 3600;
        mPosInfo.mCurrentTime.tm_min = (seconds % 3600) / 60;
        mPosInfo.mCurrentTime.tm_sec = (seconds % 60);
        mPosInfo.mPercentRead = int(
                (double) (elapsedms + nodestartms) / (double) (totalms)
                        * 100.0);
    }

    LOG4CXX_DEBUG(amisDaisyHandlerLog,
            "Total time: " << tmToTimeString(mBookInfo.mTotalTime) << " Elapsed time: " << tmToTimeString(mPosInfo.mCurrentTime) << " Percent read: " << mPosInfo.mPercentRead);

    LOG4CXX_INFO(amisDaisyHandlerLog, "5. Getting DATES information");

    mBookInfo.hasProdDate = false;
    mBookInfo.hasSourceDate = false;
    mBookInfo.hasRevisionDate = false;

    tmp = Metadata::Instance()->getMetadata("ncc:produceddate");
    if (tmp.length() == 0)
        tmp = Metadata::Instance()->getMetadata("dc:date");
    if (tmp.length() != 0)
    {
        int y, m, d;
        y = m = d = -1;
        // Daisy standard recommends "yyyy-mm-dd" as format, start with that
        if (sscanf(tmp.c_str(), "%4d-%2d-%2d", &y, &m, &d) == 3)
        {
            mBookInfo.hasProdDate = true;
            mBookInfo.mProdDate.tm_year = y - 1900;
            mBookInfo.mProdDate.tm_mon = m - 1;
            mBookInfo.mProdDate.tm_mday = d;
        }
        else if (sscanf(tmp.c_str(), "%4d", &y) == 1)
        {
            mBookInfo.hasProdDate = true;
            mBookInfo.mProdDate.tm_year = y - 1900;
            mBookInfo.mProdDate.tm_mon = -1;
            mBookInfo.mProdDate.tm_mday = -1;
        }
        else
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "Failed to parse ncc:produceddate or dc:date " << tmp);
        }
    }

    if (mBookInfo.hasProdDate)
    {
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "Production date: " << tmToDateString(mBookInfo.mProdDate));
    }

    tmp = Metadata::Instance()->getMetadata("ncc:sourcedate");
    if (tmp.length() != 0)
    {
        int y, m, d;
        y = m = d = -1;
        // Daisy standard recommends "yyyy-mm-dd" as format, start with that
        if (sscanf(tmp.c_str(), "%4d-%2d-%2d", &y, &m, &d) == 3)
        {
            mBookInfo.hasSourceDate = true;
            mBookInfo.mSourceDate.tm_year = y - 1900;
            mBookInfo.mSourceDate.tm_mon = m - 1;
            mBookInfo.mSourceDate.tm_mday = d;
        }
        else if (sscanf(tmp.c_str(), "%4d", &y) == 1)
        {
            mBookInfo.hasSourceDate = true;
            mBookInfo.mSourceDate.tm_year = y - 1900;
            mBookInfo.mSourceDate.tm_mon = -1;
            mBookInfo.mSourceDate.tm_mday = -1;
        }
        else
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "Failed to parse ncc:sourcedate " << tmp);
        }
    }

    if (mBookInfo.hasSourceDate)
    {
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "Source date: " << tmToDateString(mBookInfo.mSourceDate));
    }

    tmp = Metadata::Instance()->getMetadata("ncc:revision");
    mBookInfo.mRevisionNumber = -1;
    if (tmp.length() != 0)
    {
        int i = -1;
        if (sscanf(tmp.c_str(), "%d", &i) == 1)
        {
            mBookInfo.mRevisionNumber = i;
        }
        else
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "Failed to parse ncc:revision " << tmp);
        }
    }

    tmp = Metadata::Instance()->getMetadata("ncc:revisiondate");
    if (tmp.length() != 0)
    {
        int y, m, d;
        y = m = d = -1;
        // Daisy standard recommends "yyyy-mm-dd" as format, start with that
        if (sscanf(tmp.c_str(), "%4d-%2d-%2d", &y, &m, &d) == 3)
        {
            mBookInfo.hasRevisionDate = true;
            mBookInfo.mRevisionDate.tm_year = y - 1900;
            mBookInfo.mRevisionDate.tm_mon = m - 1;
            mBookInfo.mRevisionDate.tm_mday = d;
        }
        else if (sscanf(tmp.c_str(), "%4d", &y) == 1)
        {
            mBookInfo.hasRevisionDate = true;
            mBookInfo.mRevisionDate.tm_year = y - 1900;
            mBookInfo.mRevisionDate.tm_mon = -1;
            mBookInfo.mRevisionDate.tm_mday = -1;
        }
        else
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "Failed to parse dc:revisiondate " << tmp);
        }
    }

    if (mBookInfo.hasRevisionDate)
    {
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "Revision date: " << tmToDateString(mBookInfo.mRevisionDate));
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "Revision number: " << mBookInfo.mRevisionNumber);
    }

    // Number of sections
    mBookInfo.mSections = -1;
    if (p_nav_model != NULL and p_nav_model->getNavMap() != NULL)
    {
        mBookInfo.mSections = countSections(
                p_nav_model->getNavMap()->getRoot());
    }
}

void DaisyHandler::readPageNavPoints()
{
    mNavPoints.pages.clear();

    LOG4CXX_INFO(amisDaisyHandlerLog, "collecting page navigation points")
    amis::NavModel* navModel = NavParse::Instance()->getNavModel();
    if(navModel == NULL)
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "aborting process since NavModel is NULL");
        return;
    }

    if(!navModel->hasPages())
    {
        LOG4CXX_INFO(amisDaisyHandlerLog, "NavModel has no pages");
        return;
    }

    amis::PageList* pageList = navModel->getPageList();
    if(pageList != NULL)
    {
        amis::NavNode* navNode = pageList->first();
        while(navNode != NULL)
        {
            std::string id = navNode->getId();
            std::string text = "";
            if(navNode->getLabel() != NULL && navNode->getLabel()->hasText())
            {
                text = navNode->getLabel()->getText()->getTextString();
            }
            NavPoints::Page page(id, text, navNode->getPlayOrder(), navNode->getClass());
            mNavPoints.pages.push_back(page);
            navNode = pageList->next();
        }
    }
}

void recursiveReadSectionNavPoints(amis::NavPoint* navPoint, std::vector<DaisyHandler::NavPoints::Section>& sections)
{
    if(navPoint == NULL) return;

    std::string id = navPoint->getId();
    if(id != "ROOT")
    {
        std::string text = "";
        if(navPoint->getLabel() != NULL && navPoint->getLabel()->hasText())
        {
            text = navPoint->getLabel()->getText()->getTextString();
        }

        DaisyHandler::NavPoints::Section section(id, text, navPoint->getPlayOrder(), navPoint->getClass(), navPoint->getLevel());
        sections.push_back(section);
    }

    // recursively loop through children
    for(int i=0; i<navPoint->getNumChildren(); ++i)
    {
        amis::NavPoint* newNavPoint = navPoint->getChild(i);
        recursiveReadSectionNavPoints(newNavPoint, sections);
    }
}

void DaisyHandler::readSectionNavPoints()
{
    mNavPoints.sections.clear();

    LOG4CXX_INFO(amisDaisyHandlerLog, "collecting section navigation points")
    amis::NavModel* navModel = NavParse::Instance()->getNavModel();
    if(navModel == NULL)
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "aborting process since NavModel is NULL");
        return;
    }

    amis::NavMap* navMap = navModel->getNavMap();
    if(navMap == NULL)
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "aborting process since NavMap is NULL");
        return;
    }

    amis::NavPoint* navPoint = navMap->getRoot();
    recursiveReadSectionNavPoints(navPoint, mNavPoints.sections);
}

/**
 * Get NavPoints structure for opened book
 *
 * @return navigation points for the book
 */
DaisyHandler::NavPoints* DaisyHandler::getNavPoints()
{
    return &mNavPoints;
}

/**
 * Check if SmilMediaGroup has the specific audio ref
 *
 * @param pMedia The media to search in
 * @param audioRef The wanted audio ref
 * @return Returns true if audio ref is found
 */
bool DaisyHandler::SmilMediaGroup_has_AudioRef(SmilMediaGroup *pMedia,
        std::string audioRef)
{
    if (pMedia == NULL)
        return false;

    if (pMedia->getNumberOfAudioClips() > 0)
    {
        //LOG4CXX_DEBUG(amisDaisyHandlerLog, "pMedia has audio");

        AudioNode* p_audio = NULL;

        for (unsigned int i = 0; i < pMedia->getNumberOfAudioClips(); i++)
        {
            p_audio = pMedia->getAudio(i);
            if (p_audio != NULL)
            {
                string cur_audioRef = p_audio->getId();

                //LOG4CXX_DEBUG(amisDaisyHandlerLog, "comparing '" << audioRef << "' to '" << cur_audioRef << "'");

                if (cur_audioRef == audioRef)
                    return true;
            }
        }
    }
    return false;
}

/**
 * Load smil content for media
 *
 * @param contentUrl The url where smil content is located
 * @param audioRef The audio ref to load
 * @param offsetSecond The offset to jump to
 * @return Returns true on success
 */
bool DaisyHandler::loadSmilContent(std::string contentUrl, std::string audioRef,
        unsigned int offsetSecond)
{
    AmisError err;
    SmilMediaGroup* pMedia = NULL;
    pMedia = new SmilMediaGroup();

    LOG4CXX_INFO(amisDaisyHandlerLog, "loading " << contentUrl);
    err = SmilEngine::Instance()->loadPosition(contentUrl, pMedia);

    if (err.getCode() == OK)
    {
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "SmilEngine->loadPosition returned amis::OK");

        // If we have a specific audioRef to look for, search for it
        if (audioRef != "")
        {

            LOG4CXX_DEBUG(amisDaisyHandlerLog,
                    "Searching for audioref " +audioRef);

            string smil_file = SmilEngine::Instance()->getSmilSourcePath();
            string cur_smilfile = smil_file;
            bool bFoundit = false;

            // check if we have the correct audioRef
            if (SmilMediaGroup_has_AudioRef(pMedia, audioRef))
            {
                //LOG4CXX_WARN(amisDaisyHandlerLog, "Already at correct audioRef: " << audioRef);
                bFoundit = true;
                LOG4CXX_DEBUG(amisDaisyHandlerLog,
                        "SmilMediaGroup has audioref");

            }
            else
            {
                LOG4CXX_DEBUG(amisDaisyHandlerLog,
                        "Scanning current smilfile for correct audioRef: " << audioRef);
            }

            if (bFoundit == false)
            {

                LOG4CXX_DEBUG(amisDaisyHandlerLog,
                        "Scanning for correct audioref");

                delete pMedia;
                pMedia = new SmilMediaGroup();
                SmilEngine::Instance()->first(pMedia);

                while (smil_file == cur_smilfile)
                {

                    if (SmilMediaGroup_has_AudioRef(pMedia, audioRef))
                    {
                        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                                "Found correct audioRef in " << contentUrl);
                        bFoundit = true;
                        break;

                    }
                    else
                        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                                "Did not find correct audioref in " << contentUrl);

                    delete pMedia;
                    pMedia = NULL;
                    pMedia = new SmilMediaGroup();

                    LOG4CXX_DEBUG(amisDaisyHandlerLog,
                            "Going to SmilEngine->next()");

                    err = SmilEngine::Instance()->next(pMedia);
                    if (err.getCode() != OK)
                    {
                        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                                "SmilEngine->next() did not return amis:OK");
                        break;
                    }
                    else
                    {
                        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                                "SmilEngine->next() returned amis:OK");
                        // Check that we are still in the correct smilfile
                        cur_smilfile =
                                SmilEngine::Instance()->getSmilSourcePath();
                    }
                }
            }

            if (bFoundit == false)
            {
                LOG4CXX_WARN(amisDaisyHandlerLog,
                        "Could not locate correct audioRef, returning " << contentUrl);
                delete pMedia;
                pMedia = NULL;
                pMedia = new SmilMediaGroup();

                SmilEngine::Instance()->loadPosition(contentUrl, pMedia);
            }
        } //else LOG4CXX_WARN(amisDaisyHandlerLog, "No audioref supplied");

        LOG4CXX_DEBUG(amisDaisyHandlerLog, "Returning playmediagroup");
        return playMediaGroup(pMedia, offsetSecond);
    }
    else
    {
        //some error happened
        LOG4CXX_WARN(amisDaisyHandlerLog, "Error loading " << contentUrl);
        delete pMedia;
        reportGeneralError(err);
    }

    return false;
}

/**
 * Method for escaping characters in media
 *
 * @return Returns true on success
 */
bool DaisyHandler::escape()
{
    SmilMediaGroup* pMedia = NULL;
    pMedia = new SmilMediaGroup();

    AmisError err;

    err = SmilEngine::Instance()->escapeCurrent(pMedia);

    if (err.getCode() == OK)
    {
        return playMediaGroup(pMedia);
    }
    return false;
}

/**
 * Play the title of media
 *
 * @param pMedia Media root
 * @return Returns true if title media was found and played
 */
bool DaisyHandler::playTitle()
{
    if (mpTitle == NULL)
    {
        LOG4CXX_ERROR(amisDaisyHandlerLog, "mpTitle is NULL, maybe something is wrong with the book");
        return false;
    }

    string src;
    string clipBegin;
    string clipEnd;
    string audioref;

    if (mpTitle->getNumberOfAudioClips() > 0)
    {
        if (mpTitle->getNumberOfAudioClips() > 1)
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "**TITLEAUDIO: This MediaNode has more than 1 audio clip**");

        AudioNode* p_audio = NULL;
        for (unsigned int i = 0; i < mpTitle->getNumberOfAudioClips(); i++)
        {
            p_audio = mpTitle->getAudio(i);
            src = p_audio->getSrc();
            audioref = p_audio->getId();

            src = FilePathTools::getAsLocalFilePath(src);
            clipBegin = stringReplaceAll(mpTitle->getAudio(i)->getClipBegin(),
                    "npt=", "");
            clipEnd = stringReplaceAll(mpTitle->getAudio(i)->getClipEnd(),
                    "npt=", "");

            //double startms = convertToDouble(clipBegin) * 100;
            //double stopms = convertToDouble(clipEnd) * 100;
            long startms = parseTime(clipBegin) / 10;
            long stopms = parseTime(clipEnd) / 10;

            LOG4CXX_DEBUG(amisDaisyHandlerLog,
                    "playing " + audioref + src + " from " + clipBegin + " to " + clipEnd);

            callPlayFunction(src, (int) startms, (int) stopms);
        }

        return true;
    }

    return false;
}

/**
 * Play media group
 *
 * @param pMedia Media group with audio
 * @param offsetSecond Offset to play
 * @return Returns true on success
 */
bool DaisyHandler::playMediaGroup(SmilMediaGroup* pMedia,
        unsigned int offsetSecond)
{
    // Do not accept a mediagroup without audio
    if (pMedia != NULL)
        // Check that we have a media node with audio, if not scan for it
        if (pMedia->getNumberOfAudioClips() == 0)
        {
            AmisError err;
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "No audio, scanning for next node with audio");

            while (pMedia->getNumberOfAudioClips() == 0)
            {

                switch (naviDirection)
                {
                case FORWARD:
                    LOG4CXX_WARN(amisDaisyHandlerLog, "scanning forwards..");
                    err = SmilEngine::Instance()->next(pMedia);
                    if (err.getCode() != amis::OK)
                    {
                        err.setMessage(
                                "Error searching for mediagroup with audio: "
                                        + err.getMessage());
                        reportGeneralError(err);
                        return false;
                    }
                    break;

                case BACKWARD:
                    LOG4CXX_WARN(amisDaisyHandlerLog, "scanning backwards..");
                    err = SmilEngine::Instance()->previous(pMedia);
                    if (err.getCode() != amis::OK)
                    {
                        err.setMessage(
                                "Error searching for mediagroup with audio: "
                                        + err.getMessage());
                        reportGeneralError(err);
                        return false;
                    }
                    break;
                }
            }
        }

    if (mpCurrentMedia != NULL)
    {
        delete mpCurrentMedia;
        mpCurrentMedia = NULL;
    }

    if (pMedia != NULL)
    {
        mpCurrentMedia = pMedia;

        //if (pMedia->hasText() == true)
        //{
        //TextRenderBrain::Instance()->highlightUriTarget(pMedia->getText());

        //}
        //if there is no text, don't wait for it to render
        //else
        //{
        //TextRenderBrain::Instance()->loadBlankDocument();
        //}

        continuePlayingMediaGroup(offsetSecond);
        return true;
    }

    return false;
}

/**
 * Parse time from a time string
 *
 * @param str The string containing the time
 * @return Returns the time in milliseconds
 */
long parseTime(string str)
{
    int h, m, s, ms;
    bool ignore_max_seconds = false;

    if (sscanf(str.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms) != 4)
    {
        h = m = s = ms = 0;
        if (sscanf(str.c_str(), "%d:%d:%d", &h, &m, &s) != 3)
        {
            h = m = s = ms = 0;
            if (sscanf(str.c_str(), "%d.%ds", &s, &ms) != 2)
            {
                LOG4CXX_WARN(amisDaisyHandlerLog,
                        "parseTime: Failed to parse " << str);
                return -1;
            }
            else
                ignore_max_seconds = true;
        }
    }

    if (h >= 0 && h < 300 && // Check hours
            m >= 0 && m < 60 && // Check seconds
            (ignore_max_seconds || (s >= 0 && s < 60)) && // Ignore seconds if we have a XX.XXXs string or check seconds
            ms >= 0 && ms < 1000) // Check milliseconds
        return (h * 60 * 60 + m * 60 + s) * 1000 + ms;

    LOG4CXX_WARN(amisDaisyHandlerLog, "parseTime: Failed to parse " << str);
    return -1;
}

/**
 * Convert a string to double
 *
 * @param s String to convert
 * @return Returns the string converted to double
 * @return -1 Returns -1 if conversion failed
 */
inline double DaisyHandler::convertToDouble(const std::string& s)
{
    std::istringstream i(s);
    double x;

    try
    {
        i.exceptions(ios::badbit | ios::failbit);
        i >> x;
    } catch (ios_base::failure e)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog,
                "convertToDouble: Failed to parse " << s << " (" << e.what() << ")");
        return -1;
    }

    return x;
}

/**
 * Convert a string to integer
 *
 * @param s String to convert
 * @return Returns the string converted to integer
 * @return -1 Returns -1 if conversion failed
 */
inline int DaisyHandler::convertToInt(const std::string& s)
{
    LOG4CXX_TRACE(amisDaisyHandlerLog,
            "convertToInt: converting '" << s << "' to integer");
    std::istringstream i(s);
    int x;
    try
    {
        i.exceptions(ios::badbit | ios::failbit);
        i >> x;
    } catch (ios_base::failure& e)
    {
        LOG4CXX_WARN(amisDaisyHandlerLog,
                "convertToInt: Failed to parse " << s << " (" << e.what() << ")");
        return -1;
    }

    return x;
}

/**
 * Continue with current media group at offset second
 *
 * @param offsetSecond The wanted offset
 */
void DaisyHandler::continuePlayingMediaGroup(unsigned int offsetSecond)
{
    //USES_CONVERSION;
    if (mpCurrentMedia->hasImage() == true)
    {
        //		AfxMessageBox(_T("has an image"));

    }

    string src;
    string clipBegin;
    string clipEnd;
    string audioref;

    if (mpCurrentMedia->getNumberOfAudioClips() > 0)
    {

        //@bug
        //this won't work, we have to queue these clips for the rare instance
        //that there is more than one in a group
        //this could happen when we deal with resource files and labels, 
        //and want to couple some audio clips
        AudioNode* p_audio = NULL;

        if (mpCurrentMedia->getNumberOfAudioClips() > 1)
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "**AUDIO: This MediaNode has more than 1 audio clip**");

        for (unsigned int i = 0; i < mpCurrentMedia->getNumberOfAudioClips();
                i++)
        {
            p_audio = mpCurrentMedia->getAudio(i);
            src = p_audio->getSrc();
            audioref = p_audio->getId();

            src = FilePathTools::getAsLocalFilePath(src);
            clipBegin = stringReplaceAll(
                    mpCurrentMedia->getAudio(i)->getClipBegin(), "npt=", "");
            clipEnd = stringReplaceAll(
                    mpCurrentMedia->getAudio(i)->getClipEnd(), "npt=", "");

            long startms = parseTime(clipBegin) + (offsetSecond * 1000);
            long stopms = parseTime(clipEnd);
            //double startms = convertToDouble(clipBegin) * 100;
            //double stopms = convertToDouble(clipEnd) * 100;

            LOG4CXX_DEBUG(amisDaisyHandlerLog,
                    "**AUDIO: playing '" << audioref << "' " << src.c_str() << " from " << clipBegin.c_str() << "(" << startms << ") to " << clipEnd.c_str() << "(" << stopms << ") **");

            //LOG4CXX_DEBUG(amisDaisyHandlerLog, "Calling play function for " << src << " " << startms << "->" << stopms);
            if (!callPlayFunction(src, (int) startms / 10, (int) stopms / 10))
                callOOPlayFunction(src, startms, stopms);

        }
    }

    //printMediaGroup(mpCurrentMedia);

    //calculate our current URI
    //get only the smil file name
    string smil_file = SmilEngine::Instance()->getSmilSourcePath();
    string uri = FilePathTools::getFileName(smil_file);
    string textref = "";

    if (mpCurrentMedia != NULL)
    {
        uri.append("#");

        // Set up a text-uri in case we can't sync to the other one
        if (mpCurrentMedia->hasText())
        {
            textref = mpCurrentMedia->getText()->getId();
        }

        uri.append(mpCurrentMedia->getId());

    }

    // Synchronize navmodel to current position
    syncNavModel(uri, textref);

    //record our current position
    mLastmarkUri = uri;

    //record the lastmark in the bookmark file
    if (mpBmk != NULL)
    {
        LOG4CXX_TRACE(amisDaisyHandlerLog, "Recording lastmark");
        PositionData* p_pos = NULL;

        p_pos = new PositionData;

        p_pos->mUri = uri;
        p_pos->mTextRef = textref;
        p_pos->mAudioRef = audioref;

        NavNode* p_node = NULL;

        p_node = NavParse::Instance()->getNavModel()->getNavMap()->current();
        if (p_node != NULL)
        {
            p_pos->mNcxRef = p_node->getId();
            p_pos->mPlayOrder = p_node->getPlayOrder();

            mpBmk->setLastmark(p_pos);

            //LOG4CXX_WARN(amisDaisyHandlerLog,  "Printing lastmark");
            //mpBmk->printPositionData(p_pos);

            BookmarksWriter writer;
            if (not writer.saveFile(mBmkFilePath, mpBmk))
            {
                // only warn here
                LOG4CXX_WARN(amisDaisyHandlerLog, "Failed to save bookmark file");
            }
        }

    }
    else
        LOG4CXX_DEBUG(amisDaisyHandlerLog, "NOT recording lastmark");

    // Record an history item for this mediagroup
    if (mpHst != NULL)
    {
        PositionData *p_pos = NULL;
        p_pos = new PositionData;

        p_pos->mUri = uri;
        p_pos->mTextRef = textref;
        p_pos->mAudioRef = audioref;

        NavNode* p_node = NULL;

        p_node = NavParse::Instance()->getNavModel()->getNavMap()->current();
        if (p_node != NULL)
        {
            p_pos->mNcxRef = p_node->getId();
            p_pos->mPlayOrder = p_node->getPlayOrder();
        }

        mpHst->addItem(p_pos);
    }

    // Record current position for printouts
    currentPos->mUri = uri;
    currentPos->mTextRef = textref;
    currentPos->mAudioRef = audioref;

    NavNode* p_node = NULL;

    p_node = NavParse::Instance()->getNavModel()->getNavMap()->current();
    if (p_node != NULL)
    {
        currentPos->mNcxRef = p_node->getId();
        currentPos->mPlayOrder = p_node->getPlayOrder();
    }
    else
    {
        currentPos->mNcxRef = "";
        currentPos->mPlayOrder = 0;
    }

    printNaviPos();
}

/**
 * Send synchronisation message to registerd handlers
 *
 * @return Returns true on success
 */
bool DaisyHandler::syncPosInfo()
{
    string tmp = "";

    static int lastPlayOrder = -1;

    tmp = Metadata::Instance()->getMetadata("ncc:totaltime");
    if (tmp.length() == 0)
        tmp = Metadata::Instance()->getMetadata("dtb:totaltime");
    mPosInfo.totalSmilms = parseTime(tmp);

    tmp = SmilEngine::Instance()->getMetadata("ncc:totalelapsedtime");
    if (tmp.length() == 0)
        tmp = SmilEngine::Instance()->getMetadata("dtb:totalelapsedtime");
    if (tmp.length() == 0)
        tmp = SmilEngine::Instance()->getMetadata("ncc:total-elapsed-time");
    mPosInfo.currentSmilms = parseTime(tmp);

    NavModel* p_nav_model = NULL;
    p_nav_model = NavParse::Instance()->getNavModel();

    if (p_nav_model != NULL)
    {

        int currentPlayOrder = p_nav_model->getPlayOrder();

        // If playorder has changed since last sync..
        if (currentPlayOrder != lastPlayOrder)
        {

            // ..synchronize current section index using playorder
            if (p_nav_model->getNavMap() != NULL)
            {
                bool found = false;
                int cursec = currentSection(p_nav_model->getNavMap()->getRoot(),
                        currentPlayOrder, found);
                if (found)
                    mPosInfo.currentSectionIdx = cursec;
                else
                    mPosInfo.currentSectionIdx = -1;
            }

            // ..and synchronize current section index using playorder
            if (p_nav_model->hasPages())
            {
                mPosInfo.currentPageIdx = currentNavContainerIdx(
                        p_nav_model->getPageList(), currentPlayOrder);
            }
            else
            {
                mPosInfo.currentPageIdx = -1;
            }
        }
        lastPlayOrder = currentPlayOrder;
    }

    return true;
}

/**
 * Get a refference to the pos info structure
 *
 * @return Returns the PosInfo structure
 */
DaisyHandler::PosInfo *DaisyHandler::getPosInfo()
{
    return &mPosInfo;
}

/**
 * Sync the nav model
 *
 * @param uri Uri to sync with
 * @param textref The wanted textref
 *
 * @return Returns true on success
 */
bool DaisyHandler::syncNavModel(std::string uri, std::string textref)
{

    //LOG4CXX_WARN(amisDaisyHandlerLog, "Synchronizing navmodel to uri: '" << uri << "' textref: '" << textref << "'");
    //sync our position with the nav display and data model

    static MediaGroup *p_last_label = NULL;
    NavNode* p_sync_nav = NULL;

    // First try to sync to the ncxref
    if (uri != "")
        p_sync_nav = NavParse::Instance()->getNavModel()->goToHref(uri);

    // If we failed to sync to the regular uri, try the text reference
    if (p_sync_nav == NULL && textref != "")
    {
        string texturi = FilePathTools::getFileName(uri);
        texturi.append("#");
        texturi.append(textref);

        p_sync_nav = NavParse::Instance()->getNavModel()->goToHref(texturi);
        uri = texturi;
    }

    if (p_sync_nav != NULL)
    {
        // Get pointers to the navmodel and navmap
        NavModel *p_nav_model = NavParse::Instance()->getNavModel();
        NavMap *p_nav_map = NULL;
        if (p_nav_model != NULL)
            p_nav_map = p_nav_model->getNavMap();

        if (p_sync_nav->getTypeOfNode() == NavNode::NAV_POINT)
        {
            LOG4CXX_INFO(amisDaisyHandlerLog,
                    "Syncing navmap to NAV_POINT " << uri);

            //p_sync_nav->print(1);

            NavPoint* p_nav = (NavPoint*) p_sync_nav;
            if (p_nav_map != NULL)
                p_nav_map->updateCurrent(p_nav);
            if (p_nav_model != NULL)
                p_nav_model->syncLists(p_nav->getPlayOrder());

            p_nav_model->updatePlayOrder(p_nav->getPlayOrder());

            MediaGroup *p_label = p_nav->getLabel();
            if (p_label != NULL && p_label->hasText()
                    && p_label != p_last_label)
                if (p_label->getText()->getTextString().length())
                {
                    LOG4CXX_INFO(amisDaisyHandlerLog,
                            "NAV_POINT: '" << p_label->getText()->getTextString() << "'");
                    p_last_label = p_label;
                }

            switch (p_nav->getLevel())
            {
            case 1:
                updateNaviLevel(H1);
                break;
            case 2:
                updateNaviLevel(H2);
                break;
            case 3:
                updateNaviLevel(H3);
                break;
            case 4:
                updateNaviLevel(H4);
                break;
            case 5:
                updateNaviLevel(H5);
                break;
            case 6:
                updateNaviLevel(H6);
                break;
            }

            //TRACE(_T("continueplayingmediagroup ... asking syncNavMap\n"));
            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavMap((NavPoint*)p_sync_nav);
            syncPosInfo();
            return true;
        }
        else if (p_sync_nav->getTypeOfNode() == NavNode::PAGE_TARGET)
        {
            LOG4CXX_INFO(amisDaisyHandlerLog,
                    "Syncing navmap to PAGE_TARGET " << uri);

            PageTarget *p_target = (PageTarget*) p_sync_nav;
            NavPoint* p_nav = (NavPoint*) p_sync_nav;

            string type = "unknown";
            string text = "";
            switch (p_target->getType())
            {
            case PageTarget::PAGE_FRONT:
                type = "PAGE_FRONT";
                break;
            case PageTarget::PAGE_NORMAL:
                type = "PAGE_NORMAL";
                break;
            case PageTarget::PAGE_SPECIAL:
                type = "PAGE_SPECIAL";
                break;
            }

            MediaGroup *p_label = p_nav->getLabel();
            if (p_label != NULL && p_label->hasText()
                    && p_label != p_last_label)
                if (p_label->getText()->getTextString().length())
                {
                    text = p_label->getText()->getTextString();
                }

            LOG4CXX_INFO(amisDaisyHandlerLog,
                    "text: '" << text << "' " << "type: " << type);

            mCurrentPage = text;

            //p_sync_nav->print(1);

            if (p_nav_map != NULL)
                p_nav_map->updateCurrent(p_nav);
            if (p_nav_model != NULL)
                p_nav_model->syncLists(p_nav->getPlayOrder());

            p_nav_model->updatePlayOrder(p_nav->getPlayOrder());

            updateNaviLevel(PAGE);

            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncPageList((PageTarget*)p_sync_nav);

            syncPosInfo();

            return true;
        }
        else if (p_sync_nav->getTypeOfNode() == NavNode::NAV_TARGET)
        {
            //NavList* p_list = NULL;
            //p_list = ((NavTarget*)p_sync_nav)->getNavList();

            LOG4CXX_INFO(amisDaisyHandlerLog,
                    "Syncing navmap to NAV_TARGET " << uri);
            //p_sync_nav->print(1);

            NavPoint* p_nav = (NavPoint*) p_sync_nav;

            if (p_nav_map != NULL)
                p_nav_map->updateCurrent(p_nav);
            if (p_nav_model != NULL)
                p_nav_model->syncLists(p_nav->getPlayOrder());

            p_nav_model->updatePlayOrder(p_nav->getPlayOrder());

            MediaGroup *p_label = p_nav->getLabel();
            if (p_label != NULL && p_label->hasText()
                    && p_label != p_last_label)
                if (p_label->getText()->getTextString().length())
                {
                    LOG4CXX_INFO(amisDaisyHandlerLog,
                            "NAV_TARGET: '" << p_label->getText()->getTextString() << "'");
                    p_last_label = p_label;
                }

            //MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavList
            //(p_list, (NavTarget*)p_sync_nav);

            syncPosInfo();

            return true;
        }
        else
        {
            LOG4CXX_ERROR(amisDaisyHandlerLog, "Syncing navmap to Unknown");
            //empty
        }

    }

    return false;
}

/**
 * Sync
 *
 *
 * @param ncxref The ncxref to sync to
 * @param playorder The playorder wanted
 * @return Returns true if syncronisation is successfull
 */
bool DaisyHandler::syncNavModel(std::string ncxref, int playorder)
{

    //LOG4CXX_WARN(amisDaisyHandlerLog, "Synchronizing navmodel to ncxRef: '" << ncxref << "' playorder: '" << playorder );

    // Sync NavModel to ncxref
    if (ncxref != "")
    {
        NavModel *p_nav_model = NavParse::Instance()->getNavModel();
        NavMap *p_nav_map = NULL;
        NavPoint *p_nav = NULL;

        AmisError err = p_nav_model->goToId(ncxref, p_nav);

        if (err.getCode() == amis::OK)
        {
            LOG4CXX_WARN(amisDaisyHandlerLog,
                    "Syncing navmap to NCXREF:" << ncxref);
            //p_nav->print(1);

            if (p_nav_model != NULL)
                p_nav_map = p_nav_model->getNavMap();

            if (p_nav_map != NULL)
                p_nav_map->updateCurrent(p_nav);
            if (p_nav_model != NULL)
                p_nav_model->syncLists(p_nav->getPlayOrder());

            p_nav_model->updatePlayOrder(p_nav->getPlayOrder());

            MediaGroup *p_label = p_nav->getLabel();
            if (p_label != NULL && p_label->hasText())
            {
                LOG4CXX_INFO(amisDaisyHandlerLog,
                        "NCXREF: '" << p_label->getText()->getTextString() << "'");
            }

            syncPosInfo();

            return true;
        }
    }

    // If sync to NCXREF failed, try playorder sync
    if (playorder != -1)
    {
        LOG4CXX_DEBUG(amisDaisyHandlerLog,
                "Trying to sync navmap to playorder " << playorder);
        NavModel *p_nav_model = NavParse::Instance()->getNavModel();
        NavMap *p_nav_map = NULL;

        NavPoint *p_node = (NavPoint*) p_nav_model->getNavMap()->first();

        // Go trough each navnode until we find one we want
        while (p_node != NULL)
        {
            if (p_node->getPlayOrder() >= playorder)
            {
                LOG4CXX_WARN(amisDaisyHandlerLog,
                        "Syncing navmap to playorder:" << currentPos->mNcxRef);

                if (p_nav_model != NULL)
                    p_nav_map = p_nav_model->getNavMap();

                if (p_nav_map != NULL)
                    p_nav_map->updateCurrent(p_node);
                if (p_nav_model != NULL)
                    p_nav_model->syncLists(p_node->getPlayOrder());

                p_nav_model->updatePlayOrder(p_node->getPlayOrder());

                MediaGroup *p_label = p_node->getLabel();
                if (p_label != NULL && p_label->hasText())
                {
                    LOG4CXX_INFO(amisDaisyHandlerLog,
                            "NCXREF: '" << p_label->getText()->getTextString() << "'");
                }

                syncPosInfo();

                return true;
            }

            p_node =
                    (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->next();
        }
    }

    return false;
}

/**
 * Print the content of a SmilMediaGroup
 * Function for debugging
 *
 * @todo Make this print pretty output
 * @param pMedia A smil media group to print
 */
void DaisyHandler::printMediaGroup(SmilMediaGroup* pMedia)
{
    int i;

    LOG4CXX_WARN(amisDaisyHandlerLog, "GROUP ID = " << pMedia->getId());
    if (pMedia->hasText())
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "TEXT = " << pMedia->getText()->getSrc());
        LOG4CXX_WARN(amisDaisyHandlerLog, "TEXT ID =" << pMedia->getText()->getId());
    }

    if (pMedia->hasAudio())
    {
        for (i = 0; i < pMedia->getNumberOfAudioClips(); i++)
        {
            stringstream logstring;
            logstring <<  "\tAUDIO = ";
            logstring << pMedia->getAudio(i)->getSrc()<<", ";
            logstring << "CB = ";
            logstring << pMedia->getAudio(i)->getClipBegin()<<", ";
            logstring << "CE = ";
            logstring << pMedia->getAudio(i)->getClipEnd();
            logstring <<  "\tAUDIO ID = ";
            logstring << pMedia->getAudio(i)->getId();
            LOG4CXX_WARN(amisDaisyHandlerLog, logstring);
        }
    }
    if (pMedia->hasImage())
    {
        LOG4CXX_WARN(amisDaisyHandlerLog, "\tIMAGE = "<<pMedia->getImage()->getSrc());
    }
}

/**
 * Log an amis error
 *
 * @param err The error encountered
 */
void DaisyHandler::reportGeneralError(AmisError err)
{
    lastError.setCode(err.getCode());
    lastError.setMessage(err.getMessage());
    lastError.setFilename(err.getFilename());
    lastError.setSourceModuleName(err.getSourceModuleName());

    switch (err.getCode())
    {
    case amis::OK:
        LOG4CXX_WARN(amisDaisyHandlerLog,
                "OK in " << err.getSourceModuleName() + ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::UNDEFINED_ERROR:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "UNDEFINED_ERROR in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::AT_BEGINNING:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "AT_BEGINNING in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::AT_END:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "AT_END in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::NOT_FOUND:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "NOT_FOUND in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::IO_ERROR:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "IO_ERROR in " << err.getSourceModuleName() << ": "<< err.getMessage() << " while processing " << err.getFilename());
        break;
    case amis::NOT_SUPPORTED:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "NOT_SUPPORTED in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::PARSE_ERROR:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "PARSE_ERROR in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::PERMISSION_ERROR:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "PERMISSION_ERROR in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    case amis::NOT_INITIALIZED:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "NOT INITIALIZED in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    default:
        LOG4CXX_ERROR(amisDaisyHandlerLog,
                "?? in " << err.getSourceModuleName() << ": " << err.getMessage() << " while processing " + err.getFilename());
        break;
    }
}

/**
 * Get the last error stored
 *
 * @return The last error stored
 */
AmisError DaisyHandler::getLastError()
{
    return lastError;
}

/**
 * Print the nav list in the nav model
 * Function for debugging
 */
void DaisyHandler::printNavLists()
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    int num_lists = p_model->getNumberOfNavLists();

    for (int idx = 0; idx < num_lists; idx++)
    {
        NavList* p_list = NULL;
        p_list = p_model->getNavList(idx);

        if (p_list != NULL)
        {
            p_list->print();
            return;
        }
    }

    LOG4CXX_WARN(amisDaisyHandlerLog, "This book doesn't have navlists");
}

/**
 * Print the page list stored in the nav model
 * Function for debugging
 */
void DaisyHandler::printPageList()
{
    NavModel* p_model = NULL;
    p_model = NavParse::Instance()->getNavModel();

    if (p_model != NULL && p_model->hasPages() == true)
    {
        PageList* p_list = NULL;
        p_list = p_model->getPageList();

        if (p_list != NULL)
        {
            p_list->print();
            return;
        }
    }

    LOG4CXX_WARN(amisDaisyHandlerLog, "This book doesn't have a pagelist");
}

/**
 * Print the nav nodes stored in the the nav model
 * Function for debugging
 */
void DaisyHandler::printNavNodes()
{
    NavPoint* p_node = NULL;
    string tmpstr;
    MediaGroup* p_label = NULL;
    //int mExposedDepth = 0;

    p_node =
            (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->first();

    LOG4CXX_WARN(amisDaisyHandlerLog, "Parsing navnodes ");

    while (p_node != NULL)
    {
        p_label = p_node->getLabel();

        if (p_label != NULL)
        {
            if (p_label->hasText() == true)
            {
                tmpstr = p_label->getText()->getTextString().c_str();
            }
        }

        p_node->print(p_node->getLevel());

        p_node =
                (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->next();
    }

    LOG4CXX_WARN(amisDaisyHandlerLog, "starting navmap from the beginning");
    NavPoint* p_nav =
            (NavPoint*) NavParse::Instance()->getNavModel()->getNavMap()->first();
    p_nav->print(1);

    NavParse::Instance()->getNavModel()->syncLists(p_nav->getPlayOrder());
    NavParse::Instance()->getNavModel()->getNavMap()->updateCurrent(p_nav);

    NavParse::Instance()->getNavModel()->updatePlayOrder(p_nav->getPlayOrder());

}

/**
 * Check if the audio node contains given time segment
 *
 * @param audionode The audio node to search in
 * @param offsetSeconds Start of segment
 * @param remainingOffset End of segment
 * @return true if segment is found and false if it is not found
 */
bool audioNodeContainsTime(AudioNode* audionode, unsigned int offsetSeconds,
        unsigned int& remainingOffset)
{
    string clipBegin = stringReplaceAll(audionode->getClipBegin(), "npt=", "");
    string clipEnd = stringReplaceAll(audionode->getClipEnd(), "npt=", "");

    long begTime = parseTime(clipBegin);
    long endTime = parseTime(clipEnd);

    if ((begTime / 1000) < offsetSeconds && offsetSeconds < (endTime / 1000))
    {
        remainingOffset = offsetSeconds - (begTime / 1000);
        return true;
    }

    return false;
}

/**
 * Search for audio ref recursively in nodes
 *
 * @param node Current node
 * @param offset Offset for current node
 * @param containerid Id for the container node
 * @param audioref Audio ref to search for
 * @param textrefs Vector of textrefs
 * @param remainingOffset The remaining offset
 * @return Returns true if audio ref is found, and false if node is NULL or ref is not found
 */
bool findAudioRefRecursive(Node* node, unsigned int offset, string& containerid,
        string& audioref, vector<string>& textrefs,
        unsigned int& remainingOffset)
{
    if (node == NULL)
        return false;

    while (node != NULL)
    {
        // A TimeContainer is either a SeqNode or a ParNode
        TimeContainerNode* timecontainer =
                dynamic_cast<TimeContainerNode*>(node);
        if (timecontainer != NULL)
        {
            //LOG4CXX_DEBUG(amisDaisyHandlerLog, "TimeContainerNode with " << timecontainer->NumChildren() << " children");
            for (int i = 0; i < timecontainer->NumChildren(); ++i)
            {
                if (findAudioRefRecursive(timecontainer->getChild(i), offset,
                        containerid, audioref, textrefs, remainingOffset))
                {
                    if (containerid.empty())
                    {
                        // pick up the first non empty container id on the way back
                        containerid = timecontainer->getElementId();
                    }
                    // Found audio ref
                    return true;
                }
            }

        }
        else
        {
            // A ContentNode contains a MediaNode which is either Audio, Text, or Image
            ContentNode* contentnode = dynamic_cast<ContentNode*>(node);
            if (contentnode != NULL)
            {
                MediaNode* media = contentnode->getMediaNode();
                AudioNode* audionode = dynamic_cast<AudioNode*>(media);
                if (audionode != NULL)
                {
                    if (audioNodeContainsTime(audionode, offset,
                            remainingOffset))
                    {
                        audioref = audionode->getId();
                        return true;
                    }
                }

                TextNode* textnode = dynamic_cast<TextNode*>(media);
                if (textnode != NULL)
                {
                    textrefs.push_back(textnode->getId());
                }
            }
        }
        node = node->getFirstSibling();
    }

    return false;
}

/**
 * Searches for the closest audio ref to offset
 *
 * @param tree The smil tree to look in
 * @param offset The offset to find
 * @param containerid The container id
 * @param audioref The audio reference
 * @param textrefs The text reference
 * @param remainingOffset The remaining offset
 * @return Returns true if found
 */
bool findClosestAudioRef(SmilTree* tree, unsigned int offset,
        string& containerid, string& audioref, vector<string>& textrefs,
        unsigned int& remainingOffset)
{
    return findAudioRefRecursive(tree->getRoot(), offset, containerid, audioref,
            textrefs, remainingOffset);
}

/**
 * Jump to a given second
 *
 * @param seconds The target second
 * @return Returns true if the jump was done.
 */
bool DaisyHandler::jumpToSecond(unsigned int seconds)
{
    BinarySmilSearch search;
    SmilTreeBuilder* treebuilder = search.begin();
    BinarySmilSearch::searchDirection direction = BinarySmilSearch::DOWN;
    try
    {
        while (treebuilder != NULL)
        {
            if (search.currentSmilIsBeyond(seconds))
            {
                direction = BinarySmilSearch::DOWN;
            }
            else
            {
                if (search.currentSmilContains(seconds))
                {
                    LOG4CXX_INFO(amisDaisyHandlerLog,
                            "Smil file: " << search.getCurrentSmilPath() << " contains " << seconds << " seconds position");
                    SmilTree* tree = search.getCurrentSmilTree();
                    string smilStartsAtStr = treebuilder->getMetadata(
                            "ncc:totalelapsedtime");
                    unsigned int offset = seconds
                            - stringToSeconds(smilStartsAtStr);
                    LOG4CXX_INFO(amisDaisyHandlerLog,
                            "Looking for offset=" << offset << "s");
                    string containerid;
                    string audioref;
                    vector<string> textrefs;
                    unsigned int remainingOffset = 0;
                    bool audioRefFound = findClosestAudioRef(tree, offset,
                            containerid, audioref, textrefs, remainingOffset);
                    if (not audioRefFound)
                    {
                        LOG4CXX_WARN(amisDaisyHandlerLog,
                                "JUMP TO SECOND: Audio ref not found, starting from beginning of smil file");
                    }
                    if (not containerid.empty())
                    {
                        containerid = "#" + containerid;
                    }
                    bool smilContentLoaded = loadSmilContent(
                            search.getCurrentSmilPath() + containerid, audioref,
                            remainingOffset);

                    if (smilContentLoaded)
                    {
                        // Locate the correct position in the navmap
                        bool syncSuccess = false;
                        vector<string>::const_reverse_iterator rev_iter =
                                textrefs.rbegin();
                        while (not syncSuccess and (rev_iter != textrefs.rend()))
                        {
                            const string previous_textref = *rev_iter;
                            ++rev_iter;
                            syncSuccess = syncNavModel(
                                    search.getCurrentSmilPath(),
                                    previous_textref);
                        }
                        if (not syncSuccess)
                        {
                            ostringstream oss;
                            oss
                                    << "Failed to syncNavModel to any of the located textrefs ( ";
                            copy(textrefs.rbegin(), textrefs.rend(),
                                    std::ostream_iterator<const string>(oss,
                                            " "));
                            oss << " ) for audioref: " << audioref << endl;
                            LOG4CXX_INFO(amisDaisyHandlerLog, oss.str());
                        }
                    }

                    return smilContentLoaded;
                }
                direction = BinarySmilSearch::UP;
            }

            treebuilder = search.next(direction);
        }
    } catch (int)
    {
        return false;
    }

    return false;
}

/**
 * Returns index of current section
 *
 * @param root The root node to start from
 * @param currentPlayOrder The playorder to look for
 * @param found Boolean indikating if the wanted item was found
 * @return Returns the index or -1 if not found
 */
int currentSection(NavPoint* root, const int currentPlayOrder, bool& found)
{
    if (root == NULL)
        return -1;

    // In case playorder is == we are at that idx
    if (root->getPlayOrder() == currentPlayOrder)
    {
        found = true;
        return 0;
    }

    // In case playorder > we have gone too far, abort and return previous idx
    if (root->getPlayOrder() > currentPlayOrder)
    {
        found = true;
        return -1;
    }

    int childCount = root->getNumChildren();
    int countedChildren = 0;
    for (int cnt = 0; cnt < childCount; ++cnt)
    {
        NavPoint* node = root->getChild(cnt);

        countedChildren += currentSection(node, currentPlayOrder, found) + 1;
        if (found)
            break;
    }

    return countedChildren;
}

/**
 * Count sections in a navigation tree
 *
 * @param root The root node to start from
 * @return Returns the number of sections in tree
 */
int countSections(NavPoint* root)
{
    if (root == NULL)
        return -1;

    int childCount = root->getNumChildren();
    int countedChildren = 0;
    for (int cnt = 0; cnt < childCount; ++cnt)
    {
        NavPoint* node = root->getChild(cnt);

        countedChildren += countSections(node) + 1;
    }

    return countedChildren;
}

/**
 * Get the index of given play order item
 *
 * @param list The list of nodes
 * @param currentPlayOrder Playorder item wanted
 * @return Returns the index for the item with given playorder
 */
int currentNavContainerIdx(NavContainer* list, const int currentPlayOrder)
{
    if (list == NULL)
        return -1;

    NavNode* node = list->first();

    int index = -1;

    while (node != NULL)
    {
        // We want index to return the position of the 
        // previous node so store all we traverse
        if (node->getPlayOrder() <= currentPlayOrder)
            index++;

        node = list->next();
    }

    return index;
}

/**
 * Convert a time to string
 *
 * @param timeInfo The time to convert
 * @return Returns the time as a string
 */
std::string DaisyHandler::tmToTimeString(struct tm timeInfo)
{
    ostringstream timeString;
    timeString << timeInfo.tm_hour;
    timeString << ":";
    if (timeInfo.tm_min < 10)
        timeString << "0";
    timeString << timeInfo.tm_min;
    timeString << ":";
    if (timeInfo.tm_sec < 10)
        timeString << "0";
    timeString << timeInfo.tm_sec;
    return timeString.str();
}

/**
 * Convert a date to string
 *
 * @param TimeInfo The date to convert
 * @return Returns the date as a string
 */
std::string DaisyHandler::tmToDateString(struct tm timeInfo)
{
    ostringstream dateString;
    dateString << timeInfo.tm_year + 1900;
    if (timeInfo.tm_mon != -1 && timeInfo.tm_mday != -1)
    {
        dateString << "-";
        if (timeInfo.tm_mon < 9)
            dateString << "0";
        dateString << timeInfo.tm_mon + 1;
        dateString << "-";
        if (timeInfo.tm_mday < 10)
            dateString << "0";
        dateString << timeInfo.tm_mday;
    }
    return dateString.str();
}
