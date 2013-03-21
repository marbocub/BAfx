/*
 * Copyright 2012 @marbocub <marbocub @ google mail>
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
//#include <TextView.h>
#include <StringView.h>
#include <String.h>
#include <List.h>
#include <Entry.h>
#include <Path.h>

//#define BAFX_WINDOW_DEBUG
#ifdef BAFX_WINDOW_DEBUG
#  include <stdio.h>
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
		"終了",
		new BMessage(B_QUIT_REQUESTED),
		'Q'));
	AddChild(menu_bar);
	p_menu = new BMenu("編集");
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
//	mLogView = new BTextView(
//		mVPanedView->Bounds2(),
//		"log_view",
//		mVPanedView->Bounds2(),
//		B_FOLLOW_ALL_SIDES,
//		B_WILL_DRAW | B_PULSE_NEEDED
//	);
	mLogView = new LogView(
		mVPanedView->Bounds2(),
		"log_view"
	);
	mVPanedView->SetChild2(mLogView);

	mFilerView1->SetBackgroundColor(0,0,64);
	mFilerView1->SetSelectedColor(100,100,100);
	mFilerView1->SetHiddenColor (0,0,127);
	mFilerView1->SetCursorColor(255,255,0);
	mFilerView1->SetFileColor(255,255,255);
	mFilerView1->SetDirColor(0,255,255);

	mFilerView2->SetBackgroundColor(mFilerView1->GetBackgroundColor());
	mFilerView2->SetSelectedColor(mFilerView1->GetSelectedColor());
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
BafxWindow::CopyMoveFile(BList* list, entry_ref to_ref, int32 what)
{
	BEntry entry(&to_ref);
	BPath path;
	if (entry.InitCheck() != B_OK) {
		return;
	}
	entry.GetPath(&path);

	int count = list->CountItems();
	for (int32 i=0; i<count; i++) {
		entry_ref *ref = (entry_ref*)(list->ItemAt(i));
BString str;
str << "Copying " << ref->name << " => " << path.Path();
//sprintf(s, "Copying %s => %s ...", ref->name, path.Path());
PutText(str.String());
PutText(" COPY function is not implement yet.\n");
	}
PutText("Ready.\n");
}

void
BafxWindow::MessageReceived(BMessage* message)
{
#ifdef BAFX_WINDOW_DEBUG
	printf("BafxWindow:message:%c%c%c%c\n",
		message->what>>24,
		message->what>>16,
		message->what>>8,
		message->what);
	printf("CountNames: %d\n", message->CountNames(B_ANY_TYPE));
#endif

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
		{
			FilerView* sender;
			entry_ref  from_ref, to_ref;
			message->FindPointer("sender", (void**)&sender);

			if (sender == mFilerView1) {
				to_ref = mFilerView2->GetDirectoryRef();
			} else if (sender == mFilerView2) {
				to_ref = mFilerView1->GetDirectoryRef();
			} else {
				break;
			}
			from_ref = sender->GetDirectoryRef();
			if (from_ref == to_ref) {
				PutText("Warn: Same Directory\n");
				break;
			}

			BList* list = sender->GetMarkedList();
			CopyMoveFile(list, to_ref, message->what);
			sender->CloseMarkedList(list);
		}
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

