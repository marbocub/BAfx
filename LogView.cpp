/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "LogView.h"

#include <View.h>
#include <TextView.h>
#include <ScrollView.h>


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


class LogTextView : public BTextView
{
public:
	LogTextView(BRect frame, const char* name, BRect textRect,
		uint32 resizingMode, uint32 flags)
		: BTextView(frame, name, textRect, resizingMode, flags)
	{
		SetFontAndColor(be_fixed_font);
		MakeSelectable(true);
		MakeEditable(false);
//		MakeEditable(true);
		MakeResizable(true);
	}

	virtual void MakeFocus(bool flag)
	{
		DEBUG_PRINT(("%s:%s\n", __FUNCTION__, flag ? "true":"false"));
		if (!flag) {
			Select(TextLength()-1, TextLength()-1);
			ScrollToSelection();
		}
	}
};


LogView::LogView(BRect frame, const char* name, uint32 resizingMode, uint32 flags)
		: BView(frame, name, resizingMode, flags)
{
	mTextView = new LogTextView(
		Bounds(),
		"log_text_view",
		Bounds(),
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_PULSE_NEEDED
	);
	mScrollView = new BScrollView(NULL, mTextView,
		B_FOLLOW_ALL_SIDES, 0, false, true, B_PLAIN_BORDER);
	AddChild(mScrollView);
}


LogView::~LogView()
{
	mScrollView->RemoveChild(mTextView);
	delete mTextView;
	RemoveChild(mScrollView);
	delete mScrollView;
}


void
LogView::SetBackgroundColor(rgb_color color)
{
	mTextView->SetLowColor(color);
	mTextView->SetViewColor(color);
}


void
LogView::SetTextColor(rgb_color color)
{
	BFont font;
	mTextView->GetFontAndColor(0, &font);
	mTextView->SetFontAndColor(&font, B_FONT_ALL, &color);
	mTextView->SetHighColor(color);
}


void
LogView::PutText(const char* text)
{
	mTextView->Insert(text);
	mTextView->Select(mTextView->TextLength()-1, mTextView->TextLength()-1);
	mTextView->ScrollToSelection();
	mTextView->Invalidate();
}
