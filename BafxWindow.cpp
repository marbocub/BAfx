/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "BafxWindow.h"
#include "BafxMessage.h"

#include <Application.h>
#include <Window.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <View.h>
#include <StringView.h>
#include <Alert.h>
#include <String.h>
#include <List.h>
#include <Directory.h>
#include <Entry.h>
#include <Path.h>

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


BafxWindow::BafxWindow (BRect frame, const char *title,
		window_type type, uint32 flags, uint32 workspace)
		: BWindow(frame, title, type, flags, workspace)
{
	BRect view_rect = Bounds();

	//
	// menu bar
	//
	BMenuBar* menu_bar = new BMenuBar(BRect(0,0,0,0), "menu_bar");
	BMenu* p_menu;
	p_menu = new BMenu("BAfx");
	menu_bar->AddItem(p_menu);
	p_menu->AddItem(new BMenuItem(
		"About...",
		new BMessage(B_ABOUT_REQUESTED)));
	p_menu->AddSeparatorItem();
	p_menu->AddItem(new BMenuItem(
		"Quit",
		new BMessage(B_QUIT_REQUESTED),
		'Q'));
	AddChild(menu_bar);
	p_menu = new BMenu("Edit");
	menu_bar->AddItem(p_menu);

	view_rect.top = menu_bar->Bounds().bottom + 1;

	//
	// status view
	//
	BRect status_rect = BRect(view_rect);
	view_rect.bottom -=  B_H_SCROLL_BAR_HEIGHT;
	status_rect.top = view_rect.bottom + 1;
	BStringView* status_string = new BStringView(
		status_rect,
		"status_string",
		"",
		B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT
	);
	AddChild(status_string);

	//
	// vpaned
	//
	mVPanedView = new PanedView(
		view_rect,
		"vpaned_view",
		PanedView::V_PANED,
		PanedView::PANED_FOLLOW_RIGHTBOTTOM
	);
	mVPanedView->SetSplitPosition(0.8 * view_rect.Height());
	this->AddChild(mVPanedView);

	//
	// hpaned
	//
	mPanedView = new PanedView(
		mVPanedView->Bounds1(),
		"hpaned_view",
		PanedView::H_PANED,
		PanedView::PANED_FOLLOW_KEEP_RATIO
	);
	mVPanedView->SetChild1(mPanedView);
	mPanedView->SetRightBorder(-1);

	//
	// filers
	//
	mFilerView1 = new FilerView(mPanedView->Bounds1(), "filer1");
	mFilerView2 = new FilerView(mPanedView->Bounds2(), "filer2");
	mFilerView1->SetKeymapFunction(bafx_key_to_message);
	mFilerView2->SetKeymapFunction(bafx_key_to_message);
	mPanedView->SetChild1(mFilerView1);
	mPanedView->SetChild2(mFilerView2);

	mFilerView1->OpenDirectory("/");
	mFilerView2->OpenDirectory("/boot/home");

	//
	// log view
	//
	mLogView = new LogView(
		mVPanedView->Bounds2(),
		"log_view"
	);
	mVPanedView->SetChild2(mLogView);

	mFilerView1->SetBackgroundColor (16,16,16);
	mFilerView1->SetExecutableColor(255,255,255);
	mFilerView1->SetSelectedColor (100,100,100);
	mFilerView1->SetReadonlyColor(255,255,127);
	mFilerView1->SetHiddenColor (0,0,191);
	mFilerView1->SetCursorColor(255,255,0);
	mFilerView1->SetFileColor(255,255,255);
	mFilerView1->SetDirColor(0,255,255);

	mFilerView2->SetBackgroundColor (mFilerView1->GetBackgroundColor());
	mFilerView2->SetExecutableColor(mFilerView1->GetExecutableColor());
	mFilerView2->SetSelectedColor (mFilerView1->GetSelectedColor());
	mFilerView2->SetReadonlyColor(mFilerView1->GetReadonlyColor());
	mFilerView2->SetHiddenColor (mFilerView1->GetHiddenColor());
	mFilerView2->SetCursorColor(mFilerView1->GetCursorColor());
	mFilerView2->SetFileColor(mFilerView1->GetFileColor());
	mFilerView2->SetDirColor(mFilerView1->GetDirColor());

	mLogView->SetBackgroundColor(mFilerView1->GetBackgroundColor());
	mLogView->SetTextColor(mFilerView1->GetFileColor());

	mFilerView1->MakeFocus();
	
	PutText("BAfx loaded.\n\n");
	PutText("Ready.\n");
}

void
BafxWindow::PutText(const char* text)
{
	mLogView->PutText(text);
}

void
BafxWindow::PutEntryName(BEntry* entry)
{
	char name[B_FILE_NAME_LENGTH];
	entry->GetName(name);
	PutText(name);
}

/*
void
BafxWindow::PutFormattedEntryName(BEntry* entry, BAFX_WINDOW_OPCODE opcode, int32 length=40)
{
	char name[B_FILE_NAME_LENGTH];
	entry->GetName(name);
	PutText(name);
}
*/

void
BafxWindow::UnMark(FilerView* sender, BEntry* entry, BList* markedList)
{
	BEntry* first = (BEntry*) markedList->FirstItem();
	if (*entry == *first) {
		sender->UnMark(entry);
		markedList->RemoveItem((int32)0);
	}
}

void
BafxWindow::CopyMoveRemoveFile(BMessage* message)
{
	FilerView* sender;
	{
		message->FindPointer("sender", (void**)&sender);
	}

	BDirectory destTopDir;
	BDirectory srcTopDir;
	{
		if (sender == mFilerView1) {
			mFilerView1->GetDirectory(&srcTopDir);
			mFilerView2->GetDirectory(&destTopDir);
		} else if (sender == mFilerView2) {
			mFilerView2->GetDirectory(&srcTopDir);
			mFilerView1->GetDirectory(&destTopDir);
		} else {
			return;
		}
	}

	BList markedList;
	{
		sender->GetMarkedEntryList(&markedList);
		if (markedList.CountItems() <= 0) {
			return;
		}
	}

	// operation type
	BString strOperation;
	{
		switch (message->what) {
		case M_COPY_FILE:
			strOperation = "Copy";
			break;
		case M_MOVE_FILE:
			strOperation = "Move";
			break;
		case M_DELETE_FILE:
			strOperation = "Remove";
			break;
		}
	}

	// show aleart
	{
		int32 count = markedList.CountItems();
		BString str(strOperation);
		str << " " << count << " marked items?";
		BAlert* alert = new BAlert("BAfx", str.String(),
			"Cancel", "OK", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		alert->SetShortcut(1, B_ENTER);
		if (alert->Go() != 1) {
			return;
		}
	}

	// collect target files
	int32 totalItems = 0;
	int32 totalFiles = 0;
	int32 totalDirs  = 0;
	const EntryList srcList(&markedList);
	const int32 count = srcList.CountItems();
	{
		for (int32 i=0; i<count; i++) {
			BEntry* entry = ((BEntry*)srcList.ItemAt(i));
			if (entry != NULL) {
				if (entry->IsDirectory()) {
					totalDirs++;
				} else {
					totalFiles++;
				}
				totalItems++;
			}
		}

		BString str;
		str << "Tatget:";
		if (totalFiles > 0) {
			str << " " << totalFiles << " File(s)";
		}
		if (totalDirs > 0) {
			if (totalFiles > 0) {
				str << " and";
			}
			str << " " << totalDirs << " Dir(s)";
		}
		str << "\n";
		PutText(str);
	}

	// start operation
	int32 copyedFiles = 0;
	int32 copyedDirs  = 0;
	int32 skipedFiles = 0;
	int32 skipedDirs  = 0;
	PutText("Start\n");
	switch (message->what) {
	case M_COPY_FILE:
		{
			BDirectory destDir(destTopDir);
			EntryStack directoryStack;
			bool overwriteDir  = false;
			bool overwriteFile = false;
			for (int32 i=0; i<count; i++) {
				status_t retval;
				BEntry* entry = ((BEntry*)srcList.ItemAt(i));

				if (entry == NULL) {
					// cd ..
					PutText("finishing directory...");

					BDirectory upDir;
					retval = FileUtils::GetParentDirectory(&destDir, &upDir);
					if (retval == B_OK) {
						destDir = upDir;
						entry = directoryStack.Pop();
						BDirectory srcDir(entry);
						FileUtils::CopyDirectoryPermission(&srcDir, &destDir);
						UnMark(sender, entry, &markedList);
						PutText("done.\n");
					} else {
						BString str;
						str << "error chdir parent of destDir, code=" << retval << ".\n";
						PutText(str);
					}
				} else if (entry->IsDirectory()) {
					// mkdir and chdir
					PutText("mkdir ");
					PutEntryName(entry);
					PutText("...");
					char name[B_FILE_NAME_LENGTH];
					entry->GetName(name);
					BDirectory newDir;
					retval = FileUtils::MakeDirectory(&destDir, &newDir, name, overwriteDir);
					if (retval == B_OK) {
						destDir = newDir;
						directoryStack.Push(entry);
						PutText("done.\n");
					} else {
						BString str;
						str << "error code=" << retval << ".\n";
						PutText(str);
					}
				} else {
					// copy
					PutText("copy ");
					PutEntryName(entry);
					PutText("...");
					char name[B_FILE_NAME_LENGTH];
					entry->GetName(name);
					retval = FileUtils::CopyFile(entry, &destDir, name, overwriteFile);
					if (retval == B_OK) {
						UnMark(sender, entry, &markedList);
						PutText("done.\n");
					} else {
						BString str;
						str << "error code=" << retval << ".\n";
						PutText(str);
					}
				}

				if (retval != B_OK) {
					PutText("Abort(´・ω・｀)\n");
					break;
				}
			}
		}
		break;

	case M_MOVE_FILE:
		// move
		{
			BDirectory destDir(destTopDir);
			int32 count = markedList.CountItems();
			for (int32 i=0; i<count; i++) {
				status_t retval;
				BEntry* entry = ((BEntry*)markedList.ItemAt(i));

				// move entry
				PutText("move ");
				PutEntryName(entry);
				PutText("...");
				retval = FileUtils::MoveEntry(entry, &destDir);
				if (retval == B_OK) {
					PutText("done.\n");
				} else {
					BString str;
					str << "error code=" << retval << ".\n";
					PutText(str);
				}
				if (retval != B_OK) {
					PutText("Abort(´・ω・｀)\n");
					break;
				}
			}
		}
		break;

	case M_DELETE_FILE:
		// delete
		{
			BDirectory destDir(destTopDir);
			EntryStack directoryStack;
			for (int32 i=0; i<count; i++) {
				status_t retval;
				BEntry* entry = ((BEntry*)srcList.ItemAt(i));

				if (entry == NULL) {
					// delete directory
					if ((entry = directoryStack.Pop()) == NULL) {
						PutText("BUG, BAfx internal error.\n");
						break;
					}
					PutText("rmdir ");
					PutEntryName(entry);
					PutText("...");
					FileUtils::RemoveEntry(entry); // ignore error
					PutText("done.\n");
				} else if (entry->IsDirectory()) {
					// push directory
					directoryStack.Push(entry);
				} else {
					// delete file
					PutText("rm ");
					PutEntryName(entry);
					PutText("...");
					FileUtils::RemoveEntry(entry); // ignore error
					PutText("done.\n");
				}
			}
		}
		break;
	}

	// end operation
	PutText("Ready.\n");

/*
		{
			FilerView* sender;
			message->FindPointer("sender", (void**)&sender);
			BList list;
			sender->GetMarkedEntryList(&list);

			do {
				entry_ref from_ref, to_ref;
				from_ref = sender->GetDirectoryRef();
				if (sender == mFilerView1) {
					to_ref = mFilerView2->GetDirectoryRef();
				} else if (sender == mFilerView2) {
					to_ref = mFilerView1->GetDirectoryRef();
				} else {
					break;
				}

				if (message->what != M_DELETE_FILE) {
					if (from_ref == to_ref) {
						PutText("Warn: Same Directory\n");
						break;
					}
				}

				CopyMoveFile(list, to_ref, message->what, sender);
			} while(false);
			PutText("Ready.\n");
		}
	BEntry destEntry(&to_ref);
	BDirectory destDir(&to_ref);
 	BPath path;
	if (destEntry.InitCheck() != B_OK) {
		return;
	}
	destEntry.GetPath(&path);


	for (int32 i=0; i<count; i++) {
		BEntry srcEntry;
		entry_ref *ref = (entry_ref*)(list->ItemAt(i));

		char name[B_FILE_NAME_LENGTH];
		srcEntry.GetName(name);

		BString str;

		switch(what) {
		case M_COPY_FILE:
			str << "cp : " << ref->name << " ...";
			PutText(str.String());
			try {
				if (FileUtils::Copy(srcEntry, destDir)) {
					PutText(" done.\n");
				} else {
					PutText(" error.\n");
				}
			} catch(status_t t) {
				switch(t) {
				case B_CREATE_FILE:
					PutText("Create File Error.\n");
					break;
				case B_FAIL_IF_EXISTS:
					PutText("Fail If Exists Error.\n");
					break;
				case B_ERASE_FILE:
					PutText("Erase File Error.\n");
					break;
				case B_OPEN_AT_END:
					PutText("Open At End Error.\n");
					break;
				default:
					PutText("error\n");
					break;
				}
			}
			break;
		case M_MOVE_FILE:
			str << "mv : " << ref->name << " ...";
			PutText(str.String());
			if (FileUtils::Move(srcEntry, destDir)) {
				PutText(" done.\n");
			} else {
				PutText(" error.\n");
			}
			break;
		case M_DELETE_FILE:
			str << "rm : " << ref->name << " ...";
			PutText(str.String());
			if (FileUtils::Remove(srcEntry)) {
				PutText(" done.\n");
			} else {
				PutText(" error.\n");
			}
			break;
		}

	}
*/
}

void
BafxWindow::MessageReceived(BMessage* message)
{
	DEBUG_PRINT(("%s:message:%c%c%c%c\n",
		__FUNCTION__,
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what));
	DEBUG_PRINT(("CountNames: %d\n", message->CountNames(B_ANY_TYPE)));

	switch(message->what)
	{
	case M_FOCUS_LEFT:
		mFilerView1->MakeFocus();
		break;
	case M_FOCUS_RIGHT:
		mFilerView2->MakeFocus();
		break;
	case M_COPY_FILE:
	case M_MOVE_FILE:
	case M_DELETE_FILE:
		CopyMoveRemoveFile(message);
		break;

	case M_SHOW_MESSAGE:
		break;

	default:
		BWindow::MessageReceived(message);
		break;
	}
}

void
BafxWindow::FrameResized(float width, float height)
{
	mPanedView->SetSplitPosition(width/2.0);
}

void
BafxWindow::Quit (void)
{
	be_app->PostMessage(B_QUIT_REQUESTED, be_app);
	BWindow::Quit();
}

