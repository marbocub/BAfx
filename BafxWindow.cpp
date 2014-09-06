/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "BafxWindow.h"
#include "BafxMessage.h"

#include <Application.h>
#include <Window.h>
#include <ScrollBar.h>


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


BafxWindow::BafxWindow (BRect frame, const char *title,
		window_type type, uint32 flags, uint32 workspace)
		: BWindow(frame, title, type, flags, workspace),
		  contentView(NULL)
{
}


BafxWindow::~BafxWindow()
{
}


void
BafxWindow::ReplaceContent(BView* inContent)
{
	Lock();

	if (contentView != NULL) {
		RemoveChild(contentView);
		contentView = NULL;
	}

	BMenuBar* menuBar = KeyMenuBar();
	BRect menuFrame = menuBar->Frame();
	BRect rect = Bounds();

	rect.top += menuFrame.Height()+1;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	inContent->ResizeTo(rect.Width(), rect.Height());
	inContent->MoveTo(rect.left, rect.top);

	AddChild(inContent);
	contentView = inContent;

	Unlock();
}


void
BafxWindow::InitContent(BMenuBar* inMenuBar, BView* inContent, BView* inStatus)
{
	AddChild(inMenuBar);

	ReplaceContent(inContent);

	BRect rect = Bounds();
	rect.top = rect.bottom - B_H_SCROLL_BAR_HEIGHT + 1;
	inStatus->ResizeTo(rect.Width(), rect.Height());
	inStatus->MoveBy(rect.left, rect.top);
	AddChild(inStatus);
}


void
BafxWindow::InitContent(BMenuBar* inMenuBar, BView* inContent)
{
	AddChild(inMenuBar);

	ReplaceContent(inContent);
}


void
BafxWindow::MessageReceived(BMessage* message)
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
	default:
		BWindow::MessageReceived(message);
		break;
	}
}


void
BafxWindow::Quit (void)
{
	be_app->PostMessage(B_QUIT_REQUESTED, be_app);
	BWindow::Quit();
}
