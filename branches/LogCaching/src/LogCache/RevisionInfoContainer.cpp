#include "StdAfx.h"
#include ".\revisioninfocontainer.h"

#include "PackedDWORDInStream.h"
#include "PackedDWORDOutStream.h"
#include "DiffIntegerInStream.h"
#include "DiffIntegerOutStream.h"

// construction / destruction

CRevisionInfoContainer::CRevisionInfoContainer(void)
{
	// [lastIndex+1] must point to [size()]

	changesOffsets.push_back(0);
	copyFromOffsets.push_back(0);
}

CRevisionInfoContainer::~CRevisionInfoContainer(void)
{
}

// add information
// AddChange() always adds to the last revision

size_t CRevisionInfoContainer::Insert ( const std::string& author
									  , const std::string& comment
									  , DWORD timeStamp)
{
	// this should newer throw as there are usually more
	// changes than revisions. But you never know ...

	if (authors.size() == (DWORD)(-1))
		throw std::exception ("revision container overflow");

	// store the given revision info

	authors.push_back ((DWORD)authorPool.AutoInsert (author.c_str()));
	timeStamps.push_back (timeStamp);
	comments.Insert (comment);

	// no changes yet -> no common root path info

	rootPaths.push_back (-1);

	// empty range for changes 

	changesOffsets.push_back (*changesOffsets.rbegin());
	copyFromOffsets.push_back (*copyFromOffsets.rbegin());

	// ready

	return size()-1;
}

void CRevisionInfoContainer::AddChange ( TChangeAction action
									   , const std::string& path
									   , const std::string& fromPath
									   , DWORD fromRevision)
{
	// under x64, there might actually be an overflow

	if (changes.size() == (DWORD)(-1))
		throw std::exception ("revision container change list overflow");

	// another change

	++(*changesOffsets.rbegin());

	// add change path index and update the root path 
	// of all changes in this revision

	CDictionaryBasedPath parsedPath (&paths, path);
	changedPaths.push_back ((DWORD)parsedPath.GetIndex());

	DWORD& rootPathIndex = *rootPaths.rbegin();
	rootPathIndex = rootPathIndex == -1
				  ? (DWORD)parsedPath.GetIndex()
				  : (DWORD)parsedPath.GetCommonRoot (rootPathIndex).GetIndex();

	// add changes info (flags), and indicate presence of fromPath (if given)

	if (fromPath.empty())
	{
		changes.push_back ((char)action);
	}
	else
	{
		changes.push_back ((char)action + 1);

		// add fromPath info

		CDictionaryBasedPath parsedFromPath (&paths, fromPath);
		copyFromPaths.push_back ((DWORD)parsedFromPath.GetIndex());
		copyFromRevisions.push_back (fromRevision);

		++(*copyFromOffsets.rbegin());
	}
}

// reset content

void CRevisionInfoContainer::Clear()
{
	authorPool.Clear();
	paths.Clear();
	comments.Clear();

	authors.clear();
	timeStamps.clear();

	rootPaths.clear();

	changesOffsets.erase (changesOffsets.begin()+1, changesOffsets.end());
	copyFromOffsets.erase (copyFromOffsets.begin()+1, copyFromOffsets.end());

	changes.clear();
	changedPaths.clear();
	copyFromPaths.clear();
	copyFromRevisions.clear();

}

// stream I/O

IHierarchicalInStream& operator>> ( IHierarchicalInStream& stream
								  , CRevisionInfoContainer& container)
{
	// read the pools

	IHierarchicalInStream* authorPoolStream
		= stream.GetSubStream (CRevisionInfoContainer::AUTHOR_POOL_STREAM_ID);
	*authorPoolStream >> container.authorPool;

	IHierarchicalInStream* commentsStream
		= stream.GetSubStream (CRevisionInfoContainer::COMMENTS_STREAM_ID);
	*commentsStream >> container.comments;

	IHierarchicalInStream* pathsStream
		= stream.GetSubStream (CRevisionInfoContainer::PATHS_STREAM_ID);
	*pathsStream >> container.paths;

	// read the revision info

	CDiffIntegerInStream* authorsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::AUTHORS_STREAM_ID));
	*authorsStream >> container.authors;

	CDiffIntegerInStream* timeStampsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::TIMESTAMPS_STREAM_ID));
	*timeStampsStream >> container.timeStamps;

	CPackedDWORDInStream* rootPathsStream 
		= dynamic_cast<CPackedDWORDInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::ROOTPATHS_STREAM_ID));
	*rootPathsStream >> container.rootPaths;

	CDiffIntegerInStream* changesOffsetsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::CHANGES_OFFSETS_STREAM_ID));
	*changesOffsetsStream >> container.changesOffsets;

	CDiffIntegerInStream* copyFromOffsetsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::COPYFROM_OFFSETS_STREAM_ID));
	*copyFromOffsetsStream >> container.copyFromOffsets;

	// read the changes info

	CPackedDWORDInStream* changesStream 
		= dynamic_cast<CPackedDWORDInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::CHANGES_STREAM_ID));
	*changesStream >> container.changes;

	CDiffIntegerInStream* changedPathsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::CHANGED_PATHS_STREAM_ID));
	*changedPathsStream >> container.changedPaths;

	CDiffIntegerInStream* copyFromPathsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::COPYFROM_PATHS_STREAM_ID));
	*copyFromPathsStream >> container.copyFromPaths;

	CDiffIntegerInStream* copyFromRevisionsStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CRevisionInfoContainer::COPYFROM_REVISIONS_STREAM_ID));
	*copyFromRevisionsStream >> container.copyFromRevisions;

	// ready

	return stream;
}

IHierarchicalOutStream& operator<< ( IHierarchicalOutStream& stream
								   , const CRevisionInfoContainer& container)
{
	// write the pools

	IHierarchicalOutStream* authorPoolStream
		= stream.OpenSubStream ( CRevisionInfoContainer::AUTHOR_POOL_STREAM_ID
							   , COMPOSITE_STREAM_TYPE_ID);
	*authorPoolStream << container.authorPool;

	IHierarchicalOutStream* commentsStream
		= stream.OpenSubStream ( CRevisionInfoContainer::COMMENTS_STREAM_ID
							   , COMPOSITE_STREAM_TYPE_ID);
	*commentsStream << container.comments;

	IHierarchicalOutStream* pathsStream
		= stream.OpenSubStream ( CRevisionInfoContainer::PATHS_STREAM_ID
							   , COMPOSITE_STREAM_TYPE_ID);
	*pathsStream << container.paths;

	// write the revision info

	CDiffIntegerOutStream* authorsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::AUTHORS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*authorsStream << container.authors;

	CDiffIntegerOutStream* timeStampsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::TIMESTAMPS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*timeStampsStream << container.timeStamps;

	CPackedDWORDOutStream* rootPathsStream 
		= dynamic_cast<CPackedDWORDOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::ROOTPATHS_STREAM_ID
								  , PACKED_DWORD_STREAM_TYPE_ID));
	*rootPathsStream << container.rootPaths;

	CDiffIntegerOutStream* changesOffsetsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::CHANGES_OFFSETS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*changesOffsetsStream << container.changesOffsets;

	CDiffIntegerOutStream* copyFromOffsetsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::COPYFROM_OFFSETS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*copyFromOffsetsStream << container.copyFromOffsets;

	// write the changes info

	CPackedDWORDOutStream* changesStream 
		= dynamic_cast<CPackedDWORDOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::CHANGES_STREAM_ID
								  , PACKED_DWORD_STREAM_TYPE_ID));
	*changesStream << container.changes;

	CDiffIntegerOutStream* changedPathsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::CHANGED_PATHS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*changedPathsStream << container.changedPaths;

	CDiffIntegerOutStream* copyFromPathsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::COPYFROM_PATHS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*copyFromPathsStream << container.copyFromPaths;

	CDiffIntegerOutStream* copyFromRevisionsStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CRevisionInfoContainer::COPYFROM_REVISIONS_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));
	*copyFromRevisionsStream << container.copyFromRevisions;

	// ready

	return stream;
}
