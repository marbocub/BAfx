/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#include "EntryModel.h"

#include <List.h>
#include <File.h>
#include <Path.h>
#include <SymLink.h>
#include <kernel/fs_attr.h>
#include <TextView.h>
#include <String.h>

#include <cstdio>


#undef DEBUG

#ifdef DEBUG
# include <cstdio>
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/*
 * class EntryStack
 */
EntryStack::EntryStack()
	: BList(0)
{
}


EntryStack::~EntryStack()
{
}


void
EntryStack::Push(Entry* entry)
{
	BList::AddItem(entry);
}


Entry*
EntryStack::Pop(void)
{
	if (IsEmpty()) {
		return NULL;
	} else {
		Entry* entry = (Entry*) BList::LastItem();
		BList::RemoveItem(BList::CountItems() - 1);
		return entry;
	}
}


bool
EntryStack::IsEmpty(void)
{
	return BList::IsEmpty();
}


/*
 * class Entry
 */
Entry::~Entry()
{
}


status_t
Entry::GetPath(BString* str) const
{
	*str = "";
	return B_ERROR;
}


bool
Entry::IsFile() const
{
	return false;
}


bool
Entry::IsDirectory() const
{
	return false;
}


bool
Entry::IsSymLink() const
{
	return false;
}


bool
Entry::IsExecutable() const
{
	return false;
}


bool
Entry::IsReadOnly() const
{
	return false;
}


bool
Entry::IsHiddenFile() const
{
	return false;
}


ssize_t
Entry::ReadLink(char* buf, size_t length) const
{
	buf[0] = '\0';
	return B_ERROR;
}

Entry*
Entry::duplicate() const
{
	return NULL;
}

bool
Entry::operator==(const Entry& item) const
{
	return false;
}


bool
Entry::operator!=(const Entry& item) const
{
	return !(*this==item);
}


/*
 * class File
 */
File::File(const BEntry& entry)
	: entry_(entry)
{
}


File::File(const File& file)
	: entry_()
{
	BEntry entry;
	file.GetBEntry(&entry);

	entry_ref ref;
	entry.GetRef(&ref);
	entry_.SetTo(&ref);
}


File::~File()
{
}


void
File::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
File::GetBEntry(BEntry* entry) const
{
	*entry = entry_;
	return entry_.InitCheck();
}


status_t
File::GetVEntry(VEntry* entry) const
{
	return B_ERROR;
}


status_t
File::GetName(char* name) const
{
	return entry_.GetName(name);
}


status_t
File::GetSize(off_t* size) const
{
	return entry_.GetSize(size);
}


status_t
File::GetModificationTime(time_t* mtime) const
{
	return entry_.GetModificationTime(mtime);
}


bool
File::IsFile() const
{
	return true;
}


Entry*
File::duplicate() const
{
	return new File(entry_);
}


bool
File::operator==(const Entry& item) const
{
	BEntry entry2;

	if (item.GetBEntry(&entry2) != B_OK) {
		return false;
	}

	return (entry_==entry2);
}


/*
 * class Directory
 */
Directory::Directory(const BEntry& entry)
	: entry_(entry)
{
}


Directory::Directory(const Directory& dir)
	: entry_()
{
	BEntry entry;
	dir.GetBEntry(&entry);

	entry_ref ref;
	entry.GetRef(&ref);
	entry_.SetTo(&ref);
}


Directory::~Directory()
{
}


void
Directory::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
Directory::GetBEntry(BEntry* entry) const
{
	*entry = entry_;
	return entry_.InitCheck();
}


status_t
Directory::GetVEntry(VEntry* entry) const
{
	return B_ERROR;
}


status_t
Directory::GetName(char *name) const
{
	return entry_.GetName(name);
}


status_t
Directory::GetSize(off_t* size) const
{
	return entry_.GetSize(size);
}


status_t
Directory::GetModificationTime(time_t* mtime) const
{
	return entry_.GetModificationTime(mtime);
}


bool
Directory::IsDirectory() const
{
	return true;
}


status_t
Directory::GetPath(BString* str) const
{
	status_t retval;

	*str = "";

	BPath path;
	if ((retval = entry_.GetPath(&path)) != B_OK)
		return retval;
	*str = path.Path();

	return B_OK;
}


Entry*
Directory::duplicate() const
{
	return new Directory(entry_);
}


bool
Directory::operator==(const Entry& item) const
{
	BEntry entry2;

	if (item.GetBEntry(&entry2) != B_OK) {
		return false;
	}

	return (entry_==entry2);
}


/*
 * class SymLink
 */
SymLink::SymLink(const BEntry& entry)
	: entry_(entry)
{
}


SymLink::SymLink(const SymLink& link)
	: entry_()
{
	BEntry entry;
	link.GetBEntry(&entry);

	entry_ref ref;
	entry.GetRef(&ref);
	entry_.SetTo(&ref);
}


SymLink::~SymLink()
{
}


void
SymLink::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
SymLink::GetBEntry(BEntry* entry) const
{
	*entry = entry_;
	return entry_.InitCheck();
}


status_t
SymLink::GetVEntry(VEntry* entry) const
{
	return B_ERROR;
}


status_t
SymLink::GetName(char *name) const
{
	return entry_.GetName(name);
}


status_t
SymLink::GetSize(off_t* size) const
{
	return entry_.GetSize(size);
}


status_t
SymLink::GetModificationTime(time_t* mtime) const
{
	return entry_.GetModificationTime(mtime);
}


bool
SymLink::IsFile() const
{
	entry_ref ref;
	entry_.GetRef(&ref);
	BEntry linked(&ref, true);

	return linked.IsFile();
}


bool
SymLink::IsDirectory() const
{
	entry_ref ref;
	entry_.GetRef(&ref);
	BEntry linked(&ref, true);

	return linked.IsDirectory();
}


bool
SymLink::IsSymLink() const
{
	return true;
}


ssize_t
SymLink::ReadLink(char* buf, size_t length) const
{
	BSymLink link(&entry_);
	return link.ReadLink(buf, length);
}


Entry*
SymLink::duplicate() const
{
	return new SymLink(entry_);
}


bool
SymLink::operator==(const Entry& item) const
{
	BEntry entry2;

	if (item.GetBEntry(&entry2) != B_OK) {
		return false;
	}

	return (entry_==entry2);
}


/*
 * class VFile
 */
VFile::VFile()
{
	// not implement yet
}


VFile::~VFile()
{
}


void
VFile::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
VFile::GetBEntry(BEntry* entry) const
{
	return B_ERROR;
}


status_t
VFile::GetVEntry(VEntry* entry) const
{
	// not implement yet
	return B_ERROR;
}


status_t
VFile::GetName(char *name) const
{
	name[0] = '\0';
	return B_ERROR;
}


status_t
VFile::GetSize(off_t* size) const
{
	*size = 0;
	return B_ERROR;
}


status_t
VFile::GetModificationTime(time_t* mtime) const
{
	*mtime = 0;
	return B_ERROR;
}


bool
VFile::IsFile() const
{
	return true;
}


Entry*
VFile::duplicate() const
{
	// not implement yet
	return new VFile();
}


/*
 * class VDirectory
 */
VDirectory::VDirectory()
{
	// not implement yet
}


VDirectory::~VDirectory()
{
}


void
VDirectory::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
VDirectory::GetBEntry(BEntry* entry) const
{
	return B_ERROR;
}


status_t
VDirectory::GetVEntry(VEntry* entry) const
{
	// not implement yet
	return B_ERROR;
}


status_t
VDirectory::GetName(char *name) const
{
	name[0] = '\0';
	return B_ERROR;
}


status_t
VDirectory::GetSize(off_t* size) const
{
	*size = 0;
	return B_ERROR;
}


status_t
VDirectory::GetModificationTime(time_t* mtime) const
{
	*mtime = 0;
	return B_ERROR;
}


bool
VDirectory::IsDirectory() const
{
	return true;
}


Entry*
VDirectory::duplicate() const
{
	// not implement yet
	return new VDirectory();
}


/*
 * class VSymLink
 */
VSymLink::VSymLink()
{
	// not implement yet
}


VSymLink::~VSymLink()
{
}


void
VSymLink::Accept(Visitor* v, void* optionalData)
{
	v->Visit(this, optionalData);
}


status_t
VSymLink::GetBEntry(BEntry* entry) const
{
	return B_ERROR;
}


status_t
VSymLink::GetVEntry(VEntry* entry) const
{
	// not implement yet
	return B_ERROR;
}


status_t
VSymLink::GetName(char *name) const
{
	name[0] = '\0';
	return B_ERROR;
}


status_t
VSymLink::GetSize(off_t* size) const
{
	*size = 0;
	return B_ERROR;
}


status_t
VSymLink::GetModificationTime(time_t* mtime) const
{
	*mtime = 0;
	return B_ERROR;
}


bool
VSymLink::IsSymLink() const
{
	return true;
}


Entry*
VSymLink::duplicate() const
{
	// not implement yet
	return new VSymLink();
}


/*
 * class EntryList
 */
EntryList::EntryList()
	: BList(0),
	  leaf_(0)
{
}


EntryList::~EntryList()
{
	this->MakeEmpty();
}


void
EntryList::AddItem(Entry* entry, BString* leaf)
{
	BList::AddItem(entry);
	leaf_.AddItem(leaf);
}


void
EntryList::DeleteItem(int32 index)
{
	Entry* entry = (Entry*) BList::ItemAt(index);
	BString* leaf = (BString*) leaf_.ItemAt(index);

	if (entry)
		delete entry;
	if (leaf)
		delete leaf;

	this->RemoveItem(index);
}


void
EntryList::RemoveItem(int32 index)
{
	BList::RemoveItem(index);
	leaf_.RemoveItem(index);
}


void
EntryList::DeleteAllItems()
{
	int32 count;

	count = CountItems();
	for (int32 i=0; i<count; i++) {
		Entry* entry = (Entry*) BList::ItemAt(i);
		BString* leaf = (BString*) leaf_.ItemAt(i);
		if (entry)
			delete entry;
		if (leaf)
			delete leaf;
	}
	this->MakeEmpty();
}


void
EntryList::MakeEmpty()
{
	BList::MakeEmpty();
	leaf_.MakeEmpty();
}


int32
EntryList::CountItems()
{
	return BList::CountItems();
}


Entry*
EntryList::EntryAt(int32 index)
{
	return (Entry*) BList::ItemAt(index);
}


BString*
EntryList::LeafAt(int32 index)
{
	return (BString*) leaf_.ItemAt(index);
}


/*
 * class Visitor
 */
Visitor::Visitor()
	: canceled_(false)
{
}


Visitor::~Visitor()
{
}


bool
Visitor::IsCanceled()
{
	return canceled_;
}


void
Visitor::SetCancel(bool cancel)
{
	canceled_ = cancel;
}


void
Visitor::SetCancel()
{
	canceled_ = true;
}


void
Visitor::UnsetCancel()
{
	canceled_ = false;
}


/*
 * class ListVisitor
 */
ListVisitor::ListVisitor(Entry& entry, BList* dstList, BList* dstLeaf,
		bool recursive, bool insertNullAsUpDir)
	: srcEntry_(&entry),
	  srcEntryListPointer_(NULL),
	  dstList_(dstList),
	  dstLeaf_(dstLeaf),
	  dstEntryList_(),
	  dstEntryListPointer_(NULL),
	  recursive_(recursive),
	  insertNullAsUpDir_(insertNullAsUpDir)
{
}


ListVisitor::ListVisitor(Entry& entry, EntryList* dstEntryList,
		bool recursive, bool insertNullAsUpDir)
	: srcEntry_(&entry),
	  srcEntryListPointer_(NULL),
	  dstList_(NULL),
	  dstLeaf_(NULL),
	  dstEntryList_(),
	  dstEntryListPointer_(dstEntryList ? dstEntryList : &dstEntryList_),
	  recursive_(recursive),
	  insertNullAsUpDir_(insertNullAsUpDir)
{
}


ListVisitor::ListVisitor(EntryList& srcEntryList, EntryList* dstEntryList,
		bool recursive, bool insertNullAsUpDir)
	: srcEntry_(NULL),
	  srcEntryListPointer_(&srcEntryList),
	  dstList_(NULL),
	  dstLeaf_(NULL),
	  dstEntryList_(),
	  dstEntryListPointer_(dstEntryList ? dstEntryList : &dstEntryList_),
	  recursive_(recursive),
	  insertNullAsUpDir_(insertNullAsUpDir)
{
}


ListVisitor::~ListVisitor()
{
	dstEntryList_.DeleteAllItems();
}


status_t
ListVisitor::Dispatch(Entry* ent)
{
	try {
		BString leaf("");
		ent->Accept(this, &leaf);
	} catch(status_t retcode) {
		return retcode;
	}
	return B_OK;
}


status_t
ListVisitor::Go(void)
{
	status_t retcode = B_OK;

	if (srcEntry_) {
		retcode = Dispatch(srcEntry_);
		if (retcode != B_OK)
			return retcode;
	}

	if (srcEntryListPointer_) {
		int32 count=srcEntryListPointer_->CountItems();
		for (int32 i=0; i<count; i++) {
			if (IsCanceled()) {
				retcode = B_CANCEL;
				break;
			}

			Entry* ent = (Entry*) srcEntryListPointer_->EntryAt(i);
			retcode = Dispatch(ent);
			if (retcode != B_OK)
				break;
		}
	}

	return retcode;
}


void
ListVisitor::SetupLeaf(Entry* ent, void* optionalData, BString* leaf)
{
	char name[B_FILE_NAME_LENGTH];
	ent->GetName(name);

	leaf->SetTo(*(BString*)optionalData);
	if (leaf->Length() > 0)
		*leaf += "/";
	*leaf += name;
}


void
ListVisitor::AddItemPair(Entry* ent, BString* str)
{
	if (dstEntryListPointer_) {
		dstEntryListPointer_->AddItem(ent, str);
	} else {
		if (dstList_)
			dstList_->AddItem(ent);
		else
			delete ent;
		if (dstLeaf_)
			dstLeaf_->AddItem(str);
		else
			delete str;
	}
//	entryListPointer_->AddItem(ent);
//	leafListPointer_->AddItem(str);
}


void
ListVisitor::Visit(File* file, void* optionalData)
{
	BString leaf;
	SetupLeaf(file, optionalData, &leaf);

	AddItemPair(new File(*file), new BString(leaf));
}


void
ListVisitor::Visit(Directory* dir, void* optionalData)
{
	BString leaf;
	SetupLeaf(dir, optionalData, &leaf);

	if (recursive_)
		AddItemPair(new Directory(*dir), new BString(leaf));

	BEntry entry;
	dir->GetBEntry(&entry);
	BDirectory bDir(&entry);
	status_t retcode;

	while ((retcode = bDir.GetNextEntry(&entry)) == B_OK) {
		if (IsCanceled())
			break;

		Entry* ent;
		if (entry.IsSymLink())
			ent = new SymLink(entry);
		else if (entry.IsDirectory())
			ent = new Directory(entry);
		else
			ent = new File(entry);

		if (recursive_) {
			ent->Accept(this, &leaf);
			delete ent;
		} else {
			char name[B_FILE_NAME_LENGTH];
			ent->GetName(name);
			AddItemPair(ent, new BString(name));
		}
	}

	if (retcode != B_ENTRY_NOT_FOUND) {
		if (retcode == B_OK)
			retcode = B_CANCEL;
		throw retcode;
	}

	if (insertNullAsUpDir_)
		AddItemPair(NULL, new BString(leaf));
}


void
ListVisitor::Visit(SymLink* link, void* optionalData)
{
	BString leaf;
	SetupLeaf(link, optionalData, &leaf);

	AddItemPair(new SymLink(*link), new BString(leaf));
}


void
ListVisitor::Visit(VFile* vfile, void* optionalData)
{
	// not implement yet
}


void
ListVisitor::Visit(VDirectory* vdir, void* optionalData)
{
	// not implement yet
}


void
ListVisitor::Visit(VSymLink* vlink, void* optionalData)
{
	// not implement yet
}


EntryList*
ListVisitor::GetEntryList() const
{
	return dstEntryListPointer_;
}


/*
 * class CopyVisitor
 */
CopyVisitor::CopyVisitor(Entry& srcEntry, char* destName, bool overwrite)
	: srcEntry_(&srcEntry),
	  srcEntryListPointer_(NULL),
	  destPath_(destName),
	  destNameListPointer_(NULL),
	  destPrefix_(""),
	  overwriteDir_(overwrite),
	  overwriteFile_(overwrite)
{
}


CopyVisitor::CopyVisitor(BList& srcEntryList, BList& destNameList,
		char* destPrefix, bool overwrite)
	: srcEntry_(NULL),
	  srcEntryListPointer_(&srcEntryList),
	  destPath_("."),
	  destNameListPointer_(&destNameList),
	  destPrefix_(destPrefix),
	  overwriteDir_(overwrite),
	  overwriteFile_(overwrite)
{
}


CopyVisitor::~CopyVisitor()
{
}


void
CopyVisitor::RemoveLastSlash(BString* str)
{
	while(str->Length() &&
		 (str->Length()-1 == str->FindLast('/')))
		str->RemoveLast("/");
}


void
CopyVisitor::RemoveFirstSlash(BString* str)
{
	while(str->FindFirst('/') == 0)
		str->RemoveFirst("/");
}


status_t
CopyVisitor::Dispatch(Entry* ent, BPath* path)
{
	try {
		ent->Accept(this, path);
	} catch(status_t retcode) {
		return retcode;
	}
	return B_OK;
}


status_t
CopyVisitor::Go(void)
{
	status_t retcode = B_OK;

	if (srcEntry_) {
		retcode = Dispatch(srcEntry_, &destPath_);
		if (retcode != B_OK)
			return retcode;
	}

	if (srcEntryListPointer_) {
		int32 count=srcEntryListPointer_->CountItems();
		for (int32 i=0; i<count; i++) {
			if (IsCanceled()) {
				retcode = B_CANCEL;
				break;
			}

			BPath path(destPrefix_.String());
			BString* str = (BString*) destNameListPointer_->ItemAt(i);
			if (str) {
				RemoveFirstSlash(str);
				RemoveLastSlash(str);
				path.Append(str->String());
			}

			Entry* ent = (Entry*) srcEntryListPointer_->ItemAt(i);
			if (ent) {
				if (path.InitCheck() == B_OK) {
					retcode = Dispatch(ent, &path);
					if (retcode != B_OK)
						break;
				}

			} else {
				// NULL indicates directory list has been completed.
				BEntry srcEntry;
				Directory* dir = (Directory*) entryStack_.Pop();
				dir->GetBEntry(&srcEntry);

				BDirectory srcDir(&srcEntry), destDir(path.Path());

				CopyBfsAttributes(srcDir, destDir);
				CopyBfsPermission(srcDir, destDir);
			}
		}
	}

	return retcode;
}


void
CopyVisitor::ThrowIfFail(status_t retcode)
{
	if (retcode != B_OK)
		throw retcode;
}


void
CopyVisitor::ThrowIfNotExists(Entry* ent, status_t throwCode, BEntry* srcEntry)
{
	BEntry entry;
	ent->GetBEntry(&entry);
	if (!entry.Exists())
		throw throwCode;
	if (srcEntry)
		*srcEntry = entry;
}


void
CopyVisitor::ThrowIfNotExists(BDirectory* dir, status_t throwCode, BEntry* srcEntry)
{
	BEntry entry;
	dir->GetEntry(&entry);
	if (!entry.Exists())
		throw throwCode;
	if (srcEntry)
		*srcEntry = entry;
}


BPath*
CopyVisitor::SetupDestDir(BDirectory* dir, void* optionalData)
{
	BPath* path = (BPath*) optionalData;
	BPath parent;
	path->GetParent(&parent);
	dir->SetTo(parent.Path());

	ThrowIfFail(dir->InitCheck());
	ThrowIfNotExists(dir, B_ENTRY_NOT_FOUND);

	return path;
}


status_t
CopyVisitor::CopyBfsAttributes(BNode& srcNode, BNode& destNode)
{
	int   bufsize;
	char* buffer = CreateBuffer(&bufsize);
	char  attrName[B_ATTR_NAME_LENGTH+1];

	status_t result = B_OK;

	srcNode.RewindAttrs();

	while(srcNode.GetNextAttrName(attrName) == B_OK) {
		attr_info attrInfo;

		result = srcNode.GetAttrInfo(attrName, &attrInfo);
		if (result != B_OK) {
			break;
		}

		off_t offset = 0;
		off_t bytesLeft = attrInfo.size;

		do {
			size_t chunkSize = (size_t)bufsize;
			if ((off_t)chunkSize > bytesLeft)
				chunkSize = bytesLeft;

			ssize_t bytesRead = srcNode.ReadAttr(attrName, attrInfo.type, offset, buffer, chunkSize);
			if (bytesRead < 0) {
				result = (status_t) bytesRead;
				break;
			}

			ssize_t bytesWrite = destNode.WriteAttr(attrName, attrInfo.type, offset, buffer, (size_t)bytesRead);
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
CopyVisitor::CopyBfsPermission(BStatable& srcStat, BStatable& destStat)
{
	time_t	ctime, mtime;
	uid_t	owner;
	gid_t	group;
	mode_t	perms;

	srcStat.GetCreationTime(&ctime);
	srcStat.GetModificationTime(&mtime);
	srcStat.GetOwner(&owner);
	srcStat.GetGroup(&group);
	srcStat.GetPermissions(&perms);

	destStat.SetCreationTime(ctime);
	destStat.SetModificationTime(mtime);
	if (destStat.SetOwner(owner) == B_OK) {
		destStat.SetGroup(group);
		destStat.SetPermissions(perms);
	}

	return B_OK;
}


char*
CopyVisitor::CreateBuffer(int* bufsize)
{
//	int minbufsize =  128 * 1024;
	int maxbufsize = 1024 * 1024;
	*bufsize = maxbufsize;
	char* buffer = new char[*bufsize];
	return buffer;
}


status_t
CopyVisitor::CopyFileData(BFile& srcFile, BFile& destFile)
{
	int bufsize;
	char* buffer = CreateBuffer(&bufsize);

	status_t result = B_OK;

	while (true) {
		if (IsCanceled()) {
			result = B_CANCEL;
			break;
		}

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


void
CopyVisitor::Visit(File* file, void* optionalData)
{
	BDirectory destDir;
	BPath* path = SetupDestDir(&destDir, optionalData);

	BEntry srcEntry;
	ThrowIfNotExists(file, B_FILE_NOT_FOUND, &srcEntry);

	BFile srcFile, destFile;
	ThrowIfFail(destDir.CreateFile(path->Leaf(), &destFile, !overwriteFile_));
	ThrowIfFail(srcFile.SetTo(&srcEntry, B_READ_ONLY));

	ThrowIfFail(CopyFileData(srcFile, destFile));
	ThrowIfFail(CopyBfsAttributes(srcFile, destFile));
	ThrowIfFail(CopyBfsPermission(srcFile, destFile));
}


void
CopyVisitor::Visit(Directory* dir, void* optionalData)
{
	BDirectory destDir;
	BPath* path = SetupDestDir(&destDir, optionalData);

	status_t retcode = destDir.CreateDirectory(path->Leaf(), NULL);
	if (retcode == B_FILE_EXISTS && overwriteDir_)
		retcode = B_OK;

	ThrowIfFail(retcode);

	entryStack_.Push(dir);
}


void
CopyVisitor::Visit(SymLink* link, void* optionalData)
{
	BDirectory destDir;
	BPath* path = SetupDestDir(&destDir, optionalData);

	BEntry srcEntry;
	link->GetBEntry(&srcEntry);
	BSymLink srcLink(&srcEntry);
	char  linkPath[B_PATH_NAME_LENGTH+1];
	ThrowIfFail(srcLink.InitCheck());

	status_t retcode = srcLink.ReadLink(linkPath, B_PATH_NAME_LENGTH);
	if (retcode < 0) ThrowIfFail(retcode);

	BSymLink destLink;
	ThrowIfFail(destDir.CreateSymLink(path->Leaf(), linkPath, &destLink));
	ThrowIfFail(CopyBfsAttributes(srcLink, destLink));
	ThrowIfFail(CopyBfsPermission(srcLink, destLink));
}

void
CopyVisitor::Visit(VFile* vfile, void* optionalData)
{
	// not implement yet
}


void
CopyVisitor::Visit(VDirectory* vdir, void* optionalData)
{
	// not implement yet
}


void
CopyVisitor::Visit(VSymLink* vlink, void* optionalData)
{
	// not implement yet
}


/*
 * class RemoveVisitor
 */
RemoveVisitor::RemoveVisitor(Entry& srcEntry)
	: srcEntry_(&srcEntry),
	  srcEntryListPointer_(NULL)
{
}


RemoveVisitor::RemoveVisitor(BList& srcEntryList)
	: srcEntry_(NULL),
	  srcEntryListPointer_(&srcEntryList)
{
}


RemoveVisitor::~RemoveVisitor()
{
}


status_t
RemoveVisitor::Dispatch(Entry* ent)
{
	try {
		ent->Accept(this);
	} catch(status_t retcode) {
		return retcode;
	}
	return B_OK;
}


status_t
RemoveVisitor::RemoveEntry(Entry* ent)
{
	BEntry entry;
	ent->GetBEntry(&entry);
	return entry.Remove();
}


status_t
RemoveVisitor::Go()
{
	status_t retcode = B_OK;

	if (srcEntry_) {
		retcode = Dispatch(srcEntry_);
		if (retcode != B_OK)
			return retcode;
	}

	if (srcEntryListPointer_) {
		int32 count=srcEntryListPointer_->CountItems();
		for (int32 i=0; i<count; i++) {
			if (IsCanceled()) {
				retcode = B_CANCEL;
				break;
			}

			Entry* ent = (Entry*) srcEntryListPointer_->ItemAt(i);
			if (ent) {
				retcode = Dispatch(ent);
			} else {
				// NULL indicates remove directory entry has been completed.
				// Finally, remove the directory itself.
				retcode = RemoveEntry((Directory*) entryStack_.Pop());
			}
			if (retcode != B_OK)
				break;
		}
	}

	return retcode;
}


void
RemoveVisitor::ThrowIfFail(status_t retcode)
{
	if (retcode != B_OK)
		throw retcode;
}


void
RemoveVisitor::Visit(File* file, void* optionalData)
{
	ThrowIfFail(RemoveEntry(file));
}


void
RemoveVisitor::Visit(Directory* dir, void* optionalData)
{
	entryStack_.Push(dir);
}


void
RemoveVisitor::Visit(SymLink* link, void* optionalData)
{
	ThrowIfFail(RemoveEntry(link));
}


void
RemoveVisitor::Visit(VFile* vfile, void* optionalData)
{
	// not implement yet
}


void
RemoveVisitor::Visit(VDirectory* vdir, void* optionalData)
{
	// not implement yet
}


void
RemoveVisitor::Visit(VSymLink* vlink, void* optionalData)
{
	// not implement yet
}

