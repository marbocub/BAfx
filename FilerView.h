/*
 * Copyright 2012 @marbocub <marbocub @ google mail>
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

	status_t OpenDirectory(const char *dirname);
	status_t OpenDirectory(const entry_ref *ref);

	entry_ref GetDirectoryRef(void) const;
	entry_ref GetCurrentSelectionRef(void) const;

	int32 CountMarkedItems(void);
	BList* GetMarkedList(void);
	void CloseMarkedList(BList* list);

	void SetKeymapFunction(uint32 (*keyfunc)(char key, int32 modifiers));

	void SetBackgroundColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetSelectedColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetHiddenColor (uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetCursorColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetFileColor(uchar red, uchar green, uchar blue, uchar alpha=0);
	void SetDirColor(uchar red, uchar green, uchar blue, uchar alpha=0);

	void SetBackgroundColor(rgb_color color);
	void SetSelectedColor(rgb_color color);
	void SetHiddenColor (rgb_color color);
	void SetCursorColor(rgb_color color);
	void SetFileColor(rgb_color color);
	void SetDirColor(rgb_color color);

	rgb_color GetBackgroundColor(void) const;
	rgb_color GetSelectedColor(void) const;
	rgb_color GetHiddenColor (void) const;
	rgb_color GetCursorColor(void) const;
	rgb_color GetFileColor(void) const;
	rgb_color GetDirColor(void) const;

private:
	BListView*		mListView;
	BScrollView*	mScrollView;
	BStringView*	mDirNameView;
	BStringView*	mDirStatusView;
	BStringView*	mMarkStatusView;

	rgb_color		m_color_selected;
	rgb_color		m_color_hidden ;
	rgb_color		m_color_cursor;
	rgb_color		m_color_file;
	rgb_color		m_color_dir;

	entry_ref		m_ref;
	node_ref		m_nref;

	uint32 (*keymap_func)(char key, int32 modifiers);

	status_t OpenDirectory(BDirectory *dir);
	void CloseDirectory(void);

	void AddEntryItem(entry_ref *ref);
	void RemoveEntryItem(entry_ref *ref);

	void FormatStatusString(BString* status, int dirs, int files, off_t size);
	void StatusUpdate(void);

	void NodeWatcherHandler(BMessage *msg);
	void NodeWatcherAddItem(entry_ref ref);
	void NodeWatcherRemoveItem(node_ref nref);
	void NodeWatcherRenameItem(node_ref nref, entry_ref ref);

	void ToggleMarkFile(int32 index);
	bool EnterCurrentFile(void);
	void EnterParentDirectory(void);
};

#endif // FILER_VIEW_H
