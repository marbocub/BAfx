/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef LOG_VIEW_H
#define LOG_VIEW_H

#include <SupportDefs.h>

#include <ScrollView.h>
#include <TextView.h>

class LogView : public BView
{
public:
	LogView(BRect frame, const char* name,
		uint32 resizingMode = B_FOLLOW_ALL,
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	~LogView();
	void PutText(const char* text);
	void SetBackgroundColor(rgb_color color);
	void SetTextColor(rgb_color color);

private:
	BScrollView*	mScrollView;
	BTextView*		mTextView;
};

#endif // LOG_VIEW_H
