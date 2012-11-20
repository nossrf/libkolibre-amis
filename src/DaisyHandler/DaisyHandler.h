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

//Daisy Handler header

#ifndef DAISYHANDLER_H
#define DAISYHANDLER_H

#include "AmisError.h"
#include "SmilEngine.h"
#include "Media.h"

#include <pthread.h>

#ifdef WIN32
#define DEFAULT_BOOKMARK_PATH "C:\\"
#ifdef AMIS_DLL
#define DAISYHANDLER_API __declspec(dllexport)
#else
#define DAISYHANDLER_API __declspec(dllimport)
#endif
#else
#define DAISYHANDLER_API
#define DEFAULT_BOOKMARK_PATH "/tmp"
#endif


class HistoryRecorder;
namespace amis
{
class BookmarkFile;
class PositionData;

// This class encapsulates the Daisy book handling functions
class DAISYHANDLER_API DaisyHandler
{
protected:
    DaisyHandler();

public:
    static DaisyHandler* Instance();
    void DestroyInstance();
    ~DaisyHandler();

    // Initialization
    bool setBookmarkPath(std::string path);

    // Opens a book, gets associated bookmarks, sets up stuff necessary for playback
    bool openBook(std::string);

    // Setup bookmarks, initial phrase etc..
    bool setupBook();

    // Check if we already have a book opened
    bool bookOpen();

    // Closes the currently open book
    void closeBook();

    /**
     * States of the DaisyHandler
     */
    enum HandlerState
    {
        HANDLER_CLOSED, /**< handler is closed */
        HANDLER_OPENING,/**< handler is opening */
        HANDLER_OPEN,   /**< handler is open*/
        HANDLER_READY,  /**< handler is ready */
        HANDLER_ERROR   /**< the handler is in error state */
    };
    HandlerState getState() const;
    void setState(HandlerState);

    // Function gets called when daisyhandler wants to play audio
    void setPlayFunction(bool (*ptr)(std::string, long long, long long));

    // Function gets called when daisyhandler wants to play audio,
    // this variant supports data passing (OO)
    void setPlayFunction(bool (*ptr)(std::string, long long, long long, void *),
            void *);

    /**
     * Available navigation levels
     */
    enum NaviLevel
    {
        TOPLEVEL,//!< close book
        BEGEND,  //!< navigate between beginning or end
        BOOKMARK,//!< navigate between bookmarks
        HISTORY, //!< navigate between history points
        H1,      //!< navigate between header level 1
        H2,      //!< navigate between header levels 2 and higher
        H3,      //!< navigate between header levels 3 and higher
        H4,      //!< navigate between header levels 4 and higher
        H5,      //!< navigate between header levels 5 and higher
        H6,      //!< navigate between header levels 6 and higher
        PAGE,    //!< navigate between pages
        PHRASE,  //!< navigate between phrases
        TIME     //!< navigate in time
    };
    NaviLevel getNaviLevel();
    std::string getNaviLevelStr();
    void printNaviLevel();
    bool increaseNaviLevel();
    bool decreaseNaviLevel();

    void printNaviPos();

    /**
     * A structure to represent a position in a book
     */
    struct PosInfo
    {
        long currentSmilms; /**< current time (ms) in the smil */
        long totalSmilms; /**< total time (ms) in the smil */
        int currentPageIdx; /**< index of current page */
        int currentSectionIdx; /**< index of current section */
        bool hasCurrentTime; /**< is current time available */
        struct tm mCurrentTime; /**< the current time struct */
        int mPercentRead; /**< the procent read of the book */
    } mPosInfo;

    PosInfo *getPosInfo();

    /**
     * A structure containing information about the book
     */
    struct BookInfo
    {
    	std::string mTitle; /**< The title of the book */
    	std::string mUri; /**< The URI of the book */
        amis::MediaGroup mpTitle; /**< The title stored as a MediaGroup class */

        /**
         * @name Set info?
         */
        bool hasSetInfo; /**< is there set info available */
        int mCurrentSet;
        int mMaxSet;

        /**
         * @name NCC
         */
        int mLevels; /**< number of levels */
        int mCurrentLevel; /**< current navigation level */
        int mTocItems; /**< number of TOC items */
        int mSections; /**< number of sections */

        /**
         * @name Daisy format and content type
         */
        bool hasDaisyType; /**< is the daisy type available */
        int mDaisyType; /**< the daisy type */
        int mContentType; /**< the content type */

        /**
         * @name Pages
         */
        bool hasPages; /**< is there pages available */
        int mFrontPages; /**< number of front pages */
        int mNormalPages; /**< number of normal pages */
        int mSpecialPages; /**< number of special pages */

        int mMinPageNum; /**< the smallest page number */
        int mMaxPageNum; /**< the largest page number */

        /**
         * @name Time
         */
        bool hasTime; /**< is time info available */
        struct tm mTotalTime; /**< total time of content */

        /**
         * @name Dates
         */
        bool hasProdDate; /**< is a production date available */
        struct tm mProdDate; /**< the production date */

        bool hasSourceDate; /**< is a source date available */
        struct tm mSourceDate; /**< the source date */

        bool hasRevisionDate; /**< is a revision date available */
        int mRevisionNumber; /**< the revision date */
        struct tm mRevisionDate; /**< the revision date structure */

    } mBookInfo;

    // Book info
    BookInfo *getBookInfo();

    // Returns the last error that occurred
    amis::AmisError getLastError();

    // History
    bool nextHistory();
    bool previousHistory();
    bool loadLastHistory();
    void printHistory();

    // Skippable items
    int numCustomTests();
    std::string getCustomTestId(unsigned int);
    int getCustomTestState(unsigned int); // Gets status of custom test (returns -1 if setting does not exist)
    int getCustomTestState(std::string); // Gets status of custom test (returns -1 if setting does not exist)
    int setCustomTestState(unsigned int, bool); // Sets status of custom test (returns -1 if setting does not exist)

    // Bookmarks
    bool addBookmark();
    bool nextBookmark();
    bool previousBookmark();

    unsigned int getNumberOfBookmarks();
    int getCurrentBookmarkId();
    int getNextBookmarkId();
    bool deleteCurrentBookmark();
    bool deleteAllBookmarks();

    // Navigation
    bool nextPhrase(bool rewindWhenEndOfBook = false);
    bool previousPhrase();

    // NavPoint Navigation
    bool nextSection();
    bool previousSection();

    // Jump to beginning and end of book
    bool firstSection(bool skipTitle = false);
    bool lastSection();

    // Jump to id
    bool goToId(std::string);

    // Page Navigation
    bool nextPage();
    bool previousPage();
    bool goToPage(std::string);
    bool firstPage();
    bool lastPage();
    int currentPage(); // According to playorder
    std::string getCurrentPage(); // According to stored value

    // Debug functions
    void printNavLists();
    void printPageList();
    void printNavNodes();

    // Info functions
    bool playTitle(amis::MediaGroup* pMedia);

    bool jumpToSecond(unsigned int seconds);

    void reportGeneralError(AmisError err);
    std::string getFilePath() const;

private:
    amis::BookmarkFile* mpBmk;
    amis::PositionData* currentPos;
    HistoryRecorder* mpHst;
    SmilMediaGroup* mpCurrentMedia;
    amis::AmisError lastError;

    enum NaviDirection
    {
        FORWARD, BACKWARD
    };
    NaviDirection naviDirection;

    int mCurrentBookmark;
    std::string mCurrentPage;

    HandlerState handlerState;

    bool escape();

    bool SmilMediaGroup_has_AudioRef(SmilMediaGroup*, std::string);
    bool loadSmilContent(std::string, std::string audioRef = "",
            unsigned int offsetSecond = 0);

    bool nextInNavList(int);
    bool prevInNavList(int);

    bool (*PlayFunction)(std::string, long long, long long);
    bool callPlayFunction(std::string, long long, long long);

    bool (*OOPlayFunction)(std::string, long long, long long, void*);
    bool callOOPlayFunction(std::string, long long, long long);
    void *OOPlayFunctionData;

    void continuePlayingMediaGroup(unsigned int offsetSecond = 0);
    bool syncPosInfo();
    bool syncNavModel(std::string uri, std::string textref);
    bool syncNavModel(std::string ncxref = "", int playorder = -1);
    inline double convertToDouble(const std::string& s);
    inline int convertToInt(const std::string& s);

    bool playMediaGroup(SmilMediaGroup* pMedia, unsigned int offsetSecond = 0);

    SmilMediaGroup* getCurrentMediaGroup();
    std::string getBookFilePath();

    void printMediaGroup(SmilMediaGroup*);

    // Refresh data in mBookInfo
    void readBookInfo();

    //return the name of the bmk file
    std::string setUpBookmarks(std::string, std::string);
    bool selectBookmark(int idx);

    bool mbStartAtLastmark;
    bool mbFlagNoSync;

    std::string mFilePath;
    std::string mBmkPath;
    std::string mBmkFilePath;
    std::string mLastmarkUri;

    friend void *open_thread(void *handler);
    void join_threads();

protected:
    // Private data
    time_t autonaviStartTime;
    NaviLevel currentNaviLevel;

    bool updateNaviLevel(NaviLevel newLevel);

    // Threading used when opening a book
    pthread_t handlerThread;
    bool handlerThreadActive;
    pthread_mutex_t *handlerMutex;
    pthread_mutex_t *callbackMutex;

private:
    static DaisyHandler* pinstance;
    std::string tmToTimeString(struct tm timeInfo);
    std::string tmToDateString(struct tm timeInfo);
};

}
#endif