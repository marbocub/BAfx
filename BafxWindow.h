/*
 * Copyright 2012 @marbocub <marbocub @ google mail>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef BAFX_WINDOW_H
#define BAFX_WINDOW_H

#include <SupportDefs.h>
#include <Window.h>
//#include <TextView.h>

#include "PanedView.h"
#include "FilerView.h"
#include "LogView.h"

class BafxWindow : public BWindow
{
public:
	BafxWindow(BRect frame, const char* title, window_type type, uint32 flags,
		uint32 workspace = B_CURRENT_WORKSPACE);

	virtual void Quit(void);
	virtual void FrameResized(float width, float heigt);
	virtual void MessageReceived(BMessage* message);

	void PutText(const char* text);
	void CopyMoveFile(BList* list, entry_ref to_ref, int32 what);

private:
	PanedView*	mPanedView;
	FilerView*	mFilerView1;
	FilerView*	mFilerView2;
	PanedView*	mVPanedView;
	LogView*	mLogView;
};

#endif // BAFX_WINDOW_H
