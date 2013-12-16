/*
 * Copyright 2012-2013 @marbocub <marbocub @ gmail com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "FileUtils.h"

#include <File.h>
#include <Message.h>
#include <Node.h>
#include <kernel/fs_attr.h>
#include <SymLink.h>
#include <Path.h>

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/*
 * class EntryList
 */
EntryList::EntryList(BList* srcList) : BList(0)
{
	GetExpandFileList(srcList, this);
}

EntryList::~EntryList()
{
	FreeExpandFileList(this);
}

void
EntryList::AddFileListFromEntry(BEntry* srcEntry, BList* destList)
{
	BEntry* entry = new BEntry(*srcEntry);
	destList->AddItem(entry);

	if (srcEntry->IsDirectory()) {
		BDirectory srcDir(srcEntry);
		BEntry entry;
		while(srcDir.GetNextEntry(&entry) == B_OK) {
			AddFileListFromEntry(&entry, destList);
		}
		destList->AddItem(NULL);
	}
}

void
EntryList::GetExpandFileList(BList* srcList, BList* destList)
{
	if (srcList == NULL || destList == NULL)
		return;

	BEntry *entry;
	int32 count = srcList->CountItems();

	for (int32 i=0; i<count; i++) {
		entry = (BEntry*) srcList->ItemAt(i);
		AddFileListFromEntry(entry, destList);
	}
}

void
EntryList::FreeExpandFileList(BList* destList)
{
	BEntry *entry;
	int32 count = destList->CountItems();
	for (int32 i=0; i<count; i++) {
		entry = (BEntry*) destList->ItemAt(0);
		delete entry;
		destList->RemoveItem((int32)0);
	}
	destList->MakeEmpty();
}


/*
 * class EntryStack
 */
EntryStack::EntryStack() : BList(0)
{
}

EntryStack::~EntryStack()
{
}

void
EntryStack::Push(BEntry* entry)
{
	BList::AddItem(entry);
}

BEntry*
EntryStack::Pop(void)
{
	if (IsEmpty()) {
		return NULL;
	} else {
		BEntry* entry = (BEntry*) BList::LastItem();
		BList::RemoveItem(BList::CountItems() - 1);
		return entry;
	}
}

bool
EntryStack::IsEmpty(void)
{
	BList::IsEmpty();
}


/*
 * FileUtils static functions
 */
namespace FileUtils {

status_t
CopySymlink(BEntry* srcEntry, BDirectory* destDir, const char* destName)
{
	BSymLink srcLink(srcEntry);
	BSymLink destLink;

	status_t result = B_OK;

	do {
		if (result = srcLink.InitCheck() != B_OK)
			break;

		char  linkPath[B_PATH_NAME_LENGTH+1];
		result = srcLink.ReadLink(linkPath, B_PATH_NAME_LENGTH);
		if (result < 0 && result != B_OK)
			break;

		result = destDir->CreateSymLink(destName, linkPath, &destLink);
		if (result != B_OK)
			break;

		time_t	ctime, mtime;
		uid_t	owner;
		gid_t	group;
		mode_t	perms;
	
		srcLink.GetCreationTime(&ctime);
		srcLink.GetModificationTime(&mtime);
		srcLink.GetOwner(&owner);
		srcLink.GetGroup(&group);
		srcLink.GetPermissions(&perms);
	
		destLink.SetCreationTime(ctime);
		destLink.SetModificationTime(mtime);
		if (destLink.SetOwner(owner) == B_OK) {
			destLink.SetGroup(group);
			destLink.SetPermissions(perms);
		}
	} while(0);

	return result;
}

status_t
CopyFilePermission(BFile& srcFile, BFile& destFile)
{
	time_t	ctime, mtime;
	uid_t	owner;
	gid_t	group;
	mode_t	perms;

	srcFile.GetCreationTime(&ctime);
	srcFile.GetModificationTime(&mtime);
	srcFile.GetOwner(&owner);
	srcFile.GetGroup(&group);
	srcFile.GetPermissions(&perms);

	destFile.SetCreationTime(ctime);
	destFile.SetModificationTime(mtime);
	if (destFile.SetOwner(owner) == B_OK) {
		destFile.SetGroup(group);
		destFile.SetPermissions(perms);
	}

	return B_OK;
}

char*
CreateBuffer(int* bufsize)
{
	int minbufsize =  128 * 1024;
	int maxbufsize = 1024 * 1024;
	*bufsize = maxbufsize;
	char* buffer = new char[*bufsize];
	return buffer;
}

status_t
CopyFileAttributes(BFile& srcFile, BFile& destFile)
{
	int   bufsize;
	char* buffer = CreateBuffer(&bufsize);
	char  attrName[B_ATTR_NAME_LENGTH+1];

	status_t result = B_OK;

	srcFile.RewindAttrs();

	while(srcFile.GetNextAttrName(attrName) == B_OK) {
		attr_info attrInfo;

		result = srcFile.GetAttrInfo(attrName, &attrInfo);
		if (result != B_OK) {
			break;
		}

		off_t offset = 0;
		off_t bytesLeft = attrInfo.size;

		do {
			size_t chunkSize = (size_t)bufsize;
			if ((off_t)chunkSize > bytesLeft)
				chunkSize = bytesLeft;

			ssize_t bytesRead = srcFile.ReadAttr(
				attrName, attrInfo.type, offset, buffer, chunkSize);
			if (bytesRead < 0) {
				result = (status_t) bytesRead;
				break;
			}

			ssize_t bytesWrite = destFile.WriteAttr(
				attrName, attrInfo.type, offset, buffer, (size_t)bytesRead);
			if (bytesWrite < 0) {
				result = (status_t) bytesWrite;
				break;
			}

			bytesLeft -= bytesRead;
			offset += bytesWrite;
		} while(bytesLeft > 0);

		if (result != B_OK) {
			break;
		}
	}
	
	delete [] buffer;

	return result;
}

status_t
CopyFileData(BFile& srcFile, BFile& destFile)
{
	int bufsize;
	char* buffer = CreateBuffer(&bufsize);

	status_t result = B_OK;

	while (true) {
		ssize_t bytes = srcFile.Read(buffer, bufsize);
		if (bytes > 0) {
			ssize_t result = destFile.Write(buffer, (size_t)bytes);
			if (result != bytes) {
				// write error
				result = (status_t)B_ERROR;
				break;
			}
		} else if (bytes < 0) {
			// read error
			result = (status_t)B_ERROR;
			break;
		} else {
			// bytes == 0 then copy complete
			break;
		}
	}

	delete [] buffer;

	return result;
}

status_t
CopyFile(BEntry* srcEntry, BDirectory* destDir, const char* destName, bool overwrite=false)
{
	status_t result;
	if (srcEntry->IsSymLink()) {
		return CopySymlink(srcEntry, destDir, destName);
	} else {
		do {
			BFile srcFile(srcEntry, B_READ_ONLY);
			if (result = srcFile.InitCheck() != B_OK)
				break;
	
			BFile destFile(destDir, destName, B_READ_WRITE | B_CREATE_FILE);
			if (result = destFile.InitCheck() != B_OK)
				break;
	
			if (result = CopyFileData(srcFile, destFile) != B_OK)
				break;
			if (result = CopyFilePermission(srcFile, destFile) != B_OK)
				break;
			if (result = CopyFileAttributes(srcFile, destFile) != B_OK)
				break;
		} while(0);
	}

	return result;
}

status_t
CopyDirectoryPermission(BDirectory* srcDir, BDirectory* destDir)
{
	time_t	ctime, mtime;
	uid_t	owner;
	gid_t	group;
	mode_t	perms;

	srcDir->GetCreationTime(&ctime);
	srcDir->GetModificationTime(&mtime);
	srcDir->GetOwner(&owner);
	srcDir->GetGroup(&group);
	srcDir->GetPermissions(&perms);

	destDir->SetCreationTime(ctime);
	destDir->SetModificationTime(mtime);
	if (destDir->SetOwner(owner) == B_OK) {
		destDir->SetGroup(group);
		destDir->SetPermissions(perms);
	}

	return B_OK;
}

status_t
MakeDirectory(BDirectory* srcDir, BDirectory* newDir, char* name, bool overwrite)
{
	return srcDir->CreateDirectory(name, newDir);
}

status_t
GetParentDirectory(BDirectory* srcDir, BDirectory* newDir)
{
	BEntry entry;
	srcDir->GetEntry(&entry);
	return entry.GetParent(newDir);
}

status_t
MoveEntry(BEntry* srcEntry, BDirectory* destDir)
{
	return srcEntry->MoveTo(destDir);
}

status_t
RemoveEntry(BEntry* entry)
{
	return entry->Remove();
}

};
