/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef FILER_VIEW_H
#define FILER_VIEW_H

#include <SupportDefs.h>
#include <View.h>

#include "PanedView.h"
#include "DirectoryView.h"
#include "LogView.h"


class FilerView : public BView {

public:
	FilerView(BRect frame, const char* name, uint32 resizing_mode, uint32 flags);
	~FilerView();
	void AttachedToWindow();
	void FrameResized(float width, float height);
	void MessageReceived(BMessage *message);

	void OpenDirectory1(const char* name);
	void OpenDirectory2(const char* name);

private:
	PanedView*		h_paned_view_;
	DirectoryView*	dir_view1_;
	DirectoryView*	dir_view2_;
	PanedView*		v_paned_view_;
	LogView*		log_view_;
};

#endif // FILER_VIEW_H
