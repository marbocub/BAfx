/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "DirectoryView.h"
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
#include <SymLink.h>

#include <cstdio>	// for sprintf


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/****************************************************
 * class EntryCountView
 ****************************************************/
class EntryCountView : public BView
{
private:

	BStringView*	mTotalCount_;
	BStringView*	mMarkedCount_;
	int32			totalFiles_;
	int32			totalDirs_;
	off_t			totalBytes_;
	int32			markedFiles_;
	int32			markedDirs_;
	off_t			markedBytes_;


public:

	EntryCountView(
			BRect frame, const char* name,
			uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW)
		: BView(frame, name, resizingMode, flags),
		  mTotalCount_(new BStringView(BRect(0,0,0,0),NULL,"",B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW)),
		  mMarkedCount_(new BStringView(BRect(0,0,0,0),NULL,"",B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW)),
		  totalFiles_(0),
		  totalDirs_(0),
		  totalBytes_(0),
		  markedFiles_(0),
		  markedDirs_(0),
		  markedBytes_(0)
	{
		AddChild(mTotalCount_);
		AddChild(mMarkedCount_);
	}


	~EntryCountView()
	{
		RemoveChild(mTotalCount_);
		delete mTotalCount_;

		RemoveChild(mMarkedCount_);
		delete mMarkedCount_;
	}


	void AttachedToWindow()
	{
		if (Parent()) {
			SetViewColor(Parent()->ViewColor());
			SetHighColor(Parent()->HighColor());
			SetLowColor (Parent()->LowColor() );
		}

		BRect rect;
		rect = Bounds();
		rect.bottom = rect.top + B_H_SCROLL_BAR_HEIGHT - 1;
		mTotalCount_->ResizeTo(rect.Width(), rect.Height());
		mTotalCount_->MoveTo(rect.left, rect.top);

		rect = Bounds();
		rect.top = rect.top + B_H_SCROLL_BAR_HEIGHT;
		mMarkedCount_->ResizeTo(rect.Width(), rect.Height());
		mMarkedCount_->MoveTo(rect.left, rect.top);
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
		Display();
	}


	void SetFiles(int32 files)
	{
		totalFiles_ = files;
		Display();
	}


	void SetDirs(int32 dirs)
	{
		totalDirs_ = dirs;
		Display();
	}


	void SetBytes(off_t bytes)
	{
		totalBytes_ = bytes;
		Display();
	}


	void SetMarkedCount(int32 files, int32 dirs, off_t bytes)
	{
		markedFiles_ = files;
		markedDirs_  = dirs;
		markedBytes_ = bytes;
		Display();
	}


	void SetMarkedFiles(int32 files)
	{
		markedFiles_ = files;
		Display();
	}


	void SetMarkedDirs(int32 dirs)
	{
		markedDirs_ = dirs;
		Display();
	}


	void SetMarkedBytes(off_t bytes)
	{
		markedBytes_ = bytes;
		Display();
	}


private:

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


	void Display()
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
};


/****************************************************
 * class DirectoryView
 ****************************************************/

DirectoryView::DirectoryView(BRect frame, const char* name)
	: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	  mDirNameView(new BStringView(BRect(0,0,0,0),NULL,"",
	  		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,B_WILL_DRAW)),
	  mListView(new EntryListView(BRect(0,0,0,0),NULL,
	  		B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)),
	  mScrollView(new BScrollView(NULL,mListView,
	  		B_FOLLOW_ALL_SIDES,0,false,true,B_PLAIN_BORDER)),
	  mCountView(new EntryCountView(BRect(0,0,0,0),NULL,
	  		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW)),
	  entry_(NULL),
	  parent_(NULL),
	  entryList_()
{
	AddChild(mDirNameView);
	AddChild(mCountView);
	AddChild(mScrollView);

	mListView->SetFont(be_fixed_font);
	mDirNameView->SetFont(be_fixed_font);
	mCountView->SetFont(be_fixed_font);
}


DirectoryView::~DirectoryView()
{
	mScrollView->RemoveChild(mListView);
	delete mListView;

	RemoveChild(mScrollView);
	delete mScrollView;

	RemoveChild(mCountView);
	delete mCountView;

	RemoveChild(mDirNameView);
	delete mDirNameView;

	if (entry_)
		delete entry_;
	if (parent_)
		delete parent_;
	entryList_.DeleteAllItems();
}


status_t
DirectoryView::OpenDirectory(const char *pathname)
{
	BEntry entry(pathname);
	Directory dir(entry);

	status_t ret = OpenDirectory(&dir);

	return ret;
}


status_t
DirectoryView::OpenDirectory(const Entry* dir)
{
	if (! dir->IsDirectory()) {
		return B_ERROR;
	}

	// stop node watcher
	stop_watching(this);

	// clear entry data and listview
	mListView->DeleteAllItems();
 	entryList_.DeleteAllItems();
	if (entry_) {
		delete entry_;
		entry_ = NULL;
	}
	if (parent_) {
		delete parent_;
		parent_ = NULL;
	}
	UpdateStatus();

	// set current directory
	entry_ = new Directory(*(Directory*)dir);

	// get parent directory
	BEntry bentry;
	if (entry_->GetBEntry(&bentry) == B_OK) {
		BEntry parent;
		if (bentry.GetParent(&parent) == B_OK) {
			parent_ = new Directory(parent);
			mListView->AddItem(new EntryListItem(parent_, EntryListItem::ENTRY_IS_PARENT));
		}
	}

	// get directory entry
	ListVisitor* lv1 = new ListVisitor(*entry_, &entryList_);
	lv1->Go();
	delete lv1;

	// show directory entry
	mListView->AddEntryList((BList*) &entryList_);
	mListView->DoSort();
	if (mListView->CountItems() > 0)
		mListView->Select(0);
	UpdateStatus();

	// start node watcher
	if (bentry.InitCheck() == B_OK) {
		node_ref nref;
		bentry.GetNodeRef(&nref);
		watch_node(&nref, B_WATCH_ALL, this);
	}

	return B_OK;
}


void
DirectoryView::UpdateStatus()
{
	BString path;
	if (!entry_) {
		mDirNameView->SetText("");
	} else if (entry_->IsDirectory()) {
		entry_->GetPath(&path);
		mDirNameView->SetText(path.String());
	} else {
		mDirNameView->SetText("");
	}

	int32 totalFiles=0,  totalDirs=0;
	int32 markedFiles=0, markedDirs=0;
	off_t totalBytes=0,  markedBytes=0, size=0;

	int32 count = mListView->CountItems();
	for (int32 i=0; i<count; i++) {
		EntryListItem* item = (EntryListItem*)mListView->ItemAt(i);
		if (item->IsParent())
			continue;

		if (item->IsDirectory()) {
			totalDirs++;
			if (item->IsMarked())
				markedDirs++;
		} else {
			off_t size = item->GetSize();
			totalFiles++;
			totalBytes += size;
			if (item->IsMarked()) {
				markedFiles++;
				markedBytes += size;
			}
		}
	}

	((EntryCountView*)mCountView)->SetTotalCount (totalFiles,  totalDirs,  totalBytes);
	((EntryCountView*)mCountView)->SetMarkedCount(markedFiles, markedDirs, markedBytes);
}


void
DirectoryView::AttachedToWindow()
{
	BRect	view_rect = Bounds();
	BRect	rect(view_rect);

	if (Parent()) {
		SetViewColor(Parent()->ViewColor());
		SetHighColor(Parent()->HighColor());
		SetLowColor (Parent()->LowColor() );
	}

	rect.bottom = rect.top + B_H_SCROLL_BAR_HEIGHT;
	mDirNameView->ResizeTo(rect.Width(), rect.Height());
	mDirNameView->MoveTo(rect.left, rect.top);
	view_rect.top = rect.bottom + 1;

	rect = view_rect;
	rect.top = rect.bottom - B_H_SCROLL_BAR_HEIGHT*2;
	mCountView->ResizeTo(rect.Width(), rect.Height());
	mCountView->MoveTo(rect.left, rect.top);
	view_rect.bottom = rect.top - 1;

	rect = view_rect;
	mScrollView->ResizeTo(rect.Width(), rect.Height());
	mScrollView->MoveTo(rect.left, rect.top);

	BView::AttachedToWindow();
}


void
DirectoryView::MakeFocus(bool focused)
{
	mListView->MakeFocus(focused);
}


void
DirectoryView::MessageReceived(BMessage *message)
{
	DEBUG_PRINT(("message:%c%c%c%c\n",
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what));
	DEBUG_PRINT(("CountNames: %d\n", message->CountNames(B_ANY_TYPE)));

	switch(message->what) {
	//
	// cursor move
	//
	case M_CURSOR_UP:
		((EntryListView*)mListView)->CursorUp();
		break;
	case M_CURSOR_DOWN:
		((EntryListView*)mListView)->CursorDown();
		break;
	case M_CURSOR_PAGEUP:
		((EntryListView*)mListView)->CursorPageUp();
		break;
	case M_CURSOR_PAGEDOWN:
		((EntryListView*)mListView)->CursorPageDown();
		break;
	case M_CURSOR_HOME:
		((EntryListView*)mListView)->CursorHome();
		break;
	case M_CURSOR_END:
		((EntryListView*)mListView)->CursorEnd();
		break;

	//
	// filer reqest
	//
	case M_ENTER_FILE:
		if (((EntryListView*)mListView)->IsDirectory()) {
			bool parent = mListView->IsParent();
			Entry* entry = ((EntryListView*)mListView)->GetEntry();
			if (entry) {
				Entry* cd = entry->duplicate();
				Entry* wd = entry_->duplicate();
				OpenDirectory(cd);
				if (parent)
					mListView->SelectAt(*wd);
				delete cd;
				delete wd;
				break;
			}
		}
		{
			BMessage msg(M_OPEN_FILE);
			msg.AddPointer("sender", this);
			BMessenger(this).SendMessage(&msg);
		}
		break;

	case M_PARENT_DIRECTORY:
		if (parent_) {
			Entry* cd = parent_->duplicate();
			Entry* wd = entry_->duplicate();
			OpenDirectory(cd);
			mListView->SelectAt(*wd);
			delete cd;
			delete wd;
		}
		break;

	case M_MARK_FILE:
		((EntryListView*)mListView)->ToggleMark(mListView->CurrentSelection());
		UpdateStatus();
		((EntryListView*)mListView)->CursorDown();
		break;

	case M_MARK_ALL_FILE:
		{
			int32 count = mListView->CountItems();
			for (int32 i=0; i<count; i++) {
				Entry* entry;
				entry = ((EntryListItem*)mListView->ItemAt(i))->GetEntry();
				if (! entry->IsDirectory()) {
					mListView->ToggleMark(i);
				}
			}
			mListView->Invalidate();
			UpdateStatus();
		}
		break;

	case M_FILER_CHANGED:
		UpdateStatus();
		break;

	case B_NODE_MONITOR:
//		NodeWatcherHandler(message);
		break;

	default:
		BView::MessageReceived(message);
		break;
	}
}


/*
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
*/


void
DirectoryView::SetBackgroundColor(rgb_color color)
{
	((EntryListView*)mListView)->SetBackgroundColor(color);
}


void
DirectoryView::SetExecutableColor(rgb_color color)
{
	((EntryListView*)mListView)->SetExecutableColor(color);
}


void
DirectoryView::SetSelectedColor(rgb_color color)
{
	((EntryListView*)mListView)->SetSelectedColor(color);
}


void
DirectoryView::SetReadonlyColor (rgb_color color)
{
	((EntryListView*)mListView)->SetReadonlyColor(color);
}


void
DirectoryView::SetHiddenColor (rgb_color color)
{
	((EntryListView*)mListView)->SetHiddenColor(color);
}


void
DirectoryView::SetCursorColor(rgb_color color)
{
	((EntryListView*)mListView)->SetCursorColor(color);
}


void
DirectoryView::SetFileColor(rgb_color color)
{
	((EntryListView*)mListView)->SetFileColor(color);
}


void
DirectoryView::SetDirColor(rgb_color color)
{
	((EntryListView*)mListView)->SetDirColor(color);
}


rgb_color
DirectoryView::GetBackgroundColor(void) const
{
	return ((EntryListView*)mListView)->GetBackgroundColor();
}


rgb_color
DirectoryView::GetExecutableColor(void) const
{
	return ((EntryListView*)mListView)->GetExecutableColor();
}


rgb_color
DirectoryView::GetSelectedColor(void) const
{
	return ((EntryListView*)mListView)->GetSelectedColor();
}


rgb_color
DirectoryView::GetReadonlyColor(void) const
{
	return ((EntryListView*)mListView)->GetReadonlyColor();
}


rgb_color
DirectoryView::GetHiddenColor(void) const
{
	return ((EntryListView*)mListView)->GetHiddenColor();
}


rgb_color
DirectoryView::GetCursorColor(void) const
{
	return ((EntryListView*)mListView)->GetCursorColor();
}


rgb_color
DirectoryView::GetFileColor(void) const
{
	return ((EntryListView*)mListView)->GetFileColor();
}


rgb_color
DirectoryView::GetDirColor(void) const
{
	return ((EntryListView*)mListView)->GetDirColor();
}
