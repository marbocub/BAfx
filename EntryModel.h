/*
 * Copyright 2012-2014 @marbocub <marbocub@gmail.com>
 * All rights reserved.
 * Distributed under the terms of the MIT license.
 */


#ifndef ENTRY_MODEL_H
#define ENTRY_MODEL_H

#include <SupportDefs.h>
#include <Directory.h>
#include <Entry.h>
#include <List.h>
#include <Path.h>
#include <String.h>

#include <Looper.h>
#include <Alert.h>
#include <Message.h>
#include <Invoker.h>


class Visitor;
class VEntry {
};


class Entry {
public:
	virtual ~Entry();
	virtual void Accept(Visitor*, void* =NULL) = 0;
	virtual status_t GetBEntry(BEntry*) const = 0;
	virtual status_t GetVEntry(VEntry*) const = 0;
	virtual status_t GetName(char*) const = 0;
	virtual status_t GetSize(off_t*) const = 0;
	virtual status_t GetModificationTime(time_t*) const = 0;
	virtual bool IsFile() const;
	virtual bool IsDirectory() const;
	virtual bool IsSymLink() const;
	virtual bool IsExecutable() const;
	virtual bool IsReadOnly() const;
	virtual bool IsHiddenFile() const;
	virtual ssize_t ReadLink(char*, size_t) const;
	virtual status_t GetPath(BString *) const;
	virtual Entry* duplicate() const;
	virtual bool operator==(const Entry&) const;
	virtual bool operator!=(const Entry&) const;
};


class File : public Entry {
private:
	BEntry entry_;
public:
	File(const BEntry&);
	File(const File&);
	~File();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsFile() const;
	Entry* duplicate() const;
	bool operator==(const Entry&) const;
};


class Directory : public Entry {
private:
	BEntry entry_;
public:
	Directory(const BEntry&);
	Directory(const Directory&);
	~Directory();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetPath(BString *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsDirectory() const;
	Entry* duplicate() const;
	bool operator==(const Entry&) const;
};


class SymLink : public Entry {
private:
	BEntry entry_;
public:
	SymLink(const BEntry&);
	SymLink(const SymLink&);
	~SymLink();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsFile() const;
	bool IsDirectory() const;
	bool IsSymLink() const;
	ssize_t ReadLink(char*, size_t) const;
	Entry* duplicate() const;
	bool operator==(const Entry&) const;
};


class VFile : public Entry {
private:
public:
	VFile();
	~VFile();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsFile() const;
	Entry* duplicate() const;
};


class VDirectory : public Entry {
private:
public:
	VDirectory();
	~VDirectory();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsDirectory() const;
	Entry* duplicate() const;
};


class VSymLink : public Entry {
private:
public:
	VSymLink();
	~VSymLink();
	void Accept(Visitor*, void* optionalData=NULL);
	status_t GetBEntry(BEntry*) const;
	status_t GetVEntry(VEntry*) const;
	status_t GetName(char *) const;
	status_t GetSize(off_t*) const;
	status_t GetModificationTime(time_t*) const;
	bool IsSymLink() const;
	Entry* duplicate() const;
};


class EntryStack : private BList {
public:
	EntryStack();
	~EntryStack();
	void Push(Entry*);
	Entry* Pop();
	bool IsEmpty();
private:
};


class EntryList : private BList {
private:
	BList	leaf_;
public:
	EntryList();
	~EntryList();
	void AddItem(Entry* entry, BString* leaf=NULL);
	void EntryList::DeleteItem(int32 index);
	void RemoveItem(int32 index);
	void EntryList::DeleteAllItems();
	void MakeEmpty();
	int32 CountItems();
	Entry* EntryAt(int32 index);
	BString* LeafAt(int32 index);
};


class Visitor {
private:
	bool	canceled_;
public:
	Visitor();
	virtual ~Visitor();
	virtual status_t Go(void) = 0;
	virtual void Visit(File*, void*) = 0;
	virtual void Visit(Directory*, void*) = 0;
	virtual void Visit(SymLink*, void*) = 0;
	virtual void Visit(VFile*, void*) = 0;
	virtual void Visit(VDirectory*, void*) = 0;
	virtual void Visit(VSymLink*, void*) = 0;
	virtual bool IsCanceled(void);
	virtual void SetCancel(bool);
	virtual void SetCancel(void);
	virtual void UnsetCancel(void);
};


class ListVisitor : public Visitor {
private:
	Entry*		const srcEntry_;
	BList*		dstList_;
	BList*		dstLeaf_;
	EntryList*	const srcEntryListPointer_;
	EntryList	dstEntryList_;
	EntryList*	const dstEntryListPointer_;
	bool		recursive_;
	bool		insertNullAsUpDir_;
public:
	ListVisitor(Entry&, BList* dstList=NULL, BList* dstLeaf=NULL,
		bool recursive=false, bool insertNullAsUpDir=false);

	ListVisitor(Entry&, EntryList* dstEntryList=NULL,
		bool recursive=false, bool insertNullAsUpDir=false);
	ListVisitor(EntryList&, EntryList* dstEntryList=NULL,
		bool recursive=false, bool insertNullAsUpDir=false);
	~ListVisitor();
	status_t Go();
	void Visit(File*, void* optionalData=NULL);
	void Visit(Directory*, void* optionalData=NULL);
	void Visit(SymLink*, void* optionalData=NULL);
	void Visit(VFile*, void* optionalData=NULL);
	void Visit(VDirectory*, void* optionalData=NULL);
	void Visit(VSymLink*, void* optionalData=NULL);
	EntryList* GetEntryList() const;
private:
	status_t Dispatch(Entry*);
	void SetupLeaf(Entry*, void*, BString*);
	void AddItemPair(Entry*, BString*);
};


class CopyVisitor : public Visitor {
private:
	Entry*	const srcEntry_;
	BList*	const srcEntryListPointer_;
	BPath	destPath_;
	BList*	const destNameListPointer_;
	BString	destPrefix_;
	EntryStack	entryStack_;
	bool	overwriteDir_;
	bool	overwriteFile_;
public:
	CopyVisitor(Entry&, char*, bool overwrite=false);
	CopyVisitor(BList&, BList&, char*, bool overwrite=false);
	~CopyVisitor();
	status_t Go(void);
	void Visit(File*, void* optionalData=NULL);
	void Visit(Directory*, void* optionalData=NULL);
	void Visit(SymLink*, void* optionalData=NULL);
	void Visit(VFile*, void* optionalData=NULL);
	void Visit(VDirectory*, void* optionalData=NULL);
	void Visit(VSymLink*, void* optionalData=NULL);
	status_t Dispatch(Entry*, BPath*);
private:
	void RemoveLastSlash(BString*);
	void RemoveFirstSlash(BString*);
	BPath* SetupDestDir(BDirectory*, void*);
	void ThrowIfFail(status_t);
	void ThrowIfNotExists(Entry*, status_t, BEntry* srcEntry=NULL);
	void ThrowIfNotExists(BDirectory*, status_t, BEntry* srcEntry=NULL);
	status_t CopyBfsAttributes(BNode&, BNode&);
	status_t CopyBfsPermission(BStatable&, BStatable&);
	char* CreateBuffer(int*);
	status_t CopyFileData(BFile&, BFile&);
};


class RemoveVisitor : public Visitor {
private:
	Entry*	const srcEntry_;
	BList*	const srcEntryListPointer_;
	EntryStack	entryStack_;
public:
	RemoveVisitor(Entry&);
	RemoveVisitor(BList&);
	virtual ~RemoveVisitor();
	virtual status_t Go(void);
	virtual void Visit(File*, void*);
	virtual void Visit(Directory*, void*);
	virtual void Visit(SymLink*, void*);
	virtual void Visit(VFile*, void*);
	virtual void Visit(VDirectory*, void*);
	virtual void Visit(VSymLink*, void*);
	status_t Dispatch(Entry*);
	status_t RemoveEntry(Entry*);
	void ThrowIfFail(status_t);
};


#endif // ENTRY_MODEL_H
