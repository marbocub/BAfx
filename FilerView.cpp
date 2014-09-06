/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "FilerView.h"
#include "BafxMessage.h"

#include <cstdio>


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) {printf("%s:%s:", Name(),__FUNCTION__); printf x;}
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


FilerView::FilerView(BRect frame, const char* name, uint32 resizing_mode, uint32 flags)
	: BView(frame, name, resizing_mode, flags | B_FRAME_EVENTS),
	  v_paned_view_(new PanedView(BRect(0,0,0,0),"v_paned_view",
	  		PanedView::V_PANED,PanedView::PANED_FOLLOW_RIGHTBOTTOM)),
	  h_paned_view_(new PanedView(BRect(0,0,0,0),"h_paned_view",
	  		PanedView::H_PANED,PanedView::PANED_FOLLOW_KEEP_RATIO)),
	  log_view_(new LogView(BRect(0,0,0,0),"log_view")),
	  dir_view1_(new DirectoryView(BRect(0,0,0,0),"filer1")),
	  dir_view2_(new DirectoryView(BRect(0,0,0,0),"filer2"))
{
	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

	AddChild(v_paned_view_);
	v_paned_view_->SetChild1(h_paned_view_);
	v_paned_view_->SetChild2(log_view_);
	h_paned_view_->SetRightBorder(-1);
	h_paned_view_->SetChild1(dir_view1_);
	h_paned_view_->SetChild2(dir_view2_);

	//
	// set color
	//
//	dir_view1_->SetBackgroundColor (16,16,16);
//	dir_view1_->SetBackgroundColor (10,17,29);
	dir_view1_->SetBackgroundColor (45,45,45);
	dir_view1_->SetExecutableColor(255,255,255);
	dir_view1_->SetSelectedColor (100,100,100);
//	dir_view1_->SetReadonlyColor(255,255,127);
	dir_view1_->SetReadonlyColor(131,179,36);
	dir_view1_->SetHiddenColor (0,0,191);
//	dir_view1_->SetCursorColor(255,255,0);
//	dir_view1_->SetCursorColor(233,186,129);
//	dir_view1_->SetCursorColor(219,146,53);
	dir_view1_->SetCursorColor(251,251,63 );
//	dir_view1_->SetFileColor(255,255,255);
	dir_view1_->SetFileColor(200,200,200);
//	dir_view1_->SetFileColor(233,186,129);
//	dir_view1_->SetDirColor(0,255,255);
//	dir_view1_->SetDirColor(101,184,224);
	dir_view1_->SetDirColor(100,150,200);

	dir_view2_->SetBackgroundColor (dir_view1_->GetBackgroundColor());
	dir_view2_->SetExecutableColor(dir_view1_->GetExecutableColor());
	dir_view2_->SetSelectedColor (dir_view1_->GetSelectedColor());
	dir_view2_->SetReadonlyColor(dir_view1_->GetReadonlyColor());
	dir_view2_->SetHiddenColor (dir_view1_->GetHiddenColor());
	dir_view2_->SetCursorColor(dir_view1_->GetCursorColor());
	dir_view2_->SetFileColor(dir_view1_->GetFileColor());
	dir_view2_->SetDirColor(dir_view1_->GetDirColor());

	log_view_->SetBackgroundColor(dir_view1_->GetBackgroundColor());
	log_view_->SetTextColor(dir_view1_->GetFileColor());
}


FilerView::~FilerView()
{
}


void
FilerView::AttachedToWindow()
{
	v_paned_view_->ResizeTo(Bounds().Width(), Bounds().Height());
	v_paned_view_->SetSplitPosition(0.8 * v_paned_view_->Bounds().Height());
	h_paned_view_->SetSplitPosition((h_paned_view_->Bounds()).Width()/2.0);

	dir_view1_->MakeFocus();

	BView::AttachedToWindow();
}


void
FilerView::FrameResized(float width, float height)
{
	h_paned_view_->SetSplitPosition((h_paned_view_->Bounds()).Width()/2.0);
	BView::FrameResized(width, height);
}


void
FilerView::OpenDirectory1(const char* name)
{
	dir_view1_->OpenDirectory(name);
}


void
FilerView::OpenDirectory2(const char* name)
{
	dir_view2_->OpenDirectory(name);
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

	case M_FOCUS_LEFT:
		dir_view1_->MakeFocus();
		break;
	case M_FOCUS_RIGHT:
		dir_view2_->MakeFocus();
		break;

	case M_OPEN_FILE:
	case M_VIEW_FILE:
		printf("viewing\n");
		break;

	default:
		BView::MessageReceived(message);
		break;
	}
}
