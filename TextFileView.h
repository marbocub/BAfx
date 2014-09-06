/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef TEXT_FILE_VIEW_H
#define TEXT_FILE_VIEW_H

#include <SupportDefs.h>

#include <ScrollView.h>
#include <TextView.h>


class TextFileView : public BView
{
public:
	TextFileView(BRect frame, const char* name,
		uint32 resizingMode = B_FOLLOW_ALL,
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	~TextFileView();
	void AttachedToWindow();
	void KeyDown(const char* bytes, int32 numBytes);
	void MakeFocus(bool focused);

	void SetBackgroundColor(rgb_color color);
	void SetTextColor(rgb_color color);
	void PutText(const char* text);

private:
	BScrollView*	mScrollView;
	BTextView*		mTextView;
};

#endif // LOG_VIEW_H
