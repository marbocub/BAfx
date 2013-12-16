/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
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
#include "FileUtils.h"

class BafxWindow : public BWindow
{
private:
	enum BAFX_WINDOW_OPCODE {
		OP_COPY,
		OP_MOVE,
		OP_REMOVE,
		OP_MKDIR,
		OP_UPDIR,
		OP_CHDIR,
		OP_RMDIR
	};

public:
	BafxWindow(BRect frame, const char* title, window_type type, uint32 flags,
		uint32 workspace = B_CURRENT_WORKSPACE);

	virtual void Quit(void);
	virtual void FrameResized(float width, float heigt);
	virtual void MessageReceived(BMessage* message);

private:
	void PutText(const char* text);
	void PutEntryName(BEntry* entry);

	void CopyMoveRemoveFile(BMessage* message);
	void UnMark(FilerView* sender, BEntry* entry, BList* markedList);

private:
	PanedView*	mPanedView;
	FilerView*	mFilerView1;
	FilerView*	mFilerView2;
	PanedView*	mVPanedView;
	LogView*	mLogView;
};

#endif // BAFX_WINDOW_H
