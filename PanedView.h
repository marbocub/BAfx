/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef PANED_VIEW_H
#define PANED_VIEW_H

#include <SupportDefs.h>
#include <View.h>


class PanedView : public BView
{
private:
	BView*		mView1;
	BView*		mView2;
	BView*		mChild1;
	BView*		mChild2;
//	float		mSplitPosition;
	float		mPositionLeftTop;
	float		mPositionRightBottom;
	float		mTopBorder;
	float		mBottomBorder;
	float		mLeftBorder;
	float		mRightBorder;
	int			mOrientation;
	bool		mDragMode;
	BPoint		mDragStartPoint;
	uint32		mPanedFollowMode;

public:
	enum {
		SEPARATION_SIZE = 4
	};
	enum {
		H_PANED = 0,
		V_PANED
	};
	enum {
		PANED_FOLLOW_LEFTTOP = 1,
		PANED_FOLLOW_RIGHTBOTTOM = 2,
		PANED_FOLLOW_KEEP_RATIO = 3,
		PANED_FOLLOW_HALF = 4,
	};

	PanedView(BRect frame, const char* name, uint32 orientation,
		uint32 pane_follow_mode = PANED_FOLLOW_KEEP_RATIO,
		uint32 resizingMode = B_FOLLOW_ALL_SIDES,
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	~PanedView();

	void AttachedToWindow();
	void SetViewColor(rgb_color color);
	void SetHighColor(rgb_color color);
	void SetLowColor(rgb_color color);
	void FrameResized(float width, float height);
	void MouseDown(BPoint point);
	void MouseMoved(BPoint point, uint32 transit, const BMessage* message);
	void MouseUp(BPoint point);

	void SetBorderWidth(float border);
	void SetRightBorder(float border);
	void SetSplitPosition(float s);
	BRect Bounds1(void);
	BRect Bounds2(void);
	BView* GetChild1(void);
	BView* GetChild2(void);
	void SetChild1(BView* child);
	void SetChild2(BView* child);
};

#endif // PANED_VIEW_H
