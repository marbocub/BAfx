/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "TextFileView.h"

#include <View.h>
#include <TextView.h>
#include <ScrollView.h>
#include <cstdio>


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
		for (int32 i=0; i<1000; i++) {
			char s[1000];
			sprintf(s, "%5d: 123456789012345678901234567890123456789012345678901234567890123456789012345\n", i+1);
			Insert(s);
		}
	}


	virtual void MakeFocus(bool flag)
	{
		DEBUG_PRINT(("%s:%s\n", __FUNCTION__, flag ? "true":"false"));
		if (!flag) {
			Select(TextLength()-1, TextLength()-1);
			ScrollToSelection();
		}
	}


	void
	KeyDown(const char* bytes, int32 numBytes)
	{
		DEBUG_PRINT(("%s:%02x\n", __FUNCTION__, bytes[0]));

		int32 cmd;

		if (numBytes == 1) {
			switch (bytes[0]) {
			case B_UP_ARROW:
				ScrollBy(0,-1);
				break;
			case B_DOWN_ARROW:
				ScrollBy(0,1);
				break;
			default:
				BTextView::KeyDown(bytes, numBytes);
				break;
			}
		}
	}
};


TextFileView::TextFileView(BRect frame, const char* name, uint32 resizingMode, uint32 flags)
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
		B_FOLLOW_ALL_SIDES, 0, false, true, B_NO_BORDER);
	AddChild(mScrollView);
}


TextFileView::~TextFileView()
{
	mScrollView->RemoveChild(mTextView);
	delete mTextView;
	RemoveChild(mScrollView);
	delete mScrollView;
}


void
TextFileView::AttachedToWindow()
{
	if (Parent()) {
		SetViewColor(Parent()->ViewColor());
		SetHighColor(Parent()->HighColor());
		SetLowColor (Parent()->LowColor() );
	}
//	FrameResized(Bounds().Width(), Bounds().Height());
//	mScrollView->ResizeTo(Bounds().Width()+2, Bounds().Height()+2);
	
	mScrollView->MoveTo(0,0);
	mScrollView->ResizeTo(Bounds().Width()+1, Bounds().Height());
	BView::AttachedToWindow();
	
	MakeFocus(true);
}


void
TextFileView::MakeFocus(bool focused)
{
		DEBUG_PRINT(("%s:%s\n", __FUNCTION__, flag ? "true":"false"));
	mTextView->MakeFocus(focused);
}


void
TextFileView::KeyDown(const char* bytes, int32 numBytes)
{
	DEBUG_PRINT(("%s:%02x\n", __FUNCTION__, bytes[0]));

	int32 cmd;

	if (numBytes == 1) {
		switch (bytes[0]) {
		case B_UP_ARROW:
			mScrollView->ScrollBy(0,-1);
			break;
		case B_DOWN_ARROW:
			mScrollView->ScrollBy(0,1);
			break;
		default:
			BView::KeyDown(bytes, numBytes);
			break;
		}
	}
}


void
TextFileView::SetBackgroundColor(rgb_color color)
{
	mTextView->SetLowColor(color);
	mTextView->SetViewColor(color);
}


void
TextFileView::SetTextColor(rgb_color color)
{
	BFont font;
	mTextView->GetFontAndColor(0, &font);
	mTextView->SetFontAndColor(&font, B_FONT_ALL, &color);
	mTextView->SetHighColor(color);
}


void
TextFileView::PutText(const char* text)
{
	mTextView->Insert(text);
	mTextView->Select(mTextView->TextLength()-1, mTextView->TextLength()-1);
	mTextView->ScrollToSelection();
	mTextView->Invalidate();
}
