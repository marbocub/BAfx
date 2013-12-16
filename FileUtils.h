/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <SupportDefs.h>

#include <Directory.h>
#include <Entry.h>
#include <List.h>
#include <SymLink.h>

class EntryList : public BList {
public:
	EntryList(BList* srcList);
	~EntryList();
private:
	void AddFileListFromEntry(BEntry* srcEntry, BList* destList);
	void GetExpandFileList(BList* srcList, BList* destList);
	void FreeExpandFileList(BList* destList);
};

class EntryStack : private BList {
public:
	EntryStack();
	~EntryStack();
	void Push(BEntry* entry);
	BEntry* Pop(void);
	bool IsEmpty(void);
private:
};

namespace FileUtils {
status_t CopyFile(BEntry* srcEntry, BDirectory* destDir, const char* destName, bool overwrite=false);
status_t CopyDirectoryPermission(BDirectory* srcDir, BDirectory* destDir);
status_t MakeDirectory(BDirectory* srcDir, BDirectory* newDir, char* name, bool overwrite=false);
status_t GetParentDirectory(BDirectory* srcDir, BDirectory* newDir);
status_t MoveEntry(BEntry* srcEntry, BDirectory* destDir);
status_t RemoveEntry(BEntry* entry);

//bool Copy(BEntry& srcEntry, BDirectory& destDir);
//bool Move(BEntry& srcEntry, BDirectory& destDir);
//bool Remove(BEntry& targetEntry);
};

#endif // FILE_UTILS_H
