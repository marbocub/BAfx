/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef ENTRY_LIST_VIEW_H
#define ENTRY_LIST_VIEW_H

#include <SupportDefs.h>
#include <ListView.h>
#include <ListItem.h>

#include "EntryModel.h"


class EntryListView : public BListView
{
private:
	int32		last_selected_;
	rgb_color	color_selected_;
	rgb_color	color_cursor_;
	rgb_color	color_file_;
	rgb_color	color_dir_;
	rgb_color	color_executable_;
	rgb_color	color_readonly_;
	rgb_color	color_hidden_;

private:
	uint32 (*keymap_func)(char key, int32 modifiers);

public:
	EntryListView(BRect frame, const char* name,
		list_view_type type, uint32 resizingMode, uint32 flags);
	~EntryListView();

	void DoSort();
	void AddEntryList(BList* list);
	void DeleteAllItems();

	void MakeFocus(bool focused);
	void FrameResized(float width, float height);
	void ScrollToSelection();
	void ScrollToSelection3();
	void SelectionChanged();
	void Select(int32 i);
	void SelectAt(const Entry& entry);
	void KeyDown(const char* bytes, int32 numBytes);

	void CursorUp();
	void CursorDown();
	void CursorPageUp();
	void CursorPageDown();
	void CursorHome();
	void CursorEnd();
	bool ToggleMark(int32 index);

	bool IsParent();
	bool IsDirectory();
	Entry* GetEntry();

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
};


class EntryListItem : public BListItem
{
public:
	enum EntryItemType {
		ENTRY_IS_PARENT = -1,
		ENTRY_IS_CURRENT = 0,
		ENTRY_IS_CHILDLEN = 1,
	};

private:
	BRect		border_;
	Entry*		const entry_;
	bool		marked_;
	EntryItemType	type_;

public:
	EntryListItem(Entry* entry, EntryItemType type = ENTRY_IS_CHILDLEN);
	~EntryListItem();
	void SetMarked(bool mark);
	bool IsMarked();
	bool IsParent();
	bool IsDirectory();
	Entry* GetEntry();
	status_t GetName(char* name);
	off_t GetSize();

private:
	void DrawBackground(EntryListView* list_view, BRect frame, bool complete);
	void DrawCursor(EntryListView* list_view, BRect frame);
	void DrawSetPenColor(EntryListView* list_view, BRect frame);
	void DrawSetFont(EntryListView* list_view, BRect frame, BFont &font);
	void DrawFileTime(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos, bool with_time);
	void DrawFileSize(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos);
	void DrawFileName(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos);
	void DrawItem(BView* owner, BRect frame, bool complete = true);
};


#endif // ENTRY_LIST_VIEW_H
