/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "BafxApp.h"
#include "BafxMessage.h"

#include <Alert.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>

#include "FilerView.h"
#include "EntryModel.h"
#include "TextFileView.h"

const char kAboutMsg[] = 
	"BAfx file manager\n"
	"\n"
	"Version 0.2.3\n"
	"\n"
	"Copyright " B_UTF8_COPYRIGHT " 2012-2014 @marbocub.\n"
	"Distributed under the terms of the MIT license.";


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


BafxApp::BafxApp()
	: BApplication("application/x-vnd.marbocub-BafxApp"),
	  mainWindow_(NULL),
	  filerView_(NULL),
	  textViewerView_(NULL)
{
}


BafxApp::~BafxApp()
{
    bafx_free_keymap();
}


void
BafxApp::ReadyToRun()
{
	mainWindow_ = new BafxWindow(
		BRect(100,100,100+640,100+480),
		"BAfx App",
		B_DOCUMENT_WINDOW,
		0
	);

	filerView_ = new FilerView(
		BRect(0,0,0,0),
		"double_filer_view",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW
	);

	textViewerView_ = new TextFileView(
		BRect(0,0,0,0),
		"text_file_view",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW
	);

	BMenuBar* menuBar = MakeMenuBar();
	mainWindow_->InitContent(
		menuBar,
		filerView_,
/*
		new BView(
			BRect(0,0,0,0),
			"status_view",
			B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT,
			B_WILL_DRAW
		)
*/
		new BStringView(
			BRect(0,0,0,0),
			"status_string",
			"",
			B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT
		)
	);

	filerView_->OpenDirectory1("/boot/home/");
	filerView_->OpenDirectory2("/boot");

	mainWindow_->Show();
}


BMenuBar*
BafxApp::MakeMenuBar()
{
	BMenuBar*	theMenuBar;
	BMenu*		theFileMenu;
	BMenu*		theEditMenu;
	BMenu*		theDevelMenu;

	theMenuBar  = new BMenuBar(BRect(0,0,0,0), B_EMPTY_STRING);

	theFileMenu = new BMenu("BAfx");
	theMenuBar->AddItem(theFileMenu);
	(void)theFileMenu->AddItem(
		new BMenuItem(
			"About...",
			new BMessage(B_ABOUT_REQUESTED)
		)
	);
	(void)theFileMenu->AddSeparatorItem();
	(void)theFileMenu->AddItem(
		new BMenuItem(
			"Quit",
			new BMessage(B_QUIT_REQUESTED),
			'Q'
		)
	);
	theFileMenu->SetTargetForItems(this);

	theEditMenu = new BMenu("Edit");
	theMenuBar->AddItem(theEditMenu);
	(void)theEditMenu->AddItem(
		new BMenuItem(
			"Copy marked items",
			new BMessage(M_COPY_FILE),
			'C'
		)
	);
	(void)theEditMenu->AddItem(
		new BMenuItem(
			"Remove marked items",
			new BMessage(M_DELETE_FILE),
			'D'
		)
	);
	theEditMenu->SetTargetForItems(this);

	theDevelMenu = new BMenu("Develop");
#ifdef DEBUG
	theMenuBar->AddItem(theDevelMenu);
#endif
	(void)theDevelMenu->AddItem(
		new BMenuItem(
			"Text Viewer",
			new BMessage(M_VIEW_FILE),
			'V'
		)
	);
	(void)theDevelMenu->AddItem(
		new BMenuItem(
			"Return to Filer",
			new BMessage(M_RETURN_TO_FILER),
			'F'
		)
	);
	theDevelMenu->SetTargetForItems(this);

	return theMenuBar;
}


void
BafxApp::AboutRequested()
{
	BAlert*	alertPanel;

	alertPanel = new BAlert("about BAfx", kAboutMsg, "OK");
	(void)alertPanel->Go(NULL);

	return;
}


void
BafxApp::MessageReceived(BMessage* message)
{
	DEBUG_PRINT(("%s:message:%c%c%c%c\n",
		__FUNCTION__,
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what));
	DEBUG_PRINT(("CountNames: %d\n", message->CountNames(B_ANY_TYPE)));

	switch(message->what)
	{
	case M_COPY_FILE:
		DEBUG_PRINT(("Copy"));
		break;
	case M_DELETE_FILE:
		DEBUG_PRINT(("Delete"));
		break;
	case M_VIEW_FILE:
		mainWindow_->ReplaceContent(textViewerView_);
		break;
	case M_RETURN_TO_FILER:
		mainWindow_->ReplaceContent(filerView_);
		break;
	default:
		BApplication::MessageReceived(message);
		break;
	}
}
