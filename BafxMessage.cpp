/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "BafxMessage.h"
#include <InterfaceDefs.h>

#include <malloc.h>
#include <string.h>


BafxKeymap *keymap = NULL;


const BafxKeymap default_keymap[] = {
	// key			modifiers	message

	{B_UP_ARROW,	0,			M_CURSOR_UP},
	{B_DOWN_ARROW,	0,			M_CURSOR_DOWN},
	{B_PAGE_UP,		0,			M_CURSOR_PAGEUP},
	{B_PAGE_DOWN,	0,			M_CURSOR_PAGEDOWN},
	{B_HOME,		0,			M_CURSOR_HOME},
	{B_END,			0,			M_CURSOR_END},
	{B_LEFT_ARROW,	0,			M_FOCUS_LEFT},
	{B_RIGHT_ARROW,	0,			M_FOCUS_RIGHT},

	{B_SPACE,		0,			M_MARK_FILE},
	{B_RETURN,		0,			M_ENTER_FILE},
	{B_RETURN,		B_SHIFT_KEY | B_RIGHT_SHIFT_KEY,	M_EDIT_FILE},
	{B_RETURN,		B_SHIFT_KEY | B_LEFT_SHIFT_KEY,		M_EDIT_FILE},
	{B_BACKSPACE,	0,			M_PARENT_DIRECTORY},
	{'A',			0,			M_MARK_ALL_FILE},
	{'a',			0,			M_MARK_ALL_FILE},
	{'C',			0,			M_COPY_FILE},
	{'c',			0,			M_COPY_FILE},
	{'D',			0,			M_DELETE_FILE},
	{'d',			0,			M_DELETE_FILE},
	{'E',			0,			M_EDIT_FILE},
	{'e',			0,			M_EDIT_FILE},
	{'I',			0,			M_INFO_FILE},
	{'i',			0,			M_INFO_FILE},
	{'J',			0,			M_JUMP_DIRECTORY},
	{'j',			0,			M_JUMP_DIRECTORY},
	{'K',			B_SHIFT_KEY | B_LEFT_SHIFT_KEY,		M_MAKE_DIRECTORY},
	{'k',			B_SHIFT_KEY | B_LEFT_SHIFT_KEY,		M_MAKE_DIRECTORY},
	{'K',			B_SHIFT_KEY | B_RIGHT_SHIFT_KEY,	M_MAKE_DIRECTORY},
	{'k',			B_SHIFT_KEY | B_RIGHT_SHIFT_KEY,	M_MAKE_DIRECTORY},
	{'L',			0,			M_LOG_DIRECTORY},
	{'l',			0,			M_LOG_DIRECTORY},
	{'M',			0,			M_MOVE_FILE},
	{'m',			0,			M_MOVE_FILE},
	{'R',			0,			M_RENAME_FILE},
	{'r',			0,			M_RENAME_FILE},
	{'V',			0,			M_VIEW_FILE},
	{'v',			0,			M_VIEW_FILE},
	{'X',			0,			M_EXEC_FILE},
	{'x',			0,			M_EXEC_FILE},
	{'Z',			0,			M_SETUP_APPLICATION},
	{'z',			0,			M_SETUP_APPLICATION},
//	{'',			0,			M_},
	{0,				0,			0}
};


extern "C" void bafx_setup_keymap(void)
{
	if (keymap == NULL) {
		keymap = (BafxKeymap*)malloc(sizeof(default_keymap));
		memcpy(keymap, default_keymap, sizeof(default_keymap));
	}

}


extern "C" uint32 bafx_key_to_message(char key, int32 modifiers)
{
	bafx_setup_keymap();

	modifiers &= ~(B_CAPS_LOCK | B_SCROLL_LOCK | B_NUM_LOCK);

	for (int32 i=0; keymap[i].key!=0; i++) {
		if ((key == keymap[i].key) && (modifiers == keymap[i].modifiers))
			return keymap[i].message;
	}

	return 0;
}


extern "C" void bafx_free_keymap(void)
{
	if (keymap != NULL) free(keymap);
	keymap = NULL;
}
