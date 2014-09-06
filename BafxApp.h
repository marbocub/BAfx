/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef BAFX_APP_H
#define BAFX_APP_H

#include <SupportDefs.h>
#include <app/Application.h>
#include <MenuBar.h>

#include "BafxWindow.h"
#include "FilerView.h"
#include "TextFileView.h"
//#include "EntryModel.h"


class BafxApp : public BApplication
{
private:
	BafxWindow*		mainWindow_;
	FilerView*		filerView_;
	TextFileView*	textViewerView_;

public:
	BafxApp();
	~BafxApp();
	void MessageReceived(BMessage* message);

private:
	void ReadyToRun();
 	BMenuBar* MakeMenuBar();
	void AboutRequested();
};


#endif // BAFX_APP_H
