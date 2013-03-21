/*
 * Copyright 2012 @marbocub <marbocub @ google mail>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef BAFX_MESSAGE_H
#define BAFX_MESSAGE_H

#include <SupportDefs.h>

enum {
	M_COPY_FILE			= 'cp__',
	M_MOVE_FILE			= 'mv__',
	M_DELETE_FILE		= 'rm__',
	M_RENAME_FILE		= 'ren_',

	M_ENTER_FILE		= 'entr',

	M_OPEN_FILE			= 'open',
	M_VIEW_FILE			= 'view',
	M_EDIT_FILE			= 'edit',
	M_EXEC_FILE			= 'exec',
	M_MARK_FILE			= 'mark',
	M_INFO_FILE			= 'info',
	M_MARK_ALL_FILE		= 'mall',

	M_CHANGE_DIRECTORY	= 'cd__',
	M_PARENT_DIRECTORY	= 'cdup',
	M_MAKE_DIRECTORY	= 'mkdr',
	M_JUMP_DIRECTORY	= 'jump',
	M_LOG_DIRECTORY		= 'drv_',

	M_FOCUS_LEFT		= 'left',
	M_FOCUS_RIGHT		= 'rght',
	M_CURSOR_UP			= 'up__',
	M_CURSOR_DOWN		= 'down',
	M_CURSOR_PAGEUP		= 'pgup',
	M_CURSOR_PAGEDOWN	= 'pgdn',
	M_CURSOR_HOME		= 'home',
	M_CURSOR_END		= 'end_',

	M_SETUP_APPLICATION	= 'stup',

	M_FILER_CHANGED		= 'chgd',
};

typedef struct {
	char	key;
	uint32	modifiers;
	uint32	message;
} BafxKeymap;

extern "C" uint32 bafx_key_to_message(char key, int32 modifiers);
extern "C" void bafx_free_keymap(void);

#endif // BAFX_MESSAGE_H
