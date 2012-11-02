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

#include "SpineBuilder.h"
#include "SmilTreeBuilder.h"
#include "DaisyTest.h"

#include "AmisConstants.h"

#include "AmisError.h"
#include "FilePathTools.h"
#include "Metadata.h"
#include "TitleAuthorParse.h"
#include "BookmarksReader.h"
#include "BookmarksWriter.h"
#include "OpfItemExtract.h"
#include <DataStreamHandler.h>

#include "NavParse.h"

#include <locale.h>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include <assert.h>
#include <iomanip>
#include <sstream>

#include <BinarySmilSearch.h>
#include "setup_logging.h"

using namespace amis;
using namespace std;

DaisyTest* DaisyTest::pinstance = 0;

DaisyTest* DaisyTest::Instance()
{
	if (pinstance == 0)  // is it the first call?
	{  
		pinstance = new DaisyTest; // create sole instance
	}
	return pinstance; // address of sole instance
}

void DaisyTest::DestroyInstance()
{
	delete pinstance;
}

bool mFirstTimeContinuePlayback;

DaisyTest::DaisyTest()
{
    setup_logging();
	mFirstTimeContinuePlayback = true;

	string audio_dir = AUDIO_DIR;

	//init all member variables
	//mpCurrentPrompt = NULL;
	mpBmk = NULL;
	mpCurrentMedia = NULL;
	
	//mCurrentPromptIdx = 0;

	mbFlagNoSync = false;
	mFontSize  = 0;
	mbCanEscapeCurrent = false;
	
	mFilePath = "";
	mBmkFilePath = "";
	mLastmarkUri = "";

	mCurrentVolume = 80;

	mpBmk = NULL;
	mpCurrentMedia = NULL;
}

DaisyTest::~DaisyTest()
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
}

//--------------------------------------------------
void DaisyTest::exit()
{
	//destroy objects!
	//cout << "destorying smilengine" << endl;
	SmilEngine::Instance()->DestroyInstance();

	//cout << "destroying navparse" << endl;
	NavParse::Instance()->DestroyInstance();
	
	//cout << "destroying datastreaminstance" << endl;
	DataStreamHandler::Instance()->DestroyInstance();

	//cout << "destorying metadata" << endl;
	amis::Metadata::Instance()->DestroyInstance();

	

}

//compUid parameter is optional.  it is used to compare the book's UID to a 
//UID and see if they match
//otherwise this is possibly not the book the user intended to open
bool DaisyTest::openFile(string filename, string compUid, bool bStartAtLastmark, bool bIsStartBook)
{
	
	amis::AmisError err;
	SmilMediaGroup* pMedia = NULL;
	pMedia = new SmilMediaGroup();
	unsigned int i;

	//@bug
	//when opening a book that is currently open, AMIS crashes
	//so, as a temporary fix: 	
	//if we are already reading this book, don't open it again
	if (mFilePath.compare(filename) == 0)
	{
		return true;
	}

	mFilePath = filename;
	
	//delete old bookmarks
	if (mpBmk != NULL)
	{
		delete mpBmk;
		mpBmk = NULL;
	}


	if (filename == "")
	{
		err.setCode(amis::NOT_FOUND);
		err.setMessage("File not found: " + filename);
		
		reportGeneralError(err);	
		return false;
	}
	
	cout << "opening file in smilengine.." << endl;
	err = SmilEngine::Instance()->openBook(filename, pMedia);

	if (err.getCode() != amis::OK)
	{
		//err.setMessage("File not found: " + filename);

		reportGeneralError(err);
		return false;
	}
	
	//load the navigation file
	string ext = amis::FilePathTools::getExtension(filename);
	if (ext.compare("opf") == 0)
	{
		//get the navigation filename from the opf file
		amis::OpfItemExtract opf_extr;
		string ncx_path = opf_extr.getItemHref(filename, "ncx");
		if (ncx_path.size() == 0) ncx_path = opf_extr.getItemHref(filename, "NCX");
		cout << "opening file in navparse.." << endl;
		err = NavParse::Instance()->open(ncx_path);

	}
	else
	{
		cout << "opening file in navparse.." << endl;
		err = NavParse::Instance()->open(filename);
		cout << "done opening in navparse.." << endl;
	}

	if (err.getCode() != amis::OK)
	{
		reportGeneralError(err);

		return false;
	}

		
	//load the metadata	
	cout << "loading metadata.." << endl;
	amis::Metadata::Instance()->openFile(filename);
	
	//look for a bookmarks file, create one if does not exist
	string w_uid = amis::Metadata::Instance()->getMetadata("dc:Identifier");

	if (w_uid.size() == 0)
	{
		w_uid = amis::Metadata::Instance()->getMetadata("dc:identifier");
	}
	if (w_uid.size() == 0)
	{
		w_uid = amis::Metadata::Instance()->getMetadata("ncc:identifier");
	}
	
	cout << "getting navmodel.." << endl;
	NavModel* p_nav_model = NULL;
	p_nav_model = NavParse::Instance()->getNavModel();

	//sync the skip options between nav and smil
	if (p_nav_model != NULL)
	{
		for (i=0; i<p_nav_model->getNumberOfCustomTests(); i++)
		{
			cout << "adding skipoption.." << endl;
			SmilEngine::Instance()->addSkipOption(p_nav_model->getCustomTest(i));
		}
	}

	if (p_nav_model != NULL)
	{
		bool has_pgs = p_nav_model->hasPages();
		cout << "checking for pages.." << endl;

	}

	cout << "getting title.." << endl;

	if (NavParse::Instance()->getNavModel()->getDocTitle() != NULL)
	{
		amis::TextNode* p_txt = NULL;
		p_txt = NavParse::Instance()->getNavModel()->getDocTitle()->getText();
		if (p_txt != NULL)
		{
			string title = p_txt->getTextString().c_str();
			cout << "title: " << title << endl;
		}
	}

	//load lastmark
	if (mpBmk != NULL && bStartAtLastmark == true)
	{
		amis::PositionData* pos_data = NULL;
		pos_data = mpBmk->getLastmark();

		if (pos_data != NULL)
		{
			if (pos_data->mUri.size() > 0)
			{
				string pos = amis::FilePathTools::goRelativePath
					(this->mFilePath, pos_data->mUri);

				loadSmilContent(pos);
			}
		}
		else
		{
			playMediaGroup(pMedia);
		}
	}
	else //if(bStartAtLastmark == false)
	{
		playMediaGroup(pMedia);
	}

	return true;
}



//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::loadSmilContent(string contentUrl, bool flagNoSync)
{
	amis::AmisError err;
	SmilMediaGroup* pMedia = NULL;
	pMedia = new SmilMediaGroup();

	err = SmilEngine::Instance()->loadPosition(contentUrl, pMedia);

	if (err.getCode() == amis::OK)
	{
		playMediaGroup(pMedia);
		return true;

	}
	else
	{
		//some error happened
	}

	return false;
}


bool DaisyTest::escape()
{
	SmilMediaGroup* pMedia = NULL;
	pMedia = new SmilMediaGroup();

	amis::AmisError err;

	err = SmilEngine::Instance()->escapeCurrent(pMedia);

	if (err.getCode() == amis::OK)
	{
		playMediaGroup(pMedia);
		return true;
	}
	return false;
}

//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::nextPhrase(bool rewindWhenEndOfBook)
{
	SmilMediaGroup* pMedia = NULL;
	pMedia = new SmilMediaGroup();

	amis::AmisError err;

	err = SmilEngine::Instance()->next(pMedia);

	if (err.getCode() != amis::OK && err.getCode() != amis::AT_END)
	{
		err.setMessage("Error going to nextPhrase: " + err.getMessage());
		reportGeneralError(err);
	}
	else if (err.getCode() == amis::AT_END)
	{
		if (rewindWhenEndOfBook) {

			SmilMediaGroup* pMediaZ = NULL;
			pMediaZ = new SmilMediaGroup();
		
			amis::AmisError errZ;
			errZ = SmilEngine::Instance()->loadPosition(NavParse::Instance()->getNavModel()->getNavMap()->first()->getContent(), pMediaZ);

			if (errZ.getCode() == amis::OK)
			{
				playMediaGroup(pMediaZ);
				return true;
			}
		}

	}
	else
	{
		playMediaGroup(pMedia);
		return true;
	}
	return false;
}

//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::prevPhrase()
{
	SmilMediaGroup* pMedia = NULL;
	pMedia = new SmilMediaGroup();

	amis::AmisError err;

	err = SmilEngine::Instance()->previous(pMedia);

	if (err.getCode() != amis::OK && err.getCode() != amis::AT_BEGINNING)
	{
		err.setMessage("Error going to prevPhase: " + err.getMessage());
		reportGeneralError(err);
	}
	else if (err.getCode() == amis::AT_BEGINNING)
	{
		//beginning of book
	}
	else
	{
		playMediaGroup(pMedia);
		return true;
	}
	return false;

//	delete pMedia;
}


//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::nextInNavList(int idx)
{
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();

	int num_lists = p_model->getNumberOfNavLists();

	if (idx >=0 && idx < num_lists)
	{
		NavList* p_list = NULL;
		p_list = p_model->getNavList(idx);

		NavTarget* p_navt = NULL;
		p_navt = (NavTarget*)p_list->next();

		if (p_navt != NULL)
		{
			string content_url = p_navt->getContent();
			p_model->updatePlayOrder(p_navt->getPlayOrder());
			return loadSmilContent(content_url);
		}
		
	}
	return false;
}

//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::prevInNavList(int idx)
{
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();

	int num_lists = p_model->getNumberOfNavLists();

	if (idx >=0 && idx < num_lists)
	{
		NavList* p_list = NULL;
		p_list = p_model->getNavList(idx);

		NavTarget* p_navt = NULL;
		p_navt = (NavTarget*)p_list->previous();

		if (p_navt != NULL)
		{
			string content_url = p_navt->getContent();
			p_model->updatePlayOrder(p_navt->getPlayOrder());
			return loadSmilContent(content_url);
		}

		
		
	}

	return false;

}

//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::nextPage()
{
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();
	
	if (p_model != NULL && p_model->hasPages() == true)
	{
		PageList* p_list = NULL;
		p_list = p_model->getPageList();

		PageTarget* p_page = NULL;

		p_page = (PageTarget*)p_list->nextBasedOnPlayOrder(p_model->getPlayOrder());

		if (p_page != NULL)
		{
			string content_url = p_page->getContent();
			p_model->updatePlayOrder(p_page->getPlayOrder());
			return loadSmilContent(content_url);

		}
	}
	return false;
}

//--------------------------------------------------
//--------------------------------------------------
bool DaisyTest::prevPage()
{
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();
	
	if (p_model != NULL && p_model->hasPages() == true)
	{
		PageList* p_list = NULL;
		p_list = p_model->getPageList();

		PageTarget* p_page = NULL;

		p_page = (PageTarget*)p_list->previousBasedOnPlayOrder(p_model->getPlayOrder());

		if (p_page != NULL)
		{
			string content_url = p_page->getContent();
			p_model->updatePlayOrder(p_page->getPlayOrder());
			return loadSmilContent(content_url);

		}
	}
	return false;
}

bool DaisyTest::goToPage(string page_name)
{	
	if (page_name.size() > 0)
	{		
		NavModel* p_model = NULL;
		p_model = NavParse::Instance()->getNavModel();
	
		if (p_model != NULL && p_model->hasPages() == true)
		{
			PageList* p_list = NULL;
			p_list = p_model->getPageList();

			PageTarget* p_page = NULL;

			p_page = (PageTarget*)p_list->findPage(page_name);

			if (p_page != NULL)
			{

				string content_url = p_page->getContent();
				p_model->updatePlayOrder(p_page->getPlayOrder());
				return loadSmilContent(content_url);

			}
		}

	}

	return false;
}

//--------------------------------------------------
//--------------------------------------------------
void DaisyTest::playMediaGroup(SmilMediaGroup* pMedia, bool flagNoSync)
{

	if (mpCurrentMedia != NULL)
	{
		delete mpCurrentMedia;
		mpCurrentMedia = NULL;
	}
	
	if (pMedia != NULL)
	{
		mbFlagNoSync = flagNoSync;
		mpCurrentMedia = pMedia;

		continuePlayingMediaGroup();
	}

	

}

void DaisyTest::continuePlayingMediaGroup()
{
	//USES_CONVERSION;

	if (mpCurrentMedia->hasImage() == true)
	{
		//		AfxMessageBox(_T("has an image"));
		
	}

	std::string src;
	std::string clipBegin;
	std::string clipEnd;

	if (mpCurrentMedia->getNumberOfAudioClips() > 0)
	{

		//@bug
		//this won't work, we have to queue these clips for the rare instance
		//that there is more than one in a group
		//this could happen when we deal with resource files and labels, 
		//and want to couple some audio clips
		amis::AudioNode* p_audio = NULL;

		for (unsigned int i=0; i<mpCurrentMedia->getNumberOfAudioClips(); i++)
		{
			p_audio = mpCurrentMedia->getAudio(i);
			src = p_audio->getSrc();
				
			src = amis::FilePathTools::getAsLocalFilePath(src);
			clipBegin = mpCurrentMedia->getAudio(i)->getClipBegin();
			clipEnd = mpCurrentMedia->getAudio(i)->getClipEnd();

			std::cout <<  "**AUDIO: playing "<< src.c_str() << " from " << clipBegin.c_str() << " to " << clipEnd.c_str() << "**" << endl;

			//AmisAudio::InstanceOne()->play(src.c_str(), 
			//		       (char*)clipBegin.c_str(), 
			//		       (char*)clipEnd.c_str());

				
			if (mFirstTimeContinuePlayback) {
				//CAmisGuiMFCApp* pApp = (CAmisGuiMFCApp *) AfxGetApp();
				//pApp->setPlayPauseState(false);
				mFirstTimeContinuePlayback = false;
			}

		}
	}
	
	//cout << "constructing current uri from smilfilename#id" << endl;
	//printMediaGroup(mpCurrentMedia);
		
	//calculate our current URI
	//get only the smil file name
	string smil_file = SmilEngine::Instance()->getSmilSourcePath();
	string uri = amis::FilePathTools::getFileName(smil_file);

	if (mpCurrentMedia != NULL)
	{
		uri.append("#");
		if(mpCurrentMedia->hasText())
			uri.append(mpCurrentMedia->getText()->getId());
		else
			uri.append(mpCurrentMedia->getId());
	}

	//record the lastmark in the bookmark file
	if (mpBmk != NULL)// && 1 == 0)
	{
		amis::PositionData* p_pos = NULL;

		p_pos = new amis::PositionData;

		p_pos->mUri = uri;
		NavNode* p_node = NULL;

		p_node = NavParse::Instance()->getNavModel()->getNavMap()->current();
		if (p_node != NULL)
		{
			p_pos->mNcxRef = p_node->getId();
		}

		mpBmk->setLastmark(p_pos);

		amis::BookmarksWriter writer;
		writer.saveFile(mBmkFilePath, mpBmk);
	}

	//record our current position
	mLastmarkUri = uri;

	//sync our position with the nav display and data model
	if (mbFlagNoSync == false)
	{
		//bool b_last_value = MainWndParts::Instance()->mpSidebar->m_wndDlg.mb_ignoreNavSelect;

		//MainWndParts::Instance()->mpSidebar->m_wndDlg.mb_ignoreNavSelect = true;

		cout << "NavModel uri: " << uri << endl;
			
		NavNode* p_sync_nav = NULL;
		p_sync_nav = NavParse::Instance()->getNavModel()->goToHref(uri);
		
		if (p_sync_nav != NULL)
		{
			if (p_sync_nav->getTypeOfNode() == NavNode::NAV_POINT)
			{
				cout << "Syncing navmap to NAV_POINT" << endl;
				p_sync_nav->print(1);
				//TRACE(_T("continueplayingmediagroup ... asking syncNavMap\n"));
				//MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavMap((NavPoint*)p_sync_nav);
			}
			else if(p_sync_nav->getTypeOfNode() == NavNode::PAGE_TARGET)
			{
				cout << "Syncing navmap to PAGE_TARGET" << endl;
				p_sync_nav->print(1);
				//MainWndParts::Instance()->mpSidebar->m_wndDlg.syncPageList((PageTarget*)p_sync_nav);
			}
			else if(p_sync_nav->getTypeOfNode() == NavNode::NAV_TARGET)
			{
				//NavList* p_list = NULL;
				//p_list = ((NavTarget*)p_sync_nav)->getNavList();

				cout << "Syncing navmap to NAV_TARGET" << endl;
				//p_sync_nav->print(1);

				//MainWndParts::Instance()->mpSidebar->m_wndDlg.syncNavList
				//(p_list, (NavTarget*)p_sync_nav);

			}
			else
			{
				cout << "Syncing navmap to Unknown" << endl;
				//empty
			}
		}

		//MainWndParts::Instance()->mpSidebar->m_wndDlg.mb_ignoreNavSelect = b_last_value;
	}
	//if mbFlagNoSync == true, then the nav view was the source "click" that
	//loaded this media group
	//so return focus to the nav window
	//did this ever work ??
	else
	{
		//	MainWndParts::Instance()->mpSidebar->SetFocus();
	}
}


//function for debugging
void DaisyTest::printMediaGroup(SmilMediaGroup* pMedia)
{

	//std::ofstream outfile;
	//string logfile = "./mediagroups.log";
	//logfile = amis::FilePathTools::goRelativePath(mAppPath, logfile);

	//outfile.open(logfile.c_str(), ios::app);

	cout<<"GROUP ID = "<<pMedia->getId()<<endl;

	if (pMedia->hasText())
	{
		cout<<"\tTEXT = "<<pMedia->getText()->getSrc()<<endl;
		cout<<"\tTEXT ID ="<<pMedia->getText()->getId()<<endl;
	}

	if (pMedia->hasAudio())
	{
		for (unsigned int i=0; i<pMedia->getNumberOfAudioClips(); i++)
		{
			cout<<"\tAUDIO = ";
			cout<<pMedia->getAudio(i)->getSrc()<<", ";
			cout<<"CB = ";
			cout<<pMedia->getAudio(i)->getClipBegin()<<", ";
			cout<<"CE = ";
			cout<<pMedia->getAudio(i)->getClipEnd()<<endl;
			cout<<"\tAUDIO ID = ";
			cout<<pMedia->getAudio(i)->getId()<<endl;
		}
	}
	if (pMedia->hasImage())
	{
		cout<<"\tIMAGE = "<<pMedia->getImage()->getSrc()<<endl;
	}

	cout<<endl<<endl;


}


void DaisyTest::reportGeneralError(amis::AmisError err)
{
	std::cout << "A general error occurred '" << err.getMessage() << "'" << endl;

}

void DaisyTest::printNavLists() {
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();

	int num_lists = p_model->getNumberOfNavLists();

	for (int idx = 0; idx < num_lists; idx++)
	{
		NavList* p_list = NULL;
		p_list = p_model->getNavList(idx);

		if (p_list != NULL) {
			p_list->print();
			return;
		}
	}

	cout << "This book doesn't have navlists" << endl;
}

void DaisyTest::printPageList() {
	NavModel* p_model = NULL;
	p_model = NavParse::Instance()->getNavModel();
	
	if (p_model != NULL && p_model->hasPages() == true)
	{
		PageList* p_list = NULL;
		p_list = p_model->getPageList();

		if (p_list != NULL) {
			p_list->print();
			return;
		}
	}

	cout << "This book doesn't have a pagelist" << endl;
}

void parsenavnodes() {

        NavPoint* p_node = NULL;
	string tmpstr;
	amis::MediaGroup* p_label = NULL;
	int mExposedDepth = 0;

        p_node = (NavPoint*)NavParse::Instance()->getNavModel()->getNavMap()->first();

	cout << "Parsing navnodes "<< endl;

        //nodes will represent open nodes, one at each level
        //the vector is only as long as the current nested level
        vector<NavPoint *> nodes;

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


                if (p_node->getLevel() == 1)
                {

			//cout << "level 1? do what?" << endl;
                        //hItem = m_wndTree.InsertItem(tmpstr);
                        //hParent = hItem;
                        nodes.clear();
                }
                else
                {
                        //find the right parent node
                        //subtract one to adjust for 0-based array
                        //subtract one more to adjust for what the parent node index should be
                        int lvl = p_node->getLevel() - 2;
                        int sz = nodes.size();

                        //loop backwards, removing a node for each level that cannot be a parent
                        //to the new node
                        for (int i=sz-1; i > lvl; i--)
                        {
                                nodes.pop_back();
                        }

                        //hItem = m_wndTree.InsertItem(tmpstr, nodes.back());//(LPCTSTR)ctmpstr, nodes.back());

                }
                nodes.push_back(p_node);
		p_node->print(p_node->getLevel());
		//cout << "node: "<< tmpstr << endl;
                //m_wndTree.SetItemData(hItem, (DWORD)p_node);

                p_node = (NavPoint*)NavParse::Instance()->getNavModel()->getNavMap()->next();
        }

        mExposedDepth = NavParse::Instance()->getNavModel()->getNavMap()->getMaxDepth();

        //m_wndTree.ShowWindow(SW_SHOW);
}

void nextnextnavnode() {
        NavPoint* p_node = NULL;
	string tmpstr;
	amis::MediaGroup* p_label = NULL;
	int mExposedDepth = 0;

        p_node = (NavPoint*)NavParse::Instance()->getNavModel()->getNavMap()->first();

	cout << "Parsing navnodes "<< endl;

        //nodes will represent open nodes, one at each level
        //the vector is only as long as the current nested level
        vector<NavPoint *> nodes;

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

		p_node = (NavPoint*)NavParse::Instance()->getNavModel()->getNavMap()->next();
        }

}


void nextnextnext() {
	while(DaisyTest::Instance()->nextPhrase());
}

void prevprevprev() {
	while(DaisyTest::Instance()->prevPhrase());
}

void nexth1() {
	

}


bool findSmilAt( unsigned int seconds )
{
    cout << __PRETTY_FUNCTION__ << endl;

    BinarySmilSearch search;
    SmilTreeBuilder* treebuilder = search.begin();
    BinarySmilSearch::searchDirection direction = BinarySmilSearch::DOWN;
    try {
        while( treebuilder != NULL )
        {
            if( search.currentSmilIsBeyond( seconds ) )
            {
                treebuilder = search.next( BinarySmilSearch::DOWN );
            }
            else
            {
                if( search.currentSmilContains( seconds ) )
                {
                    //break; // We found the smil file covering seconds.
                    cout << "Smil file: " << search.getCurrentSmilPath() << " contains " << seconds << " seconds position" << endl;
                    return true;
                }
                treebuilder = search.next( BinarySmilSearch::UP );
            }


        }
    } catch(int param) {
        cout << __PRETTY_FUNCTION__ << ": Search caught an exception: " << param << endl;
        return false;
    }

    cout << __PRETTY_FUNCTION__ << ": no file contains second : " << seconds << endl;
    return false;
}

int main(int argc, char *argv[]) {

	char * currentLocale = setlocale(LC_ALL,"");
	cout << "currentLocale=" << currentLocale  <<  endl;


	std::cout << "NavParse and SmilEngine test.." << endl;
	string meh;

	if(argc < 2) {
		cout << "Please specify an ncc.html file on the command line" << endl;
		exit(-1);
	}
	

	for(int i = 1; i < argc; i++) {
		if(not DaisyTest::Instance()->openFile(argv[i])){
            cout << "Unable to open requested file: " << argv[i] << endl;;
            exit(1);
        }
		NavParse::Instance()->getNavModel()->getNavMap()->print();
		
		DaisyTest::Instance()->printNavLists();
        DaisyTest::Instance()->printPageList();
        //parsenavnodes();
        string totaltime;
        switch( SmilEngine::Instance()->getDaisyVersion() ){
        case DAISY3:
            totaltime = amis::Metadata::Instance()->getMetadata("dtb:totaltime");
            break;
        case DAISY202:
            totaltime = amis::Metadata::Instance()->getMetadata("ncc:totaltime");
            break;
        default:
            cout << "Daisy version not supported: " << SmilEngine::Instance()->getDaisyVersion();
            exit(1);
        }
        int seconds = stringToSeconds( totaltime );
        int test = 0;
        cout << "Test " << test++ << " jump to: " << -1 << endl;
        assert( ! findSmilAt( -1 ) ); // Testar med gränsvärden
        cout << "Test " << test++ << " jump to: " << 0 << endl;
        assert( findSmilAt( 0 ) );
        cout << "Test " << test++ << " jump to: " << seconds/10 << endl;
        assert( findSmilAt( seconds/10 ) ); // en bit in i boken
        cout << "Test " << test++ << " jump to: " << seconds/2 << endl;
        assert( findSmilAt( seconds/2 ) ); // halvvägs
        cout << "Test " << test++ << " jump to: " << seconds << endl;
        assert( findSmilAt( seconds ) ); // slutet
        cout << "Test " << test++ << " jump to: " << seconds+100 << endl;
        assert( ! findSmilAt( seconds+100 ) ); // förbi slutet
    }

	DaisyTest::Instance()->exit();
	DaisyTest::Instance()->DestroyInstance();

}
