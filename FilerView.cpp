/*
 * Copyright 2012 @marbocub <marbocub @ google mail>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "FilerView.h"
#include "BafxMessage.h"

#include <SupportDefs.h>
#include <Messenger.h>
#include <String.h>

#include <View.h>
#include <ListView.h>
#include <ListItem.h>
#include <ScrollView.h>
#include <StringView.h>

#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <NodeMonitor.h>

#include <stdio.h>  // for sprintf()

//#define FILER_VIEW_DEBUG


/****************************************************
 * class for local use
 ****************************************************/
class FilerListView;
class FilerListItem;

/* */
class FilerListView : public BListView
{
	int32		m_last_selected;
	FilerView*	mFilerView;

public:
	FilerListView(BRect frame, const char* name, FilerView* parent,
		list_view_type type = B_SINGLE_SELECTION_LIST,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);

	virtual void MakeFocus(bool focused = true);
	virtual void KeyDown(const char* bytes, int32 numBytes);
	virtual void SelectionChanged(void);
	virtual void MessageReceived(BMessage* message);

	void CursorPageUp(void);
	void CursorPageDown(void);
	void CursorUp(void);
	void CursorDown(void);
	void CursorHome(void);
	void CursorEnd(void);
	void ScrollToSelection3(void);

	rgb_color GetBackgroundColor(void) const;
	rgb_color GetSelectedColor(void) const;
	rgb_color GetHiddenColor (void) const;
	rgb_color GetCursorColor(void) const;
	rgb_color GetFileColor(void) const;
	rgb_color GetDirColor(void) const;

	void DoSort(void);
};



/****************************************************
 * class FilerListItem
 ****************************************************/
class FilerListItem : public BListItem
{
	// common data
	bool		m_checked;
	BRect		m_border;
	bool		m_parent;

	// real mode
	entry_ref	m_ref;
	entry_ref	m_symlink_ref;
	struct stat	m_st;
	struct stat	m_symlink_st;
	node_ref	m_nref;
	node_ref	m_symlink_nref;

	// virtual mode
	bool		m_virtual_mode;
	BString		m_virtual_name;
	off_t		m_virtual_size;
	time_t		m_virtual_mtime;
	bool		m_virtual_type_dir;
	bool		m_virtual_type_file;

public:
	FilerListItem(entry_ref ref) : BListItem()
	{
		// common data
		m_checked = false;
		m_parent  = false;
		m_border.top  = m_border.bottom = 0;
		m_border.left = m_border.right  = 3;

		// real mode data
		m_ref = ref;
		BEntry entry(&ref);
		entry.GetStat(&m_st);
		entry.GetNodeRef(&m_nref);
		if (entry.IsSymLink()) {
			BEntry symentry(&ref, true);
			symentry.GetRef(&m_symlink_ref);
			symentry.GetStat(&m_symlink_st);
			symentry.GetNodeRef(&m_symlink_nref);
		}

		// virtual mode data
		m_virtual_mode = false;
	}

	void SetParentFlag(bool flag) { m_parent = flag; }
	void SetCheck(bool check) { if (!IsParent()) m_checked = check; }
	bool IsChecked(void) const { return m_checked; }

	entry_ref GetEntryRef(void) const { return m_ref; }
	node_ref  GetNodeRef(void) const { return m_nref; }
	void OverWriteRef(entry_ref ref) { m_ref = ref; }
	struct stat GetStat(void) const { return m_st; }
	const char* GetName(void) { return m_ref.name; }
	off_t GetSize(void) { return m_st.st_size; }

	bool IsParent(void) { return m_parent; };
	bool IsFile(void) { return S_ISREG(m_st.st_mode); }
	bool IsDirectory(void) { return S_ISDIR(m_st.st_mode); }
	bool IsSymLink(void) { return S_ISLNK(m_st.st_mode); }
	bool IsSymLinkedDirectory(void) {
		if (IsSymLink())
			return S_ISDIR(m_symlink_st.st_mode);
		else
			return false;
	}

	void DrawItem(BView* owner, BRect frame, bool complete = true)
	{
		FilerListView* list_view = (FilerListView*)owner;

		/* draw background */
		if (IsChecked()) {
			owner->SetHighColor(list_view->GetSelectedColor());
			owner->SetLowColor (list_view->GetSelectedColor());
		} else {
			owner->SetHighColor(list_view->GetBackgroundColor());
			owner->SetLowColor (list_view->GetBackgroundColor());
		}
		owner->FillRect(frame);

		/* draw cursor */
		if (IsSelected() && owner->IsFocus()) {
			BRect rect = BRect(frame);
			rect.top = rect.bottom - 1;
			owner->SetHighColor(list_view->GetCursorColor());
			owner->FillRect(rect);
		}

		/* set string color */
		rgb_color color;
		if (IsFile()) {
			color = list_view->GetFileColor();
		} else if (IsDirectory() || IsSymLinkedDirectory()) {
			color = list_view->GetDirColor();
		} else {
			color = list_view->GetFileColor();
		}
		owner->SetHighColor(color);

		/* string font */
		BFont font;
		owner->GetFont(&font);
		font_height height;
		font.GetHeight(&height);
		if (owner->Flags() & B_SUBPIXEL_PRECISE) {
			m_border.bottom = height.descent;
		} else {
			m_border.bottom = (int)height.descent;
		}
		BPoint pos;
		pos.y = frame.bottom - m_border.bottom;
		pos.x = frame.right  - m_border.right;

		/* minimum column width */
		int capacity = (int)(frame.Width() / font.StringWidth(" "));
		int name_min = 12;
		int size_min = 6;
		int date_len = 8;
		int time_len = 8;

		// draw date
		if (capacity >= name_min + 1 + size_min + 1 + date_len) {
			char buf[80];
			struct tm ts;
			localtime_r(&m_st.st_mtime, &ts);
			if (capacity >= name_min + size_min + 1 + date_len + 1 + time_len) {
				pos.x -= font.StringWidth("00-00-00 00:00:00");
				strftime(buf, 80, "%y-%m-%d %T", &ts);
			} else {
				pos.x -= font.StringWidth("00-00-00");
				strftime(buf, 80, "%y-%m-%d", &ts);
			}
			owner->MovePenTo(pos);
			owner->DrawString(buf);
			pos.x -= font.StringWidth(" ");
		}

		// draw size
		if (capacity >= name_min + 1 + size_min) {
			BString sizestr;
			if (IsFile()) {
				sizestr << m_st.st_size;
			} else if (IsDirectory()) {
				sizestr.SetTo("<DIR>");
			} else if (IsSymLink()) {
				sizestr.SetTo("<LNK>");
			} else {
				sizestr.SetTo("<DEV>");
			}
			pos.x -= font.StringWidth(sizestr);
			owner->MovePenTo(pos);
			owner->DrawString(sizestr);
			pos.x -= font.StringWidth(" ");
		}

		// draw name
		BString namestr;
		if (m_parent)
			namestr = "..";
		else
			namestr = m_ref.name;
		if (IsSymLink()) {
			BPath path(&m_symlink_ref);
			namestr << " -> " << path.Path();
		}
		BRect namerect(frame);
		namerect.left += m_border.left;
		namerect.right = pos.x;
		int32 len = (int)(namerect.Width() / font.StringWidth(" "));
		if (len < namestr.Length()) {
			int start = len - 3;
			if (start < 6) start = 6;
			namestr.Remove(start, namestr.Length());
			namestr.Append("···");
		}
		owner->MovePenTo(frame.left + m_border.left, pos.y);
		owner->DrawString(namestr);
	}
};



/********************************************************
 * class FilerListView
 ********************************************************/
int
NameCompareFuncByItem(BListItem* bitem1, BListItem* bitem2)
{
	FilerListItem* item1 = (FilerListItem*) bitem1;
	FilerListItem* item2 = (FilerListItem*) bitem2;

	if (item1->IsParent())
		return -1;
	if (item2->IsParent())
		return 1;

	bool isdir1 = item1->IsDirectory();
	bool isdir2 = item2->IsDirectory();
	if (!isdir1) isdir1 = item1->IsSymLinkedDirectory();
	if (!isdir2) isdir2 = item2->IsSymLinkedDirectory();
	if (isdir1 && !isdir2)
		return -1;
	else if (isdir2 && !isdir1)
		return 1;

	BString name1 = item1->GetName();
	BString name2 = item2->GetName();
	return name1.Compare(name2);
}

int
NameCompareFunc(const void* firstArg, const void* secondArg)
{
	return NameCompareFuncByItem(*((BListItem**)firstArg), *((BListItem**)secondArg));
}

FilerListView::FilerListView(BRect frame, const char* name, FilerView* parent,
		list_view_type type, uint32 resizingMode, uint32 flags)
		: BListView(frame, name, type, resizingMode, flags)
{
	mFilerView = parent;
	m_last_selected = -1;
}

void
FilerListView::CursorPageUp(void)
{
	if (CountItems() > 0) {
		BRect itemFrame = ItemFrame(CurrentSelection(0));
		int32 index = IndexOf(BPoint(Bounds().left, Bounds().top+itemFrame.Height()));

		if (index <= 1) {
			index = 0;
		} else if (index >= CurrentSelection()) {
			ScrollTo(Bounds().left, Bounds().top-Bounds().Height()+itemFrame.Height()*3);
			index = IndexOf(BPoint(Bounds().left, Bounds().top+itemFrame.Height()));
			if (index <= 1) {
				index = 0;
			}
		}
		Select(index);
		itemFrame = ItemFrame(CurrentSelection(0));
		ScrollTo(itemFrame.left, itemFrame.top-itemFrame.Height());
	}
}

void
FilerListView::CursorPageDown(void)
{
	if (CountItems() > 0) {
		BRect itemFrame = ItemFrame(CurrentSelection(0));
		int32 index = IndexOf(BPoint(Bounds().left, Bounds().bottom-itemFrame.Height()));
		if (index >= CountItems()-2 || index < 0) {
			index = CountItems()-1;
		} else if (index <= CurrentSelection()) {
			ScrollTo(Bounds().left, Bounds().top+Bounds().Height()-itemFrame.Height()*3);
			index = IndexOf(BPoint(Bounds().left, Bounds().bottom-itemFrame.Height()));
			if (index <= 1) {
				index = 0;
			}
		}
		Select(index);
		itemFrame = ItemFrame(CurrentSelection(0));
		ScrollTo(itemFrame.left, itemFrame.bottom-Bounds().Height()+itemFrame.Height());
	}
}

void
FilerListView::CursorUp(void)
{
	if (CountItems() > 0) {
		int32 index = CurrentSelection();
		if (index > 0) {
			index--;
		}
		Select(index);
		ScrollToSelection();
	}
}

void
FilerListView::CursorDown(void)
{
	if (CountItems() > 0) {
		int32 index = CurrentSelection();
		if (index < CountItems()-1) {
			index++;
		}
		Select(index);
		ScrollToSelection();
	}
}

void
FilerListView::CursorHome(void)
{
	if (CountItems() > 0) {
		Select(0);
		ScrollToSelection();
	}
}

void
FilerListView::CursorEnd(void)
{
	if (CountItems() > 0) {
		Select(CountItems()-1);
		ScrollToSelection();
	}
}

void
FilerListView::ScrollToSelection3(void)
{
	BRect itemFrame = ItemFrame(CurrentSelection(0));

	if (Bounds().Contains(itemFrame))
		return;

	ScrollTo(itemFrame.left, itemFrame.top-itemFrame.Height()*2-2);
}

void
FilerListView::DoSort(void)
{
	SortItems(&NameCompareFunc);
}

void
FilerListView::MakeFocus(bool focused)
{
	InvalidateItem(m_last_selected);
	BListView::MakeFocus(focused);
}

void
FilerListView::SelectionChanged(void)
{
	int32 current = CurrentSelection();

	if (current >= 0)
		m_last_selected = current;
	else
		Select(m_last_selected);

	BListView::SelectionChanged();
}

void
FilerListView::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1) {
		int32 index;
		BRect itemFrame = ItemFrame(CurrentSelection(0));

		switch (bytes[0]) {
		case B_TAB:
			BListView::KeyDown(bytes, numBytes);
			break;
		default:
			mFilerView->KeyDown(bytes, numBytes);
			break;
		}
	} else {
			mFilerView->KeyDown(bytes, numBytes);
	}
}

void
FilerListView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	default:
		BListView::MessageReceived(message);
		break;
	}
}

rgb_color FilerListView::GetBackgroundColor(void) const
{
	return mFilerView->GetBackgroundColor();
}

rgb_color
FilerListView::GetSelectedColor(void) const
{
	return mFilerView->GetSelectedColor();
}

rgb_color
FilerListView::GetHiddenColor(void) const
{
	return mFilerView->GetHiddenColor();
}

rgb_color
FilerListView::GetCursorColor(void) const
{
	return mFilerView->GetCursorColor();
}

rgb_color
FilerListView::GetFileColor(void) const
{
	return mFilerView->GetFileColor();
}

rgb_color
FilerListView::GetDirColor(void) const
{
	return mFilerView->GetDirColor();
}


/****************************************************
 * class FilerView
 ****************************************************/
FilerView::FilerView(BRect frame, const char* name)
	: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BString	sub_name;
	BRect	view_rect = Bounds();
	BRect	ctrl_rect;

	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

	// directory name label
	ctrl_rect = view_rect;
	ctrl_rect.bottom = ctrl_rect.top + B_H_SCROLL_BAR_HEIGHT;
 	sub_name.SetTo(name);
	sub_name.Append("_dir_name");
	mDirNameView = new BStringView(
		ctrl_rect,
		sub_name,
		"",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
		B_WILL_DRAW
	);
	AddChild(mDirNameView);
	view_rect.top = ctrl_rect.bottom + 1;

	// mark stat label
	ctrl_rect = view_rect;
	ctrl_rect.top = ctrl_rect.bottom - B_H_SCROLL_BAR_HEIGHT;
	sub_name.SetTo(name);
	sub_name.Append("_mark_status");
	mMarkStatusView = new BStringView(
		ctrl_rect,
		sub_name,
		"",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
		B_WILL_DRAW
	);
	AddChild(mMarkStatusView);
	view_rect.bottom = ctrl_rect.top - 1;

	// directory stat label
	ctrl_rect = view_rect;
	ctrl_rect.top = ctrl_rect.bottom - B_H_SCROLL_BAR_HEIGHT;
	sub_name.SetTo(name);
	sub_name.Append("_dir_status");
	mDirStatusView = new BStringView(
		ctrl_rect,
		sub_name,
		"",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
		B_WILL_DRAW
	);
	AddChild(mDirStatusView);
	view_rect.bottom = ctrl_rect.top - 1;

	// directory view
	ctrl_rect = view_rect;
	ctrl_rect.right -= (B_V_SCROLL_BAR_WIDTH + 1);
	ctrl_rect.left  += 1;
	sub_name.SetTo(name);
	sub_name.Append("_list_view");
	mListView = new FilerListView(
		ctrl_rect,
		sub_name,
		this,
		B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE
	);
	sub_name.SetTo(name);
	sub_name.Append("_scroll_view");
	mScrollView = new BScrollView(
		sub_name,
		mListView,
		B_FOLLOW_ALL_SIDES,
		0,
		false, true,
		B_PLAIN_BORDER
	);
	AddChild(mScrollView);

	// font
	mListView ->SetFont(be_fixed_font);
	mDirNameView->SetFont(be_fixed_font);
	mDirStatusView->SetFont(be_fixed_font);
	mMarkStatusView->SetFont(be_fixed_font);

	// color
	rgb_color viewcolor  = ViewColor();
	rgb_color highcolor = HighColor();
	rgb_color lowcolor = LowColor();
	SetBackgroundColor(lowcolor);
	SetSelectedColor(viewcolor);
	SetHiddenColor (highcolor);
	SetCursorColor(highcolor);
	SetFileColor(highcolor);
	SetDirColor(highcolor);

	// keymap function
	keymap_func = NULL;
}

FilerView::~FilerView()
{
	CloseDirectory();

	mScrollView->RemoveChild(mListView);
	delete mListView;

	RemoveChild(mScrollView);
	delete mScrollView;

	RemoveChild(mDirStatusView);
	delete mDirStatusView;

	RemoveChild(mMarkStatusView);
	delete mMarkStatusView;

	RemoveChild(mDirNameView);
	delete mDirNameView;
}

void
FilerView::MakeFocus(bool focused)
{
	mListView->MakeFocus(focused);
}

status_t
FilerView::OpenDirectory(BDirectory *dir)
{
	status_t retval = dir->InitCheck();
	entry_ref old_ref = m_ref;

	CloseDirectory();

	if (retval == B_OK) {
		BEntry entry;
		BPath  path;
		entry_ref ref;

		// directory path
		dir->GetEntry(&entry);
		entry.GetRef(&m_ref);
		entry.GetPath(&path);
		entry.GetNodeRef(&m_nref);
		mDirNameView->SetText(path.Path());
		mDirNameView->Invalidate();
		path.Unset();

		// parent directory
		if (entry.GetParent(&entry) == B_OK) {
			entry.GetRef(&ref);
			FilerListItem* item = new FilerListItem(ref);
			mListView->AddItem(item);
			item->SetParentFlag(true);
		}

		// read directory entry
		while(dir->GetNextRef(&ref) == B_OK) {
			mListView->AddItem(new FilerListItem(ref));
		}

		// update status line
		StatusUpdate();

		// sort
		((FilerListView*)mListView)->DoSort();

		// move selection
		mListView->Select(0);
		for (int32 i=mListView->CountItems()-1; i>=0; i--) {
			if (old_ref == ((FilerListItem*)(mListView->ItemAt(i)))->GetEntryRef()) {
				mListView->Select(i);
				((FilerListView*)mListView)->ScrollToSelection3();
				break;
			}
		}

		// start NodeWatcher
		watch_node (&m_nref, B_WATCH_ALL, this);

	} else {
		mDirStatusView->SetText("Cannot open directory");
	}

	return retval;
}

status_t
FilerView::OpenDirectory(const entry_ref *ref)
{
	BDirectory* dir = new BDirectory(ref);
	status_t ret = OpenDirectory(dir);
	delete dir;

	return ret;
}

status_t
FilerView::OpenDirectory(const char *dirname)
{
	BDirectory* dir = new BDirectory(dirname);
	status_t ret = OpenDirectory(dir);
	delete dir;

	return ret;
}

void
FilerView::CloseDirectory(void)
{
	BListItem* item;

	// stop NodeWatcher
	stop_watching(this);

	// DeselectAllItems
	if (mListView->CountItems() > 0) {
		mListView->Select(mListView->CountItems()-1);
		mListView->Deselect(mListView->CountItems()-1);
	}
	mListView->DeselectAll();

	// RemoveAllItems
	for (int32 i=mListView->CountItems()-1; i>=0; i--) {
		if ((item = mListView->RemoveItem(i)) != NULL) {
			item->Deselect();
			delete item;
		}
	}

	mDirNameView->SetText("");
	mDirStatusView->SetText("");
	mMarkStatusView->SetText("");
}

void
FilerView::ToggleMarkFile(int32 index)
{
	FilerListItem* item;
	item = (FilerListItem*) mListView->ItemAt(index);
	if (item != NULL) {
		item->SetCheck(! item->IsChecked());
	}
	mListView->InvalidateItem(index);
	StatusUpdate();
}

bool
FilerView::EnterCurrentFile(void)
{
	FilerListItem* item;
	item = (FilerListItem*) mListView->ItemAt(mListView->CurrentSelection());
	if (item != NULL) {
		if (item->IsDirectory()) {
			entry_ref ref = item->GetEntryRef();
			if (OpenDirectory(&ref) == B_OK)
				return true;
			else
				return false;
		} else if (item->IsSymLinkedDirectory()) {
			entry_ref ref = item->GetEntryRef();
			if (OpenDirectory(&ref) == B_OK)
				return true;
			else
				return false;
		}
	}
	return false;
}

void
FilerView::EnterParentDirectory(void)
{
	BEntry entry(&m_ref);
	BDirectory dir;
	if (entry.GetParent(&dir) == B_OK)
		OpenDirectory(&dir);
	entry.Unset();
	dir.Unset();
}

void
FilerView::FormatStatusString(BString* status, int dirs, int files, off_t size)
{
	char buf[80];

	status->SetTo(" ");
	if (dirs > 0) {
		sprintf(buf, "%3d Dirs ", dirs);
		status->Append(buf);
	} else {
		status->Append("         ");
	}
	if (files > 0) {
		sprintf(buf, "%4d Files ", files);
		status->Append(buf);
	} else {
		status->Append("           ");
	}
	if (size > 0) {
		if (size > 1024*1024*1024) {
			sprintf(buf, "%6.2lf GBytes", size/1024.0/1024.0/1024.0);
		} else if (size > 1024*1024) {
			sprintf(buf, "%6.2lf MBytes", size/1024.0/1024.0);
		} else if (size > 1024) {
			sprintf(buf, "%6.2lf KBytes", size/1024.0);
		} else {
			sprintf(buf, "%6lld Bytes ", size);
		}
		status->Append(buf);
	} else {
		status->Append("             ");
	}
}

void
FilerView::StatusUpdate(void)
{
	int32 dirs = 0, files = 0, mdirs = 0, mfiles = 0;
	off_t size = 0, msize = 0; 

	int32 cnt = mListView->CountItems();
	for (int32 i=0; i<cnt; i++) {
		FilerListItem* item = (FilerListItem*)mListView->ItemAt(i);
		if (item->IsParent())
			continue;

		if (item->IsDirectory() || item->IsSymLinkedDirectory()) {
			dirs++;
			if (item->IsChecked())
				mdirs++;
		} else {
			files++;
			size += item->GetSize();
			if (item->IsChecked()) {
				mfiles++;
				msize += item->GetSize();
			}
		}
	}

	BString status;
	FormatStatusString(&status, dirs,  files,  size);
	mDirStatusView->SetText(status);
	mDirStatusView->Invalidate();

	if (mdirs > 0 || mfiles > 0) {
		FormatStatusString(&status, mdirs, mfiles, msize);
		status << " Marked";
	} else {
		status.SetTo("");
	}
	mMarkStatusView->SetText(status);
	mMarkStatusView->Invalidate();
}

void
FilerView::SetBackgroundColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	mListView->SetViewColor(red, green, blue, alpha);
}

void
FilerView::SetSelectedColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	m_color_selected.red   = red;
	m_color_selected.green = green;
	m_color_selected.blue  = blue;
	m_color_selected.alpha = alpha;
}

void
FilerView::SetHiddenColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	m_color_hidden.red   = red;
	m_color_hidden.green = green;
	m_color_hidden.blue  = blue;
	m_color_hidden.alpha = alpha;
}

void
FilerView::SetCursorColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	m_color_cursor.red   = red;
	m_color_cursor.green = green;
	m_color_cursor.blue  = blue;
	m_color_cursor.alpha = alpha;
}

void
FilerView::SetFileColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	m_color_file.red   = red;
	m_color_file.green = green;
	m_color_file.blue  = blue;
	m_color_file.alpha = alpha;
}

void
FilerView::SetDirColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	m_color_dir.red   = red;
	m_color_dir.green = green;
	m_color_dir.blue  = blue;
	m_color_dir.alpha = alpha;
}

void FilerView::SetBackgroundColor(rgb_color color) { mListView->SetViewColor(color); }
void FilerView::SetSelectedColor(rgb_color color) { m_color_selected = color; }
void FilerView::SetHiddenColor (rgb_color color) { m_color_hidden = color; }
void FilerView::SetCursorColor(rgb_color color) { m_color_cursor = color; }
void FilerView::SetFileColor(rgb_color color) { m_color_file = color; }
void FilerView::SetDirColor(rgb_color color) { m_color_dir = color; }
rgb_color FilerView::GetBackgroundColor(void) const { return mListView->ViewColor(); }
rgb_color FilerView::GetSelectedColor(void) const { return m_color_selected; }
rgb_color FilerView::GetHiddenColor(void) const { return m_color_hidden;   }
rgb_color FilerView::GetCursorColor(void) const { return m_color_cursor; }
rgb_color FilerView::GetFileColor(void) const { return m_color_file; }
rgb_color FilerView::GetDirColor(void) const { return m_color_dir; }

void
FilerView::SetKeymapFunction(uint32 (*keyfunc)(char key, int32 modifiers))
{
	keymap_func = keyfunc;
}

void
FilerView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 cmd;

	if (numBytes == 1) {
		switch (bytes[0]) {
		default:
			if (keymap_func != NULL) {
				if ((cmd = (*keymap_func)(bytes[0], modifiers())) != 0) {
					BMessage msg(cmd);
					msg.AddPointer("sender", this);
					BMessenger(this).SendMessage(&msg);
				}
			}
			break;
		}
	}
}

void
FilerView::NodeWatcherAddItem(entry_ref ref)
{
	BListItem* item = new FilerListItem(ref);
	int32 cnt = mListView->CountItems();
	int32 i;
	for (i=0; i<cnt; i++) {
		BListItem* litem = mListView->ItemAt(i);
		if (NameCompareFuncByItem(item, litem) < 0)
			break;
	}
	mListView->AddItem(item, i);
	StatusUpdate();
}

void
FilerView::NodeWatcherRemoveItem(node_ref nref)
{
	if (m_nref == nref) {
		OpenDirectory("/");
	} else {
		int32 cnt = mListView->CountItems();
		for (int32 i=0; i<cnt; i++) {
			BListItem* litem = mListView->ItemAt(i);
			if (nref == ((FilerListItem*)litem)->GetNodeRef()) {
				mListView->RemoveItem(litem);
				delete litem;
				break;
			}
		}
		StatusUpdate();
	}
}

void
FilerView::NodeWatcherRenameItem(node_ref nref, entry_ref ref)
{
	int32 cnt = mListView->CountItems();
	for (int32 i=0; i<cnt; i++) {
		BListItem* item = mListView->ItemAt(i);
		if (nref == ((FilerListItem*)item)->GetNodeRef()) {
			((FilerListItem*)item)->OverWriteRef(ref);
			mListView->InvalidateItem(i);
			break;
		}
	}
	StatusUpdate();
}

void
FilerView::NodeWatcherHandler(BMessage *msg)
{
	int32 opcode;
	if (msg->FindInt32("opcode", &opcode) != B_OK)
		return;

	dev_t device;
	ino_t directory;
	ino_t node;
	const char *name;
	entry_ref ref;
	node_ref nref;
	node_ref nref_from;
	node_ref nref_to;

	switch (opcode) {
	case B_ENTRY_CREATED:
		msg->FindInt32("device", &ref.device);
		msg->FindInt64("directory", &ref.directory);
		msg->FindString("name", &name);
		ref.set_name(name);
		NodeWatcherAddItem(ref);
		break;

	case B_ENTRY_REMOVED:
		msg->FindInt32("device", &nref.device);
		msg->FindInt64("node", &nref.node);
		NodeWatcherRemoveItem(nref);
		break;

	case B_ENTRY_MOVED:
		// file node_ref
		msg->FindInt32("device", &nref.device);
		msg->FindInt64("node", &nref.node);

		// file entry_ref
		ref.device = nref.device;
		msg->FindString("name", &name);

		// directory node_ref(s)
		nref_from.device = nref.device;
		nref_to.device = nref.device;
		msg->FindInt64("from directory", &nref_from.node);
		msg->FindInt64("to directory", &nref_to.node);

		// update list view
		if (nref_from == nref_to) {
			ref.directory = nref.node;
			ref.set_name(name);
			NodeWatcherRenameItem(nref, ref);
		} else if (nref_from == m_nref) {
			ref.directory = nref_from.node;
			ref.set_name(name);
			NodeWatcherRemoveItem(nref);
		} else if (nref_to == m_nref) {
			ref.directory = nref_to.node;
			ref.set_name(name);
			NodeWatcherAddItem(ref);
		}

		break;

	default:
#ifdef FILER_VIEW_DEBUG
		printf("NodeWatcher:opcode:%d\n", opcode);
#endif
		break;
	}
}

void
FilerView::MessageReceived(BMessage *message)
{
#ifdef FILER_VIEW_DEBUG
	printf("FilerView:message:%c%c%c%c\n",
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what);
	printf("CountNames: %d\n", message->CountNames(B_ANY_TYPE));
#endif

	switch(message->what) {

	/*
	 * node watcher
	 */
	case B_NODE_MONITOR:
		NodeWatcherHandler(message);
		break;

	/*
	 * cursor move
	 */
	case M_CURSOR_UP:
		((FilerListView*)mListView)->CursorUp();
		break;
	case M_CURSOR_DOWN:
		((FilerListView*)mListView)->CursorDown();
		break;
	case M_CURSOR_PAGEUP:
		((FilerListView*)mListView)->CursorPageUp();
		break;
	case M_CURSOR_PAGEDOWN:
		((FilerListView*)mListView)->CursorPageDown();
		break;
	case M_CURSOR_HOME:
		((FilerListView*)mListView)->CursorHome();
		break;
	case M_CURSOR_END:
		((FilerListView*)mListView)->CursorEnd();
		break;

	/*
	 * filer reqest
	 */
	case M_ENTER_FILE:
		if (! EnterCurrentFile()) {
			BMessage msg(M_OPEN_FILE);
			msg.AddPointer("sender", this);
			BMessenger(this).SendMessage(&msg);
		}
		break;

	case M_PARENT_DIRECTORY:
		EnterParentDirectory();
		break;

	case M_MARK_FILE:
		ToggleMarkFile(mListView->CurrentSelection());
		((FilerListView*)mListView)->CursorDown();
		break;

	case M_MARK_ALL_FILE:
		{
			int32 cnt = mListView->CountItems();
			for (int32 i=0; i<cnt; i++) {
				FilerListItem* item = (FilerListItem*) mListView->ItemAt(i);
				if (!item->IsDirectory() && !item->IsSymLinkedDirectory()) {
					ToggleMarkFile(i);
				}
			}
		}
		break;

	default:
		BView::MessageReceived(message);
		break;
	}
}

int32
FilerView::CountMarkedItems(void)
{
	int32 len = mListView->CountItems();
	int32 cnt = 0;
	for (int32 i=0; i<len; i++) {
		FilerListItem* item = (FilerListItem*)mListView->ItemAt(i);
		if (item->IsChecked()) {
			cnt++;
		}
	}
	return cnt;
}

BList*
FilerView::GetMarkedList(void)
{
	BList* list = new BList(CountMarkedItems());
	int32 len = mListView->CountItems();
	int32 j=0;
	for (int32 i=0; i<len; i++) {
		FilerListItem* item = (FilerListItem*)mListView->ItemAt(i);
		if (item->IsChecked()) {
			entry_ref* ref = new entry_ref(item->GetEntryRef());
			list->AddItem((void*)ref);
		}
	}
	return list;
}

void
FilerView::CloseMarkedList(BList* list)
{
	int32 cnt = list->CountItems();
	for (int32 i=0; i<cnt; i++) {
		entry_ref* ref = (entry_ref*)(list->RemoveItem(i));
		delete ref;
	}
	list->MakeEmpty();
	delete list;
}

entry_ref
FilerView::GetDirectoryRef(void) const
{
	return m_ref;
}

entry_ref
FilerView::GetCurrentSelectionRef(void) const
{
	return ((FilerListItem*)(mListView->ItemAt(mListView->CurrentSelection())))->GetEntryRef();
}

