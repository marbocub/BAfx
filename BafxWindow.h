/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef BAFX_WINDOW_H
#define BAFX_WINDOW_H

#include <SupportDefs.h>
#include <Window.h>
#include <MenuBar.h>


class BafxWindow : public BWindow
{
private:
	BView*		contentView;
public:
	BafxWindow(BRect frame, const char* title, window_type type, uint32 flags,
		uint32 workspace = B_CURRENT_WORKSPACE);
	~BafxWindow();
	void ReplaceContent(BView* inContent);
	void InitContent(BMenuBar* inMenuBar, BView* inContent, BView* inStatus);
	void InitContent(BMenuBar* inMenuBar, BView* inContent);

	void Quit(void);
	void MessageReceived(BMessage* message);
};

#endif // BAFX_WINDOW_H
