// TortoiseSVN - a Windows shell extension for easy version control

// External Cache Copyright (C) 2005 - 2006 - Will Dean, Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
#pragma once

#include "TSVNPath.h"

/**
 * \ingroup TSVNCache
 * Handles notifications to the shell about status changes. This is done
 * in a separate thread to avoid deadlocks.
 */
class CShellUpdater
{
public:
	CShellUpdater(void);
	~CShellUpdater(void);

public:
	void Initialise();
	void AddPathForUpdate(const CTSVNPath& path);
	void Stop();

private:
	static unsigned int __stdcall ThreadEntry(void* pContext);
	void WorkerThread();

private:
	CComAutoCriticalSection m_critSec;
	HANDLE m_hThread;
	std::deque<CTSVNPath> m_pathsToUpdate;
	HANDLE m_hTerminationEvent;
	HANDLE m_hWakeEvent;
	
	bool m_bItemsAddedSinceLastUpdate;
	bool m_bRunning;
};


