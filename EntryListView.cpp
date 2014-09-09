/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "EntryListView.h"
#include "BafxMessage.h"


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


EntryListItem::EntryListItem(Entry* entry, EntryItemType type)
	: BListItem(),
	  entry_(entry),
	  marked_(false),
	  border_(3,0,3,0),
	  type_(type)
{
}


EntryListItem::~EntryListItem()
{
}


void
EntryListItem::SetMarked(bool mark)
{
	if (!IsParent())
		marked_ = mark;
}


bool
EntryListItem::IsMarked()
{
	return marked_;
}


bool
EntryListItem::IsParent()
{
	return (type_ == ENTRY_IS_PARENT);
}


Entry*
EntryListItem::GetEntry()
{
	return entry_;
}


bool
EntryListItem::IsDirectory()
{
	if (entry_)
		return entry_->IsDirectory();
	else
		return false;
}


status_t
EntryListItem::GetName(char* name)
{
	if (entry_) {
		if (entry_->GetName(name) == B_OK)
			return B_OK;
		else
			name[0] = '\0';
	} else {
		name[0] = 'N';
		name[1] = 'U';
		name[2] = 'L';
		name[3] = 'L';
		name[4] = '\0';
	}
	return B_ERROR;
}


off_t
EntryListItem::GetSize()
{
	off_t size;
	if (entry_) {
		if (entry_->GetSize(&size) != B_OK) {
			size = 0;
		}
	} else {
		size = 0;
	}
	return size;
}


inline void
EntryListItem::DrawBackground(EntryListView* list_view, BRect frame, bool complete)
{
	if (marked_) {
		list_view->SetHighColor(list_view->GetSelectedColor());
		list_view->SetLowColor(list_view->GetSelectedColor());
	} else {
		list_view->SetHighColor(list_view->ViewColor());
		list_view->SetLowColor(list_view->ViewColor());
	}
	if (marked_ || complete) {
		list_view->FillRect(frame);
	}
}


inline void
EntryListItem::DrawCursor(EntryListView* list_view, BRect frame)
{
	if (IsSelected() && list_view->IsFocus()) {
		BRect rect = BRect(frame);
		rect.top = rect.bottom - 1;
		list_view->SetHighColor(list_view->GetCursorColor());
		list_view->FillRect(rect);
	}
}


inline void
EntryListItem::DrawSetPenColor(EntryListView* list_view, BRect frame)
{
	rgb_color color;

	if (entry_) {
		if (entry_->IsHiddenFile()) {
			color = list_view->GetHiddenColor();
		} else if (entry_->IsReadOnly()) {
			color = list_view->GetReadonlyColor();
		} else if (entry_->IsFile()) {
			if (entry_->IsExecutable()) {
				color = list_view->GetExecutableColor();
			} else {
				color = list_view->GetFileColor();
			}
		} else if (entry_->IsDirectory()) {
			color = list_view->GetDirColor();
		} else {
			color = list_view->GetFileColor();
		}
	} else {
		color = list_view->GetFileColor();
	}

	list_view->SetHighColor(color);
}


inline void
EntryListItem::DrawSetFont(EntryListView* list_view, BRect frame, BFont &font)
{
	font_height height;
	font.GetHeight(&height);

	if (list_view->Flags() & B_SUBPIXEL_PRECISE) {
		border_.bottom = height.descent;
	} else {
		border_.bottom = (int)height.descent;
	}
}


inline void
EntryListItem::DrawFileTime(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos, bool with_time)
{
	time_t mtime;
	struct tm ts;
	char buf[80] = "";

	if (entry_ == NULL)
		return;

	if (entry_->GetModificationTime(&mtime) == B_OK) {
		localtime_r(&mtime, &ts);

		if (with_time) {
			pos.x -= font.StringWidth("00-00-00 00:00:00");
			strftime(buf, 80, "%y-%m-%d %T", &ts);
		} else {
			pos.x -= font.StringWidth("00-00-00");
			strftime(buf, 80, "%y-%m-%d", &ts);
		}
	} else {
		if (with_time) {
			pos.x -= font.StringWidth("00-00-00 00:00:00");
			strcpy(buf, "xx-xx-xx xx:xx:xx");
		} else {
			pos.x -= font.StringWidth("00-00-00");
			strcpy(buf, "xx-xx-xx");
		}
	}
	list_view->MovePenTo(pos);
	list_view->DrawString(buf);
	pos.x -= font.StringWidth(" ");
}


inline void
EntryListItem::DrawFileSize(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos)
{
	if (entry_ == NULL)
		return;

	off_t size = GetSize();

	BString sizestr;
	if (entry_->IsSymLink()) {
		sizestr.SetTo("<LNK>");
	} else if (entry_->IsFile()) {
		sizestr << size;
	} else if (entry_->IsDirectory()) {
		sizestr.SetTo("<DIR>");
	} else {
		sizestr.SetTo("<\?\?\?>");
	}
	pos.x -= font.StringWidth(sizestr.String());
	list_view->MovePenTo(pos);
	list_view->DrawString(sizestr.String());

	pos.x -= font.StringWidth(" ");
}


inline void
EntryListItem::DrawFileName(EntryListView* list_view, BRect frame, BFont &font, BPoint& pos)
{
	char name[B_FILE_NAME_LENGTH];
	BString namestr;

	if (entry_) {
		GetName(name);

		// draw name
		if (IsParent()) {
			namestr = "..";
		} else {
			namestr = name;
		}

		char linkpath[MAXPATHLEN];
		if (entry_->ReadLink(linkpath, MAXPATHLEN-1) > 0) {
			namestr << " -> " << linkpath;
		}

		BRect namerect(frame);
		namerect.left += border_.left;
		namerect.right = pos.x;
		int32 len = (int)(namerect.Width() / font.StringWidth(" "));
		if (len < namestr.Length()) {
			int start = len - 3;
			if (start < 6) start = 6;
			namestr.Remove(start, namestr.Length() - start);
			namestr.Append("···");
		}

	} else {
		namestr.Append("NULL");
	}

	list_view->MovePenTo(frame.left + border_.left, pos.y);
	list_view->DrawString(namestr.String());
}


void
EntryListItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	EntryListView* list_view = (EntryListView*)owner;

	DrawBackground(list_view, frame, complete);
	DrawCursor(list_view, frame);
	DrawSetPenColor(list_view, frame);

	BFont font;
	list_view->GetFont(&font);
	DrawSetFont(list_view, frame, font);

	int capacity = (int)(frame.Width() / font.StringWidth(" "));
	int name_min = 12;
	int size_min = 6;
	int date_len = 8;
	int time_len = 8;
	BPoint pos;
	pos.y = frame.bottom - border_.bottom;
	pos.x = frame.right  - border_.right;

	if (capacity >= name_min + 1 + size_min + 1 + date_len) {
		bool with_time = (capacity >= name_min + size_min + 1 + date_len + 1 + time_len);
		DrawFileTime(list_view, frame, font, pos, with_time);
	}
	if (capacity >= name_min + 1 + size_min) {
		DrawFileSize(list_view, frame, font, pos);
	}
	DrawFileName(list_view, frame, font, pos);
}


/****************************************************
 * class EntryListView
 ****************************************************/
EntryListView::EntryListView(BRect frame, const char* name,
	list_view_type type, uint32 resizingMode, uint32 flags)
	: BListView(frame, name, type, resizingMode, flags),
	  last_selected_(-1),
	  color_selected_(ui_color(B_MENU_SELECTION_BACKGROUND_COLOR)),
	  color_cursor_(ui_color(B_WINDOW_TAB_COLOR)),
	  color_file_(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	  color_dir_(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	  color_executable_(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	  color_readonly_(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	  color_hidden_(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	  keymap_func(bafx_key_to_message)
{
	BListView::SetFont(be_fixed_font);
}


EntryListView::~EntryListView()
{
}


void
EntryListView::AddEntryList(BList* list)
{
	int32 count = list->CountItems();
	for (int32 i=0; i<count; i++) {
		AddItem(new EntryListItem((Entry*) list->ItemAt(i)));
	}
}


void
EntryListView::DeleteAllItems()
{
	if (CountItems() > 0)
		Select(0);

	for (int32 i=CountItems()-1; i>=0; i--) {
		BListItem* item;
		if ((item = RemoveItem(i)) != NULL) {
			item->Deselect();
			delete item;
		}
	}
	last_selected_ = -1;
}


void
EntryListView::MakeFocus(bool focused)
{
	InvalidateItem(CurrentSelection());
	BListView::MakeFocus(focused);
}


void
EntryListView::FrameResized(float width, float height)
{
	BListView::FrameResized(width, height);
	ScrollToSelection();
}


void
EntryListView::ScrollToSelection()
{
	BRect itemFrame = ItemFrame(CurrentSelection(0));
	if (Bounds().Contains(itemFrame))
		return;

	float scrollPos;
	if (itemFrame.top < Bounds().top) {
		scrollPos = itemFrame.top;
	} else {
		int32 num = (int32)Bounds().Height() / (int32)(itemFrame.Height() + 1);
		int32 index = CurrentSelection(0) - num + 1;
		BRect toFrame = ItemFrame(index);
		scrollPos = toFrame.top;
	}

	ScrollTo(itemFrame.left, scrollPos);
}


void
EntryListView::ScrollToSelection3()
{
	BRect itemFrame = ItemFrame(CurrentSelection(0));

	if (Bounds().Contains(itemFrame))
		return;

	ScrollTo(itemFrame.left, itemFrame.top-itemFrame.Height()*2-2);
}


void
EntryListView::SelectionChanged(void)
{
	int32 current = CurrentSelection();

	if (current >= 0)
		last_selected_ = current;
	else
		BListView::Select(last_selected_);

	BListView::SelectionChanged();
}


void
EntryListView::Select(int32 i)
{
	BListView::Select(i);
	ScrollToSelection3();
}


void
EntryListView::SelectAt(const Entry& entry)
{
	if (CountItems() > 0) {
		int32 count = CountItems();
		for (int32 i=0; i<count; i++) {
			EntryListItem* item = (EntryListItem*)ItemAt(i);
			Entry* entry1 = item->GetEntry();
			if (*entry1 == entry) {
				BListView::Select(i);
				ScrollToSelection3();
				break;
			}
		}
	}
}


void
EntryListView::KeyDown(const char* bytes, int32 numBytes)
{
	DEBUG_PRINT(("numBytes=%d, modifiers=0x%08x, bytes[0]=0x%02x\n",
		numBytes, modifiers(), bytes[0]));

	int32 cmd;

	if (numBytes == 1) {
		switch (bytes[0]) {
		case B_TAB:
			BListView::KeyDown(bytes, numBytes);
			break;
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
EntryListView::CursorUp(void)
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
EntryListView::CursorDown(void)
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
EntryListView::CursorPageUp(void)
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
EntryListView::CursorPageDown(void)
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
EntryListView::CursorHome(void)
{
	if (CountItems() > 0) {
		BListView::Select(0);
		ScrollToSelection();
	}
}


void
EntryListView::CursorEnd(void)
{
	if (CountItems() > 0) {
		BListView::Select(CountItems()-1);
		ScrollToSelection();
	}
}


bool
EntryListView::ToggleMark(int32 index)
{
	EntryListItem* item;
	item = (EntryListItem*) ItemAt(index);
	if (item != NULL) {
		item->SetMarked(!item->IsMarked());
		InvalidateItem(index);
	}
	return item->IsMarked();
}

/*
void
EntryListView::SendUpdateNotify()
{
	BMessage msg(M_FILER_CHANGED);
	msg.AddPointer("sender", this);
	BMessenger(this).SendMessage(&msg);
}
*/

bool
EntryListView::IsDirectory()
{
	EntryListItem* item = (EntryListItem*)ItemAt(CurrentSelection(0));
	return item->IsDirectory();
}


bool
EntryListView::IsParent()
{
	EntryListItem* item = (EntryListItem*)ItemAt(CurrentSelection(0));
	return item->IsParent();
}


Entry*
EntryListView::GetEntry()
{
	EntryListItem* item = (EntryListItem*)ItemAt(CurrentSelection(0));
	return item->GetEntry();
}


int
CompareItemByName(const void* bitem1, const void* bitem2)
{
	EntryListItem* item1 = *(EntryListItem**) bitem1;
	EntryListItem* item2 = *(EntryListItem**) bitem2;

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
EntryListView::DoSort()
{
	SortItems(&CompareItemByName);
}


void
EntryListView::SetBackgroundColor(rgb_color color)
{
	SetViewColor(color);
}


void
EntryListView::SetExecutableColor(rgb_color color)
{
	color_executable_ = color;
}


void
EntryListView::SetSelectedColor(rgb_color color)
{
	color_selected_ = color;
}


void
EntryListView::SetReadonlyColor (rgb_color color)
{
	color_readonly_ = color;
}


void
EntryListView::SetHiddenColor (rgb_color color)
{
	color_hidden_ = color;
}


void
EntryListView::SetCursorColor(rgb_color color)
{
	color_cursor_ = color;
}


void
EntryListView::SetFileColor(rgb_color color)
{
	color_file_ = color;
}


void
EntryListView::SetDirColor(rgb_color color)
{
	color_dir_ = color;
}


rgb_color
EntryListView::GetBackgroundColor(void) const
{
	return ViewColor();
}


rgb_color
EntryListView::GetExecutableColor(void) const
{
	return color_executable_;
}


rgb_color
EntryListView::GetSelectedColor(void) const
{
	return color_selected_;
}


rgb_color
EntryListView::GetReadonlyColor(void) const
{
	return color_readonly_;
}


rgb_color
EntryListView::GetHiddenColor(void) const
{
	return color_hidden_;
}


rgb_color
EntryListView::GetCursorColor(void) const
{
	return color_cursor_;
}


rgb_color
EntryListView::GetFileColor(void) const
{
	return color_file_;
}


rgb_color
EntryListView::GetDirColor(void) const
{
	return color_dir_;
}


/*
*/

/*


*/


/*
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
	BListItem* item = (BListItem*) new EntryListItem(entry);
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
		if (((EntryListItem*)listItem)->IsThis(&nref)) {
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
		if (((EntryListItem*)listItem)->IsThis(&nref)) {
			BEntry entry(&ref);
			((EntryListItem*)listItem)->SetEntry(&entry);
			break;
		}
	}
	DoSort();
	Invalidate();
	return true;
}

*/
