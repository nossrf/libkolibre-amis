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

#ifndef AMISCONSTANTS_H
#define AMISCONSTANTS_H
/*
 //test build define variable
 #ifndef AMISTESTBUILD
 #define AMISTESTBUILD 1
 #endif

 #ifdef AMISTESTBUILD
 #define AMIS_VERSION					"Test Build 2.4"
 #define AMIS_VERSIONNOTE				"2006-09-16"
 #endif
 */

#define AMIS_VERSION					"Version 2.5"
#define AMIS_VERSIONNOTE				"Release date: 2006-11-02"

//messages from anyone to amis
#define WM_MESSAGETOAMIS	WM_USER + 100
//messages from amis to plugins
#define WM_MESSAGETOPLUGIN	WM_USER + 200
//messages telling amis to play a prompt
#define WM_MESSAGEPROMPT	WM_USER + 300
//messages telling amis to play a media node
#define WM_MESSAGEMEDIANODE	WM_USER + 400

namespace amis
{

//system commands
enum AmisCommandId
{
    OPEN_FILE,
    LOAD_CD,
    SEARCH_DRIVE,
    TOGGLE_SIDEBAR,
    BASIC_VIEW,
    DEFAULT_VIEW,
    NEXT_PHRASE,
    PREV_PHRASE,
    NEXT_SECTION,
    PREV_SECTION,
    EXIT,
    LARGER_FONT,
    RESET_FONT,
    TOGGLE_CONTRAST,
    SHOW_PREFERENCES,
    SHOW_ABOUT,
    NEXT_PAGE,
    PREV_PAGE,
    GO_TO_PAGE,
    PAUSE, //deprecated
    PLAY, //deprecated
    SLOW,
    NORMAL,
    FAST,
    SET_BMK,
    HELP,
    SHOW_SKIP_OPTIONS,
    ESCAPE,
    SHOW_CURRENT_POSITION_INFO,
    SHOW_NAV_OPTIONS,
    SHOW_PUBLICATION_SUMMARY,
    BOOK_AUDIO_CLIP_FINISHED,
    UI_AUDIO_CLIP_FINISHED,
    UI_TTS_FINISHED,
    PLAY_PROMPT,
    GET_PROMPT,
    PLAY_MEDIANODE,
    FOCUS_ON_SIDEBAR,
    VOLUME_UP,
    VOLUME_DOWN,
    TOGGLE_VIEW,
    SHOW_FIND_IN_TEXT,
    FIND_IN_TEXT_NEXT,
    FIND_IN_TEXT_PREV,
    PLAYPAUSE,
    INCREASE_SECTION_DEPTH,
    DECREASE_SECTION_DEPTH,
    FOCUS_ON_TEXT,
    RESET_HIGHLIGHT_COLORS,
    TOGGLE_AUDIO_CONTENT_PLAYBACK,
    TOGGLE_AUDIO_SELFVOICING_PLAYBACK
};

namespace plugin
{
enum PluginCommandId
{
    PLAY
};
}

}

#define MAX_ANYTHING					49

#define RECENT_BOOK_LIST_FILE			"./settings/config/amisRecentBooks.xml"
#define PREFS_FILE						"./settings/config/amisPrefs.xml"
#define PLUGINS_DIR						"./settings/plugins/";
#define LANG_DIR						"./settings/lang/";
#define IMAGE_DIR						"./settings/img/";

//within the language directory ./settings/lang/LANG_ID/
#define DEFAULT_BOOK					"./start/ncc.html"
#define HELP_BOOK						"./help/ncc.html"
#define AUDIO_DIR						"./audio/";

#define MODULE_DESC_FILE				"moduleDesc.xml"
#define AUDIO_PROMPTS_FILE				"audioPrompts.xml"

#define MAX_RECENT_BOOKS				4

//to add a file type, use this format
//string; string; string
//like: "ncc.*; *.opf"
//be sure to put a semicolon and space between the strings

#define FILE_OPEN_FILTER				"ncc.*; *.opf";

#endif
