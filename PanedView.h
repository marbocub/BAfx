/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef PANED_VIEW_H
#define PANED_VIEW_H

#include <SupportDefs.h>
#include <View.h>

class PanedView : public BView
{
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
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint point);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage* message);
	virtual void MouseUp(BPoint point);
	virtual void FrameResized(float width, float height);

	void SetBorderWidth(float border);
	void SetRightBorder(float border);
	void SetSplitPosition(float s);
	BRect Bounds1(void);
	BRect Bounds2(void);
	BView* GetChild1(void);
	BView* GetChild2(void);
	void SetChild1(BView* child);
	void SetChild2(BView* child);

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
};

#endif // PANED_VIEW_H
