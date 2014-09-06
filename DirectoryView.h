/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef DIRECTORY_VIEW_H
#define DIRECTORY_VIEW_H

#include <SupportDefs.h>
#include <View.h>
#include <StringView.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Directory.h>
#include <Entry.h>
#include <List.h>

#include "EntryListView.h"
#include "EntryModel.h"


class DirectoryView : public BView
{
private:
	BStringView*	mDirNameView;
	EntryListView*	mListView;
	BScrollView*	mScrollView;
	BView*			mCountView;
	EntryList		entryList_;
	Entry*			entry_;
	Entry*			parent_;

public:
	DirectoryView(BRect frame, const char* name);
	~DirectoryView();

	status_t OpenDirectory(const char* pathname);
	status_t OpenDirectory(const Entry* dir);

	void AttachedToWindow();
	void MakeFocus(bool focused = true);
	void MessageReceived(BMessage *message);

	void SetBackgroundColor (uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetExecutableColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetSelectedColor (uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetReadonlyColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetHiddenColor (uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetCursorColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetFileColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetDirColor(uchar red, uchar green, uchar blue, uchar alpha=0);

	void SetBackgroundColor (rgb_color color);
	void SetExecutableColor(rgb_color color);
	void SetSelectedColor (rgb_color color);
	void SetReadonlyColor(rgb_color color);
	void SetHiddenColor (rgb_color color);
	void SetCursorColor(rgb_color color);
	void SetFileColor(rgb_color color);
	void SetDirColor(rgb_color color);

	rgb_color GetBackgroundColor (void) const;
	rgb_color GetExecutableColor(void) const;
	rgb_color GetSelectedColor (void) const;
	rgb_color GetReadonlyColor(void) const;
	rgb_color GetHiddenColor (void) const;
	rgb_color GetCursorColor(void) const;
	rgb_color GetFileColor(void) const;
	rgb_color GetDirColor(void) const;

private:
	void UpdateStatus();
};


inline void
DirectoryView::SetBackgroundColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetBackgroundColor(color);
}


inline void
DirectoryView::SetExecutableColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetExecutableColor(color);
}


inline void
DirectoryView::SetSelectedColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetSelectedColor(color);
}


inline void
DirectoryView::SetReadonlyColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetReadonlyColor(color);
}


inline void
DirectoryView::SetHiddenColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetHiddenColor(color);
}


inline void
DirectoryView::SetCursorColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetCursorColor(color);
}


inline void
DirectoryView::SetFileColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetFileColor(color);
}


inline void
DirectoryView::SetDirColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetDirColor(color);
}


#endif // DIRECTORY_VIEW_H
