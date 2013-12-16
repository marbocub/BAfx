/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "FilerView.h"
#include "BafxMessage.h"

#include <SupportDefs.h>
/* ApplicationKit */
#include <Messenger.h>
/* SupportKit */
#include <String.h>
/* InterfaceKit */
#include <View.h>
#include <ListView.h>
#include <ListItem.h>
#include <ScrollView.h>
#include <StringView.h>
/* StorageKit */
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <NodeMonitor.h>
#include <SymLink.h>

#include <cstdio>	// for sprintf

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/****************************************************
 * class FilerCountView
 ****************************************************/
class FilerCountView : public BView
{
private:
	int32			totalFiles_;
	int32			totalDirs_;
	off_t			totalBytes_;
	int32			markedFiles_;
	int32			markedDirs_;
	off_t			markedBytes_;

	BStringView*	mTotalCount_;
	BStringView*	mMarkedCount_;

	void FormatCountString(BString* status, int32 files, int32 dirs, off_t size)
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

	void PrintAll()
	{
		BString count;

		FormatCountString(&count, totalFiles_, totalDirs_, totalBytes_);
		mTotalCount_->SetText(count);

		if (markedDirs_ > 0 || markedFiles_ > 0) {
			FormatCountString(&count, markedFiles_, markedDirs_, markedBytes_);
			count << " Marked";
		} else {
			count.SetTo("");
		}
		mMarkedCount_->SetText(count);

		Invalidate();
	}

public:
	FilerCountView(
			BRect frame, const char* name,
			uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW)
		: BView(frame, name, resizingMode, flags)
	{
		BRect ctrl_rect;
		BString ctrl_name;

		BView::SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

		ctrl_rect = Bounds();
		ctrl_rect.bottom = ctrl_rect.top + B_H_SCROLL_BAR_HEIGHT - 1;
		ctrl_name.SetTo(name);
		ctrl_name.Append("_total_count");
		mTotalCount_  = new BStringView(
			ctrl_rect, ctrl_name, "",
			B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
		AddChild(mTotalCount_);

		ctrl_rect = Bounds();
		ctrl_rect.top = ctrl_rect.top + B_H_SCROLL_BAR_HEIGHT;
		ctrl_name.SetTo(name);
		ctrl_name.Append("_marked_count");
		mMarkedCount_  = new BStringView(
			ctrl_rect, ctrl_name, "",
			B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
		AddChild(mMarkedCount_);

		totalFiles_  = 0;
		totalDirs_   = 0;
		totalBytes_  = 0;
		markedFiles_ = 0;
		markedDirs_  = 0;
		markedBytes_ = 0;

		SetFont(be_fixed_font);
	}

	~FilerCountView()
	{
		RemoveChild(mTotalCount_);
		delete mTotalCount_;

		RemoveChild(mMarkedCount_);
		delete mMarkedCount_;
	}

	void SetFont(const BFont* font, uint32 properties = B_FONT_ALL)
	{
		BView::SetFont(font);
		mTotalCount_ ->SetFont(font);
		mMarkedCount_->SetFont(font);
	}

	void Invalidate()
	{
		BView::Invalidate();
		mTotalCount_ ->Invalidate();
		mMarkedCount_->Invalidate();
	}

	void SetTotalCount(int32 files, int32 dirs, off_t bytes)
	{
		totalFiles_ = files;
		totalDirs_  = dirs;
		totalBytes_ = bytes;
		PrintAll();
	}

	void SetFiles(int32 files)
	{
		totalFiles_ = files;
		PrintAll();
	}

	void SetDirs(int32 dirs)
	{
		totalDirs_ = dirs;
		PrintAll();
	}

	void SetBytes(off_t bytes)
	{
		totalBytes_ = bytes;
		PrintAll();
	}

	void SetMarkedCount(int32 files, int32 dirs, off_t bytes)
	{
		markedFiles_ = files;
		markedDirs_  = dirs;
		markedBytes_ = bytes;
		PrintAll();
	}

	void SetMarkedFiles(int32 files)
	{
		markedFiles_ = files;
		PrintAll();
	}

	void SetMarkedDirs(int32 dirs)
	{
		markedDirs_ = dirs;
		PrintAll();
	}

	void SetMarkedBytes(off_t bytes)
	{
		markedBytes_ = bytes;
		PrintAll();
	}
};

/****************************************************
 * class FilerListView
 ****************************************************/
class FilerListView : public BListView
{
	/****************************************************
	 * class FilerListItem
	 ****************************************************/
	class FilerListItem : public BListItem
	{
		BEntry		entry_;
		bool		marked_;
		bool		parent_;
		BRect		border_;
		node_ref	nref_;

	public:
		FilerListItem(const BEntry& newEntry) : BListItem()
		{
			entry_  = newEntry;
			marked_ = false;
			parent_ = false;
			entry_.GetNodeRef(&nref_);
			border_.top  = border_.bottom = 0;
			border_.left = border_.right  = 3;
		}

		void GetEntry(BEntry* entry) const
		{
			if (entry != NULL) {
				*entry = entry_;
			}
		}

		BEntry* GetEntry()
		{
			return &entry_;
		}

		void SetEntry(BEntry* entry)
		{
			entry_ = *entry;
			entry_.GetNodeRef(&nref_);
		}

		bool IsThis(const node_ref* nref)
		{
			if (nref_ == *nref)
				return true;
			else
				return false;
		}

		bool IsThis(const BEntry* entry)
		{
			if (entry_ == *entry)
				return true;
			else
				return false;
		}

		void SetMarked(bool mark)
		{
			if (!IsParent())
				marked_ = mark;
		}

		bool IsMarked() const
		{
			return marked_;
		}

		void SetParentDirFlag(bool flag)
		{
			parent_ = flag;
		}

		bool IsParent() const
		{
			return parent_;
		};

		status_t GetSize(off_t* size) const
		{
			return entry_.GetSize(size);
		}

		off_t GetSize() const
		{
			off_t size;
			entry_.GetSize(&size);
			return size;
		}

		status_t GetName(char* name) const
		{
			return entry_.GetName(name);
		}

		bool IsFile() const
		{
			if (entry_.IsFile()) {
				return true;
			} else if (entry_.IsSymLink()) {
				entry_ref ref;
				entry_.GetRef(&ref);
				BEntry linkEntry(&ref, true);
				if (linkEntry.IsFile())
					return true;
			}
			return false;
		}

		bool IsDirectory() const
		{
			if (entry_.IsDirectory()) {
				return true;
			} else if (entry_.IsSymLink()) {
				entry_ref ref;
				entry_.GetRef(&ref);
				BEntry linkEntry(&ref, true);
				if (linkEntry.IsDirectory())
					return true;
			}
			return false;
		}

		bool IsSymLink() const
		{
			return entry_.IsSymLink();
		}

		bool IsHiddenFile() const
		{
			return false;
		}

		bool IsReadonly() const
		{
			return false;
		}

		bool IsExecutable() const
		{
			return false;
		}

		void DrawItem(BView* owner, BRect frame, bool complete = true)
		{
			FilerListView* list_view = (FilerListView*)owner;
			char name[B_FILE_NAME_LENGTH];
			entry_.GetName(name);

			/* draw background */
			if (IsMarked()) {
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
			if (IsHiddenFile()) {
				color = list_view->GetHiddenColor();
			} else if (IsReadonly()) {
				color = list_view->GetReadonlyColor();
			} else if (IsFile()) {
				if (IsExecutable()) {
					color = list_view->GetExecutableColor();
				} else {
					color = list_view->GetFileColor();
				}
			} else if (IsDirectory()) {
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
				border_.bottom = height.descent;
			} else {
				border_.bottom = (int)height.descent;
			}
			BPoint pos;
			pos.y = frame.bottom - border_.bottom;
			pos.x = frame.right  - border_.right;

			/* minimum column width */
			int capacity = (int)(frame.Width() / font.StringWidth(" "));
			int name_min = 12;
			int size_min = 6;
			int date_len = 8;
			int time_len = 8;

			// draw mtime
			if (capacity >= name_min + 1 + size_min + 1 + date_len) {
				time_t mtime;
				struct tm ts;
				char buf[80];
				if (entry_.GetModificationTime(&mtime) == B_OK) {
					localtime_r(&mtime, &ts);

					if (capacity >= name_min + size_min + 1 + date_len + 1 + time_len) {
						pos.x -= font.StringWidth("00-00-00 00:00:00");
						strftime(buf, 80, "%y-%m-%d %T", &ts);
					} else {
						pos.x -= font.StringWidth("00-00-00");
						strftime(buf, 80, "%y-%m-%d", &ts);
					}
				} else {
					if (capacity >= name_min + size_min + 1 + date_len + 1 + time_len) {
						pos.x -= font.StringWidth("00-00-00 00:00:00");
						strcpy(buf, "xx-xx-xx xx:xx:xx");
					} else {
						pos.x -= font.StringWidth("00-00-00");
						strcpy(buf, "xx-xx-xx");
					}
				}
				owner->MovePenTo(pos);
				owner->DrawString(buf);
				pos.x -= font.StringWidth(" ");
			}

			// draw filesize
			if (capacity >= name_min + 1 + size_min) {
				off_t size;
				entry_.GetSize(&size);

				BString sizestr;
				if (IsSymLink()) {
					sizestr.SetTo("<LNK>");
				} else if (IsFile()) {
					sizestr << size;
				} else if (IsDirectory()) {
					sizestr.SetTo("<DIR>");
				} else {
					sizestr.SetTo("<???>");
				}
				pos.x -= font.StringWidth(sizestr);
				owner->MovePenTo(pos);
				owner->DrawString(sizestr);
				pos.x -= font.StringWidth(" ");
			}

			// draw name
			BString namestr;
			if (parent_) {
				namestr = "..";
			} else {
				namestr = name;
			}
			if (IsSymLink()) {
				char linkpath[MAXPATHLEN];
				BSymLink symlink(&entry_);
				symlink.ReadLink(linkpath, MAXPATHLEN-1);

				namestr << " -> " << linkpath;
			}
			BRect namerect(frame);
			namerect.left += border_.left;
			namerect.right = pos.x;
			int32 len = (int)(namerect.Width() / font.StringWidth(" "));
			if (len < namestr.Length()) {
				int start = len - 3;
				if (start < 6) start = 6;
				namestr.Remove(start, namestr.Length());
				namestr.Append("···");
			}
			owner->MovePenTo(frame.left + border_.left, pos.y);
			owner->DrawString(namestr);
		}

	};	/* end of FilerListItem */

private:
	BDirectory	dir_;

	int32		last_selected_;
	BList		markedList_;

	int32		totalFiles_;
	int32		totalDirs_;
	off_t		totalBytes_;
	int32		markedFiles_;
	int32		markedDirs_;
	off_t		markedBytes_;

	rgb_color	color_executable_;
	rgb_color	color_selected_;
	rgb_color	color_readonly_;
	rgb_color	color_hidden_;
	rgb_color	color_cursor_;
	rgb_color	color_file_;
	rgb_color	color_dir_;

	void
	DoSort(void)
	{
		SortItems(&CompareFunc);
	}

	static int
	CompareFunc(const void* firstArg, const void* secondArg)
	{
		return CompareItemByName(*((BListItem**)firstArg), *((BListItem**)secondArg));
	}

	static int
	CompareItemByName(BListItem* bitem1, BListItem* bitem2)
	{
		FilerListItem* item1 = (FilerListItem*) bitem1;
		FilerListItem* item2 = (FilerListItem*) bitem2;

		if (item1->IsParent())
			return -1;
		if (item2->IsParent())
			return 1;

		bool isdir1 = item1->IsDirectory();
		bool isdir2 = item2->IsDirectory();
		if (isdir1 && !isdir2)
			return -1;
		else if (isdir2 && !isdir1)
			return 1;

		char name[B_FILE_NAME_LENGTH];
		item1->GetName(name);
		BString name1(name);
		item2->GetName(name);
		BString name2(name);
		return name1.Compare(name2);
	}

	void
	UpdateCounts()
	{
		totalFiles_  = 0;
		totalDirs_   = 0;
		totalBytes_  = 0;
		markedFiles_ = 0;
		markedDirs_  = 0;
		markedBytes_ = 0;

		markedList_.MakeEmpty();

		int32 count = CountItems();
		for (int32 i=0; i<count; i++) {
			FilerListItem* item = (FilerListItem*)ItemAt(i);
			if (item->IsParent())
				continue;

			if (item->IsMarked())
				markedList_.AddItem(item->GetEntry());

			if (item->IsDirectory()) {
				totalDirs_++;
				if (item->IsMarked())
					markedDirs_++;
			} else {
				totalFiles_++;
				totalBytes_ += item->GetSize();
				if (item->IsMarked()) {
					markedFiles_++;
					markedBytes_ += item->GetSize();
				}
			}
		}
	}

	void
	SendUpdateNotify()
	{
		UpdateCounts();

		BMessage msg(M_FILER_CHANGED);
		msg.AddPointer("sender", this);
		BMessenger(this).SendMessage(&msg);
	}

	void
	ScrollToSelection3()
	{
		BRect itemFrame = ItemFrame(CurrentSelection(0));

		if (Bounds().Contains(itemFrame))
			return;

		ScrollTo(itemFrame.left, itemFrame.top-itemFrame.Height()*2-2);
	}

	void
	NodeWatcherHandler(BMessage *msg)
	{
		int32 opcode;
		if (msg->FindInt32("opcode", &opcode) != B_OK)
			return;

		const char* name;
		entry_ref ref;
		node_ref nref;
		node_ref nref_from;
		node_ref nref_to;
		node_ref nref_cd;

		dir_.GetNodeRef(&nref_cd);

		switch (opcode) {

		case B_ENTRY_CREATED:
			msg->FindInt32("device", &ref.device);
			msg->FindInt64("directory", &ref.directory);
			msg->FindString("name", &name);
			ref.set_name(name);
			AddItem(ref);
			SendUpdateNotify();
			break;

		case B_ENTRY_REMOVED:
			msg->FindInt32("device", &nref.device);
			msg->FindInt64("node", &nref.node);
			RemoveItem(nref);
			SendUpdateNotify();
			break;

		case B_ENTRY_MOVED:
			// node_ref
			msg->FindInt32("device", &nref.device);
			msg->FindInt64("node", &nref.node);

			// entry_ref
			ref.device = nref.device;
			msg->FindString("name", &name);
			ref.set_name(name);

			// node_ref(s)
			nref_from.device = nref.device;
			nref_to.device = nref.device;
			msg->FindInt64("from directory", &nref_from.node);
			msg->FindInt64("to directory", &nref_to.node);

			if (nref_from == nref_to) {
				{
					BEntry entry;
					dir_.Rewind();
					while(dir_.GetNextEntry(&entry) == B_OK) {
						node_ref tmp;
						entry.GetNodeRef(&tmp);
						if (nref == tmp) {
							entry.GetRef(&ref);
							break;
						}
					}
				}
				UpdateItem(nref, ref);
				SendUpdateNotify();

			} else if (nref_from == nref_cd) {
				RemoveItem(nref);
				SendUpdateNotify();

			} else if (nref_to == nref_cd) {
				ref.directory = nref_to.node;
				AddItem(ref);
				SendUpdateNotify();
			}
			break;

		case B_STAT_CHANGED:
		case B_ATTR_CHANGED:
			Invalidate();
			break;

		default:
			break;
		}
	}

public:
	FilerListView(BRect frame, const char* name,
		list_view_type type, uint32 resizingMode, uint32 flags)
		: BListView(frame, name, type, resizingMode, flags)
	{
		last_selected_ = -1;

		totalFiles_  = 0;
		totalDirs_   = 0;
		totalBytes_  = 0;
		markedFiles_ = 0;
		markedDirs_  = 0;
		markedBytes_ = 0;
	}

	~FilerListView()
	{
		UnsetDirectory();
	}

	bool
	SetDirectory(BDirectory* dir)
	{
		BEntry entry;

		if (dir == NULL)
			return false;
		if (dir->InitCheck() != B_OK)
			return false;
		if (dir->GetEntry(&entry) != B_OK)
			return false;
		if (entry.InitCheck() != B_OK)
			return false;

		BEntry oldDirEntry;
		dir_.GetEntry(&oldDirEntry);

		UnsetDirectory();

		node_ref nref;
		entry.GetNodeRef(&nref);
		dir_ = *dir;

		// display parent directory item
		if (entry.GetParent(&entry) == B_OK) {
			FilerListItem* item = new FilerListItem(entry);
			BListView::AddItem(item);
			item->SetParentDirFlag(true);
		}

		// display directory entry item
		while(dir_.GetNextEntry(&entry) == B_OK) {
			BListView::AddItem(new FilerListItem(entry));
		}
		DoSort();
		BListView::Select(0);
		Select(oldDirEntry);

		SendUpdateNotify();

		// start NodeWatcher
		watch_node(&nref, B_WATCH_ALL, this);

		return true;
	}

	void
	UnsetDirectory()
	{
		// stop NodeWatcher
		stop_watching(this);

		// DeselectAllItems
		if (CountItems() > 0) {
			BListView::Select(CountItems()-1);
			Deselect(CountItems()-1);
		}
		DeselectAll();

		// RemoveAllItems
		for (int32 i=CountItems()-1; i>=0; i--) {
			BListItem* item;
			if ((item = BListView::RemoveItem(i)) != NULL) {
				item->Deselect();
				delete item;
			}
		}

		dir_.Unset();

		SendUpdateNotify();
	}

	void
	GetDirectory(BDirectory* dir)
	{
		*dir = dir_;
	}

	bool
	GetParentDirectory(BDirectory* dir)
	{
		BEntry entry;
		BDirectory parent;

		dir_.GetEntry(&entry);
		if (entry.GetParent(&parent) != B_OK) {
			dir->Unset();
			return false;
		}
		*dir = parent;
		return true;
	}

	void
	GetCursorEntry(BEntry* entry)
	{
		FilerListItem* item = (FilerListItem*) ItemAt(CurrentSelection());
		item->GetEntry(entry);
	}

	bool
	GetPath(BPath* path)
	{
		BEntry entry;
		if (dir_.GetEntry(&entry) != B_OK)
			return false;

		if (entry.GetPath(path) != B_OK)
			return false;

		return true;
	}

	bool
	GetPath(BString* str)
	{
		*str = "";

		BEntry entry;
		if (dir_.GetEntry(&entry) != B_OK)
			return false;

		BPath path;
		if (entry.GetPath(&path) != B_OK)
			return false;

		*str = path.Path();
	}

	int32
	GetMarkedEntryList(BList* list)
	{
		if (list == NULL)
			return -1;

		list->MakeEmpty();
		*list = markedList_;
		return list->CountItems();
	}

	bool
	AddItem(BListItem* newItem, bool sort)
	{
		if (sort) {
			int32 cnt = CountItems();
			int32 i;
			for (i=0; i<cnt; i++) {
				BListItem* listItem = ItemAt(i);
				if (CompareFunc((void*)&newItem, (void*)&listItem) < 0)
					break;
			}
			return BListView::AddItem(newItem, i);
		} else {
			return BListView::AddItem(newItem);
		}
	}

	bool
	AddItem(entry_ref ref)
	{
		BEntry entry(&ref, false);
		BListItem* item = (BListItem*) new FilerListItem(entry);
		return AddItem(item, true);
	}

	bool
	RemoveItem(node_ref nref)
	{
		bool retval = false;
		int32 count = CountItems();
		int32 index = CurrentSelection();
		for (int32 i=0; i<count; i++) {
			BListItem* listItem = ItemAt(i);
			if (((FilerListItem*)listItem)->IsThis(&nref)) {
				bool isSelected = listItem->IsSelected();
				retval = BListView::RemoveItem(listItem);
				delete listItem;
				if (isSelected) {
					BListView::Select(index);
				}
				if (index >= count-1) {
					index--;
					if (index >= 0) {
						BListView::Select(index);
					}
				}
				break;
			}
		}
		return retval;
	}

	bool
	UpdateItem(node_ref nref, entry_ref ref)
	{
		int32 cnt = CountItems();
		for (int32 i=0; i<cnt; i++) {
			BListItem* listItem = ItemAt(i);
			if (((FilerListItem*)listItem)->IsThis(&nref)) {
				BEntry entry(&ref);
				((FilerListItem*)listItem)->SetEntry(&entry);
				break;
			}
		}
		DoSort();
		Invalidate();
		return true;
	}

	void
	Select(BEntry entry)
	{
		if (entry.InitCheck() == B_OK) {
			int32 cnt = CountItems();
			int32 i;
			for (i=0; i<cnt; i++) {
				BListItem* listItem = ItemAt(i);
				BEntry listEntry;
				((FilerListItem*)listItem)->GetEntry(&listEntry);
				if (entry == listEntry) {
					BListView::Select(i);
					ScrollToSelection3();
					break;
				}
			}
		}
	}

	bool
	ToggleMark(int32 index)
	{
		FilerListItem* item;
		item = (FilerListItem*) ItemAt(index);
		if (item != NULL) {
			item->SetMarked(!item->IsMarked());
		}
		InvalidateItem(index);
		SendUpdateNotify();
		return item->IsMarked();
	}

	bool
	ToggleMark()
	{
		return ToggleMark(CurrentSelection());
	}

	void
	UnMark(BEntry* entry)
	{
		int32 count = CountItems();
		for (int32 i=0; i<count; i++) {
			FilerListItem* item;
			item = (FilerListItem*) ItemAt(i);
			if (item->IsThis(entry)) {
				ToggleMark(i);
				break;
			}
		}
	}

	bool
	IsDirectory(int32 index)
	{
		FilerListItem* item = (FilerListItem*) ItemAt(index);
		if (item != NULL) {
			return item->IsDirectory();
		}
		return false;
	}

	bool
	IsDirectory()
	{
		return IsDirectory(CurrentSelection());
	}

	void
	SelectionChanged(void)
	{
		int32 current = CurrentSelection();

		if (current >= 0)
			last_selected_ = current;
		else
			BListView::Select(last_selected_);

		BListView::SelectionChanged();
	}

	void
	KeyDown(const char* bytes, int32 numBytes)
	{
		if (numBytes == 1) {
			int32 index;
			BRect itemFrame = ItemFrame(CurrentSelection(0));

			switch (bytes[0]) {
			case B_TAB:
				BListView::KeyDown(bytes, numBytes);
				break;
			default:
				Parent()->Parent()->KeyDown(bytes, numBytes);
				break;
			}
		} else {
				Parent()->Parent()->KeyDown(bytes, numBytes);
		}
	}

	void
	MessageReceived(BMessage* message)
	{
		switch(message->what)
		{
		case B_NODE_MONITOR:
			NodeWatcherHandler(message);
			break;
		default:
			BListView::MessageReceived(message);
			break;
		}
	}

	void
	MakeFocus(bool focused)
	{
		InvalidateItem(last_selected_);
		BListView::MakeFocus(focused);
	}

	void
	CursorPageUp(void)
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
			BListView::Select(index);
			itemFrame = ItemFrame(CurrentSelection(0));
			ScrollTo(itemFrame.left, itemFrame.top-itemFrame.Height());
		}
	}

	void
	CursorPageDown(void)
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
			BListView::Select(index);
			itemFrame = ItemFrame(CurrentSelection(0));
			ScrollTo(itemFrame.left, itemFrame.bottom-Bounds().Height()+itemFrame.Height());
		}
	}

	void
	CursorUp(void)
	{
		if (CountItems() > 0) {
			int32 index = CurrentSelection();
			if (index > 0) {
				index--;
			}
			BListView::Select(index);
			ScrollToSelection();
		}
	}

	void
	CursorDown(void)
	{
		if (CountItems() > 0) {
			int32 index = CurrentSelection();
			if (index < CountItems()-1) {
				index++;
			}
			BListView::Select(index);
			ScrollToSelection();
		}
	}

	void
	CursorHome(void)
	{
		if (CountItems() > 0) {
			BListView::Select(0);
			ScrollToSelection();
		}
	}

	void
	CursorEnd(void)
	{
		if (CountItems() > 0) {
			BListView::Select(CountItems()-1);
			ScrollToSelection();
		}
	}

	int32
	GetTotalFiles()
	{
		return totalFiles_;
	}

	int32
	GetTotalDirs()
	{
		return totalDirs_;
	}

	off_t
	GetTotalBytes()
	{
		return totalBytes_;
	}

	int32
	GetMarkedFiles()
	{
		return markedFiles_;
	}

	int32
	GetMarkedDirs()
	{
		return markedDirs_;
	}

	off_t
	GetMarkedBytes()
	{
		return markedBytes_;
	}

	void
	SetBackgroundColor(rgb_color color)
	{
		SetViewColor(color);
	}

	void
	SetExecutableColor(rgb_color color)
	{
		color_executable_ = color;
	}

	void
	SetSelectedColor(rgb_color color)
	{
		color_selected_ = color;
	}

	void
	SetReadonlyColor (rgb_color color)
	{
		color_readonly_ = color;
	}

	void
	SetHiddenColor (rgb_color color)
	{
		color_hidden_ = color;
	}

	void
	SetCursorColor(rgb_color color)
	{
		color_cursor_ = color;
	}

	void
	SetFileColor(rgb_color color)
	{
		color_file_ = color;
	}

	void
	SetDirColor(rgb_color color)
	{
		color_dir_ = color;
	}

	rgb_color
	GetBackgroundColor(void) const
	{
		return ViewColor();
	}

	rgb_color
	GetExecutableColor(void) const
	{
		return color_executable_;
	}

	rgb_color
	GetSelectedColor(void) const
	{
		return color_selected_;
	}

	rgb_color
	GetReadonlyColor(void) const
	{
		return color_readonly_;
	}

	rgb_color
	GetHiddenColor(void) const
	{
		return color_hidden_;
	}

	rgb_color
	GetCursorColor(void) const
	{
		return color_cursor_;
	}

	rgb_color
	GetFileColor(void) const
	{
		return color_file_;
	}

	rgb_color
	GetDirColor(void) const
	{
		return color_dir_;
	}
};

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

	// directory name
	ctrl_rect = view_rect;
	ctrl_rect.bottom = ctrl_rect.top + B_H_SCROLL_BAR_HEIGHT;
 	sub_name.SetTo(name);
	sub_name.Append("_dir_name");
	mDirNameView = new BStringView(
		ctrl_rect, sub_name, "",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW);
	AddChild(mDirNameView);
	view_rect.top = ctrl_rect.bottom + 1;

	// count view
	ctrl_rect = view_rect;
	ctrl_rect.top = ctrl_rect.bottom - B_H_SCROLL_BAR_HEIGHT*2;
	sub_name.SetTo(name);
	sub_name.Append("_count_view");
	mCountView = new FilerCountView(
		ctrl_rect, sub_name,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
	AddChild(mCountView);
	view_rect.bottom = ctrl_rect.top - 1;

	// directory view
	ctrl_rect = view_rect;
	ctrl_rect.right -= (B_V_SCROLL_BAR_WIDTH + 1);
	ctrl_rect.left  += 1;
	sub_name.SetTo(name);
	sub_name.Append("_list_view");
	mListView = new FilerListView(
		ctrl_rect, sub_name,
		B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE);
	sub_name.SetTo(name);
	sub_name.Append("_scroll_view");
	mScrollView = new BScrollView(
		sub_name, mListView,
		B_FOLLOW_ALL_SIDES, 0, false, true, B_PLAIN_BORDER);
	AddChild(mScrollView);

	// font
	mListView ->SetFont(be_fixed_font);
	mDirNameView->SetFont(be_fixed_font);
	mCountView->SetFont(be_fixed_font);

	// color
	rgb_color viewcolor  = ViewColor();
	rgb_color highcolor = HighColor();
	rgb_color lowcolor = LowColor();
	SetBackgroundColor (lowcolor);
	SetExecutableColor(highcolor);
	SetSelectedColor (viewcolor);
	SetReadonlyColor(highcolor);
	SetHiddenColor (highcolor);
	SetCursorColor(highcolor);
	SetFileColor(highcolor);
	SetDirColor(highcolor);

	// keymap function
	keymap_func = NULL;
}

FilerView::~FilerView()
{
	mScrollView->RemoveChild(mListView);
	delete mListView;

	RemoveChild(mScrollView);
	delete mScrollView;

	RemoveChild(mCountView);
	delete mCountView;

	RemoveChild(mDirNameView);
	delete mDirNameView;
}

status_t
FilerView::OpenDirectory(const char *dirname)
{
	BDirectory dir(dirname);
	status_t ret = OpenDirectory(&dir);

	return ret;
}

status_t
FilerView::OpenDirectory(const BEntry* entry)
{
	BDirectory dir(entry);
	status_t ret = OpenDirectory(&dir);

	return ret;
}

status_t
FilerView::OpenDirectory(BDirectory* dir)
{
	((FilerListView*)mListView)->SetDirectory(dir);
	return B_OK;
}

void
FilerView::GetDirectory(BDirectory* dir)
{
	((FilerListView*)mListView)->GetDirectory(dir);
}

int32
FilerView::GetMarkedEntryList(BList* list)
{
	return ((FilerListView*)mListView)->GetMarkedEntryList(list);
}

void
FilerView::UnMark(BEntry* entry)
{
	((FilerListView*)mListView)->UnMark(entry);
}

void
FilerView::StatusUpdate(void)
{
	BString pathStr;
	((FilerListView*)mListView)->GetPath(&pathStr);
	mDirNameView->SetText(pathStr);

	int32 totalFiles, totalDirs;
	int32 markedFiles, markedDirs;
	off_t totalBytes, markedBytes;

	totalFiles  = ((FilerListView*)mListView)->GetTotalFiles();
	totalDirs   = ((FilerListView*)mListView)->GetTotalDirs();
	totalBytes  = ((FilerListView*)mListView)->GetTotalBytes();
	markedFiles = ((FilerListView*)mListView)->GetMarkedFiles();
	markedDirs  = ((FilerListView*)mListView)->GetMarkedDirs();
	markedBytes = ((FilerListView*)mListView)->GetMarkedBytes();

	((FilerCountView*)mCountView)->SetTotalCount (totalFiles,  totalDirs,  totalBytes);
	((FilerCountView*)mCountView)->SetMarkedCount(markedFiles, markedDirs, markedBytes);
}

void
FilerView::MakeFocus(bool focused)
{
	mListView->MakeFocus(focused);
}

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
FilerView::MessageReceived(BMessage *message)
{
	DEBUG_PRINT(("message:%c%c%c%c\n",
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what));
	DEBUG_PRINT(("CountNames: %d\n", message->CountNames(B_ANY_TYPE)));

	switch(message->what) {

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
		if (((FilerListView*)mListView)->IsDirectory()) {
			BEntry entry;
			((FilerListView*)mListView)->GetCursorEntry(&entry);
			OpenDirectory(&entry);
		} else {
			BMessage msg(M_OPEN_FILE);
			msg.AddPointer("sender", this);
			BMessenger(this).SendMessage(&msg);
		}
		break;

	case M_PARENT_DIRECTORY:
		{
			BDirectory parent;
			if (((FilerListView*)mListView)->GetParentDirectory(&parent))
				OpenDirectory(&parent);
		}
		break;

	case M_MARK_FILE:
		((FilerListView*)mListView)->ToggleMark();
		((FilerListView*)mListView)->CursorDown();
		break;

	case M_MARK_ALL_FILE:
		{
			int32 count = mListView->CountItems();
			for (int32 i=0; i<count; i++) {
				if (!((FilerListView*)mListView)->IsDirectory(i)) {
					((FilerListView*)mListView)->ToggleMark(i);
				}
			}
		}
		break;

	case M_FILER_CHANGED:
		StatusUpdate();
		break;

	default:
		BView::MessageReceived(message);
		break;
	}
}

void
FilerView::SetBackgroundColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetBackgroundColor(color);
}

void
FilerView::SetExecutableColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetExecutableColor(color);
}

void
FilerView::SetSelectedColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetSelectedColor(color);
}

void
FilerView::SetReadonlyColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetReadonlyColor(color);
}

void
FilerView::SetHiddenColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetHiddenColor(color);
}

void
FilerView::SetCursorColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetCursorColor(color);
}

void
FilerView::SetFileColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetFileColor(color);
}

void
FilerView::SetDirColor(uchar red, uchar green, uchar blue, uchar alpha)
{
	rgb_color color;
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	SetDirColor(color);
}

void
FilerView::SetBackgroundColor(rgb_color color)
{
	((FilerListView*)mListView)->SetBackgroundColor(color);
}

void
FilerView::SetExecutableColor(rgb_color color)
{
	((FilerListView*)mListView)->SetExecutableColor(color);
}

void
FilerView::SetSelectedColor(rgb_color color)
{
	((FilerListView*)mListView)->SetSelectedColor(color);
}

void
FilerView::SetReadonlyColor (rgb_color color)
{
	((FilerListView*)mListView)->SetReadonlyColor(color);
}

void
FilerView::SetHiddenColor (rgb_color color)
{
	((FilerListView*)mListView)->SetHiddenColor(color);
}

void
FilerView::SetCursorColor(rgb_color color)
{
	((FilerListView*)mListView)->SetCursorColor(color);
}

void
FilerView::SetFileColor(rgb_color color)
{
	((FilerListView*)mListView)->SetFileColor(color);
}

void
FilerView::SetDirColor(rgb_color color)
{
	((FilerListView*)mListView)->SetDirColor(color);
}

rgb_color
FilerView::GetBackgroundColor(void) const
{
	return ((FilerListView*)mListView)->GetBackgroundColor();
}

rgb_color
FilerView::GetExecutableColor(void) const
{
	return ((FilerListView*)mListView)->GetExecutableColor();
}

rgb_color
FilerView::GetSelectedColor(void) const
{
	return ((FilerListView*)mListView)->GetSelectedColor();
}

rgb_color
FilerView::GetReadonlyColor(void) const
{
	return ((FilerListView*)mListView)->GetReadonlyColor();
}

rgb_color
FilerView::GetHiddenColor(void) const
{
	return ((FilerListView*)mListView)->GetHiddenColor();
}

rgb_color
FilerView::GetCursorColor(void) const
{
	return ((FilerListView*)mListView)->GetCursorColor();
}

rgb_color
FilerView::GetFileColor(void) const
{
	return ((FilerListView*)mListView)->GetFileColor();
}

rgb_color
FilerView::GetDirColor(void) const
{
	return ((FilerListView*)mListView)->GetDirColor();
}

