// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2007-2007 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once

///////////////////////////////////////////////////////////////
// base class and member includes
///////////////////////////////////////////////////////////////

#include "ILogIterator.h"

#include "DictionaryBasedTempPath.h"
#include "CachedLogInfo.h"

///////////////////////////////////////////////////////////////
// begin namespace LogCache
///////////////////////////////////////////////////////////////

namespace LogCache
{

///////////////////////////////////////////////////////////////
// CLogIteratorBase
///////////////////////////////////////////////////////////////

class CLogIteratorBase : public ILogIterator
{
protected:

	// data source

	const CCachedLogInfo* logInfo;

	// just enough to describe our position
	// and what we are looking for

	revision_t revision;
	CDictionaryBasedTempPath path;

	// construction 
	// (copy construction & assignment use default methods)

	CLogIteratorBase ( const CCachedLogInfo* cachedLog
					 , revision_t startRevision
					 , const CDictionaryBasedTempPath& startPath);

	// implement copy-history following strategy

	virtual bool HandleCopyAndDelete() = 0;

	// react on cache updates

	virtual void HandleCacheUpdates();

	// do we have data for that revision?

	bool InternalDataIsMissing() const;

	// comparison methods

	static bool PathsIntersect 
		( const CDictionaryBasedPath& lhsPath
		, const CDictionaryBasedPath& rhsPath);
	static bool PathInRevision 
		( const CRevisionInfoContainer::CChangesIterator& first
		, const CRevisionInfoContainer::CChangesIterator& last
		, const CDictionaryBasedTempPath& path);

	virtual bool PathInRevision() const;

	// utilities for efficient HandleCopyAndDelete() implementation

	// any HandleCopyAndDelete() entries in that revision?

	static bool ContainsCopyOrDelete 
		( const CRevisionInfoContainer::CChangesIterator& first
		, const CRevisionInfoContainer::CChangesIterator& last);

	// return true, searchPath / searchRevision have been modified

	static bool InternalHandleCopyAndDelete 
		( const CRevisionInfoContainer::CChangesIterator& first
		, const CRevisionInfoContainer::CChangesIterator& last
		, const CDictionaryBasedPath& revisionRootPath
		, CDictionaryBasedTempPath& searchPath
		, revision_t& searchRevision);

	// log scanning sub-routines

	virtual revision_t SkipNARevisions();
	virtual void ToNextRevision();

	// log scanning

	virtual void InternalAdvance();

public:

	// destruction

	virtual ~CLogIteratorBase(void);

	// implement ILogIterator

	virtual bool DataIsMissing() const;
	virtual revision_t GetRevision() const;
	virtual const CDictionaryBasedTempPath& GetPath() const;
	virtual bool EndOfPath() const;

	virtual void Advance();

	virtual void Retry();
};

///////////////////////////////////////////////////////////////
// do we have data for that revision?
///////////////////////////////////////////////////////////////

inline bool CLogIteratorBase::InternalDataIsMissing() const
{
	return logInfo->GetRevisions()[revision] == NO_INDEX;
}

///////////////////////////////////////////////////////////////
// implement ILogIterator
///////////////////////////////////////////////////////////////

inline revision_t CLogIteratorBase::GetRevision() const
{
	return revision;
}

inline const CDictionaryBasedTempPath& CLogIteratorBase::GetPath() const
{
	return path;
}

inline bool CLogIteratorBase::EndOfPath() const
{
	return revision == NO_REVISION;
}

///////////////////////////////////////////////////////////////
// end namespace LogCache
///////////////////////////////////////////////////////////////

}

