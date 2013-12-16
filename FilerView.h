/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FILER_VIEW_H
#define FILER_VIEW_H

#include <SupportDefs.h>
#include <View.h>
#include <StringView.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Directory.h>
#include <Entry.h>
#include <List.h>

class FilerView : public BView
{
public:
	FilerView(BRect frame, const char* name);
	~FilerView();

	virtual void MakeFocus(bool focused = true);
	virtual void KeyDown(const char* bytes, int32 numBytes);
	virtual void MessageReceived(BMessage *message);

	status_t OpenDirectory(const char* dirname);
	void GetDirectory(BDirectory* dir);
	int32 GetMarkedEntryList(BList* list);

	void UnMark(BEntry* entry);

	void SetKeymapFunction(uint32 (*keyfunc)(char key, int32 modifiers));

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
	BListView*		mListView;
	BScrollView*	mScrollView;
	BStringView*	mDirNameView;
	BView*			mCountView;

	uint32 (*keymap_func)(char key, int32 modifiers);

	status_t OpenDirectory(BDirectory* dir);
	status_t OpenDirectory(const BEntry* entry);

	void StatusUpdate(void);
};

#endif // FILER_VIEW_H
