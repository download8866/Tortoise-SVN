// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2004 - Tim Kemp and Stefan Kueng

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
#include "StdAfx.h"
#include "client.h"
#include "svn_error_codes.h"
#include "UnicodeUtils.h"
#include "registry.h"
#include "Utils.h"
#include "SVN.h"
#include ".\revisiongraph.h"


CRevisionGraph::CRevisionGraph(void) : SVNPrompt()
	, m_bCancelled(FALSE)
{
	m_app = NULL;
	hWnd = NULL;
	memset (&ctx, 0, sizeof (ctx));
	parentpool = svn_pool_create(NULL);
	svn_utf_initialize(parentpool);

	Err = svn_config_ensure(NULL, parentpool);
	pool = svn_pool_create (parentpool);
	// set up the configuration
	if (Err == 0)
		Err = svn_config_get_config (&(ctx.config), NULL, pool);

	if (Err != 0)
	{
		::MessageBox(NULL, this->GetLastErrorMessage(), _T("TortoiseSVN"), MB_ICONERROR);
		svn_pool_destroy (pool);
		svn_pool_destroy (parentpool);
		exit(-1);
	} // if (Err != 0) 

	// set up authentication
	Init(pool);

	ctx.cancel_func = cancel;
	ctx.cancel_baton = this;

	//set up the SVN_SSH param
	CString tsvn_ssh = CRegString(_T("Software\\TortoiseSVN\\SSH"));
	tsvn_ssh.Replace('\\', '/');
	if (!tsvn_ssh.IsEmpty())
	{
		svn_config_t * cfg = (svn_config_t *)apr_hash_get ((apr_hash_t *)ctx.config, SVN_CONFIG_CATEGORY_CONFIG,
			APR_HASH_KEY_STRING);
		svn_config_set(cfg, SVN_CONFIG_SECTION_TUNNELS, "ssh", CUnicodeUtils::GetUTF8(tsvn_ssh));
	}
}

CRevisionGraph::~CRevisionGraph(void)
{
	svn_pool_destroy (parentpool);
	for (int i=0; i<m_arEntryPtrs.GetCount(); ++i)
	{
		delete (CRevisionEntry*)m_arEntryPtrs.GetAt(i);
	}
}

BOOL CRevisionGraph::ProgressCallback(CString /*text*/, CString /*text2*/, DWORD /*done*/, DWORD /*total*/) {return TRUE;}

svn_error_t* CRevisionGraph::cancel(void *baton)
{
	CRevisionGraph * me = (CRevisionGraph *)baton;
	if (me->m_bCancelled)
	{
		CStringA temp;
		temp.LoadString(IDS_SVN_USERCANCELLED);
		return svn_error_create(SVN_ERR_CANCELLED, NULL, temp);
	}
	return SVN_NO_ERROR;
}

svn_error_t* CRevisionGraph::logDataReceiver(void* baton, 
								  apr_hash_t* ch_paths, 
								  svn_revnum_t rev, 
								  const char* author, 
								  const char* date, 
								  const char* msg, 
								  apr_pool_t* pool)
{
// put all data we receive into an array for later use.
	svn_error_t * error = NULL;
	log_entry * e = NULL;
	CRevisionGraph * me = (CRevisionGraph *)baton;

	e = (log_entry *)apr_pcalloc (me->pool, sizeof(log_entry));
	if (e==NULL)
	{
		return svn_error_create(APR_OS_START_SYSERR + ERROR_NOT_ENOUGH_MEMORY, NULL, NULL);
	}
	if (date && date[0])
	{
		//Convert date to a format for humans.
		error = svn_time_from_cstring (&e->time, date, pool);
		if (error)
			return error;
	}
	e->author = apr_pstrdup(me->pool, author);
	//create a copy of the hash
	e->ch_paths = apr_hash_make(me->pool);
	apr_hash_index_t* hi;
	for (hi = apr_hash_first (pool, ch_paths); hi; hi = apr_hash_next (hi))
	{
		const char * key;
		const char * keycopy;
		svn_log_changed_path_t *val;
		svn_log_changed_path_t *valcopy;
		apr_hash_this(hi, (const void**)&key, NULL, (void**)&val);
		keycopy = apr_pstrdup(me->pool, key);
		valcopy = (svn_log_changed_path_t*)apr_pmemdup(me->pool, val, sizeof(svn_log_changed_path_t));
		valcopy->copyfrom_path = apr_pstrdup(me->pool, val->copyfrom_path);
		apr_hash_set(e->ch_paths, keycopy, APR_HASH_KEY_STRING, valcopy);
	}
	e->msg = apr_pstrdup(me->pool, msg);
	e->rev = rev;
	if (me->m_lHeadRevision < rev)
		me->m_lHeadRevision = rev;
	CString temp, temp2;
	temp.LoadString(IDS_REVGRAPH_PROGGETREVS);
	temp2.Format(IDS_REVGRAPH_PROGCURRENTREV, rev);
	if (!me->ProgressCallback(temp, temp2, me->m_lHeadRevision - rev, me->m_lHeadRevision))
		me->m_bCancelled = TRUE;
	APR_ARRAY_PUSH(me->m_logdata, log_entry *) = e;
	return SVN_NO_ERROR;
}

BOOL CRevisionGraph::FetchRevisionData(CString path)
{
	// prepare the path for Subversion
	SVN::preparePath(path);
	CStringA url = CUnicodeUtils::GetUTF8(path);

	// convert a working copy path into an URL if necessary
	if (!svn_path_is_url(url))
	{
		//not an url, so get the URL from the working copy path first
		svn_wc_adm_access_t *adm_access;          
		const svn_wc_entry_t *entry;  
		const char * canontarget = svn_path_canonicalize(url, pool);
#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression is constant
			Err = svn_wc_adm_probe_open2 (&adm_access, NULL, canontarget,
				FALSE, 0, pool);
			if (Err) return FALSE;
			Err =  svn_wc_entry (&entry, canontarget, adm_access, FALSE, pool);
			if (Err) return FALSE;
			Err = svn_wc_adm_close (adm_access);
			if (Err) return FALSE;
#pragma warning(pop)

			url = entry ? entry->url : "";
	}
	if (!CUtils::IsEscaped(url))
		url = CUtils::PathEscape(url);

	// we have to get the log from the repository root
	if (!GetRepositoryRoot(url))
		return FALSE;
	m_sRepoRoot = url;
	apr_array_header_t *targets = apr_array_make (pool, 1, sizeof (const char *));
	const char * target = apr_pstrdup (pool, url);
	(*((const char **) apr_array_push (targets))) = target;

	m_logdata = apr_array_make(pool, 100, sizeof(log_entry *));
	m_lHeadRevision = -1;
	Err = svn_client_log (targets, 
		SVNRev(1), 
		SVNRev(SVNRev::REV_HEAD), 
		TRUE,
		FALSE,
		logDataReceiver,
		(void *)this, &ctx, pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CRevisionGraph::AnalyzeRevisionData(CString path)
{
	if (m_logdata == NULL)
		return FALSE;
	SVN::preparePath(path);
	CStringA url = CUnicodeUtils::GetUTF8(path);

	// convert a working copy path into an URL if necessary
	if (!svn_path_is_url(url))
	{
		//not an url, so get the URL from the working copy path first
		svn_wc_adm_access_t *adm_access;          
		const svn_wc_entry_t *entry;  
		const char * canontarget = svn_path_canonicalize(url, pool);
#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression is constant
		Err = svn_wc_adm_probe_open2 (&adm_access, NULL, canontarget,
			FALSE, 0, pool);
		if (Err) return FALSE;
		Err =  svn_wc_entry (&entry, canontarget, adm_access, FALSE, pool);
		if (Err) return FALSE;
		Err = svn_wc_adm_close (adm_access);
		if (Err) return FALSE;
#pragma warning(pop)

		url = entry ? entry->url : "";
	}
	if (!CUtils::IsEscaped(url))
		url = CUtils::PathEscape(url);
	url = url.Mid(m_sRepoRoot.GetLength());
	m_nRecurseLevel = 0;
	AnalyzeRevisions(url, m_lHeadRevision, 1);
	CheckForwardCopies();
	return TRUE;
}

BOOL CRevisionGraph::AnalyzeRevisions(CStringA url, LONG startrev, LONG endrev)
{
	LONG forward = 1;
	if (startrev > endrev)
		forward = -1;
	m_nRecurseLevel++;
	TRACE(_T("Analyzing %s from %ld to %ld - recurse level %d\n"), (LPCTSTR)CString(url), startrev, endrev, m_nRecurseLevel);
	for (long currentrev=startrev; currentrev!=endrev; currentrev += forward)
	{
		if (m_nRecurseLevel==1)
		{
			CString temp, temp2;
			temp.LoadString(IDS_REVGRAPH_PROGANALYZE);
			temp2.Format(IDS_REVGRAPH_PROGANALYZEREV, currentrev);
			if (!ProgressCallback(temp, temp2, forward==1 ? endrev-currentrev : startrev-currentrev, forward==1 ? endrev-startrev : startrev-endrev))
				return FALSE;
		}
		log_entry * logentry = APR_ARRAY_IDX(m_logdata, currentrev-1, log_entry *);
		if (logentry)
		{
			ASSERT(logentry->rev == currentrev);
			apr_hash_index_t* hi;
			for (hi = apr_hash_first (pool, logentry->ch_paths); hi; hi = apr_hash_next (hi))
			{
				const char * key;
				svn_log_changed_path_t *val;
				apr_hash_this(hi, (const void**)&key, NULL, (void**)&val);
				if (IsParentOrItself(key, url))
				{
					if (strcmp(key, url)==0)
					{
						CRevisionEntry * reventry = new CRevisionEntry();
						reventry->revision = currentrev;
						reventry->author = logentry->author;
						reventry->date = logentry->time;
						reventry->message = logentry->msg;
						reventry->url = key;
						reventry->action = val->action;
						reventry->level = m_nRecurseLevel;
						if (val->copyfrom_path)
						{
							reventry->pathfrom = val->copyfrom_path;
							reventry->revisionfrom = val->copyfrom_rev;
						}
						m_arEntryPtrs.Add(reventry);
						TRACE("revision entry: %ld - %s\n", reventry->revision, reventry->url);
						if (val->copyfrom_path)
						{
							// the file/folder was copied to here
							// so we have to get all the information from that source too.
							AnalyzeRevisions(val->copyfrom_path, currentrev-1, startrev);
						}
					}
					else if (val->action != 'M')
					{
						//the parent got moved, and therefore we too
						CRevisionEntry * reventry = new CRevisionEntry();
						reventry->revision = currentrev;
						reventry->author = logentry->author;
						reventry->date = logentry->time;
						reventry->message = logentry->msg;
						reventry->url = key;
						reventry->action = val->action;
						reventry->level = m_nRecurseLevel;
						if (val->copyfrom_path)
						{
							reventry->pathfrom = val->copyfrom_path;
							reventry->revisionfrom = val->copyfrom_rev;
						}
						m_arEntryPtrs.Add(reventry);
						TRACE("revision entry: %ld - %s\n", reventry->revision, reventry->url);
						AnalyzeRevisions(key, currentrev+1, m_lHeadRevision);
					}
				}
				else
				{
					if (val->copyfrom_path)
					{
						//check if our file/folder may be the source of a copy operation
						if (IsParentOrItself(val->copyfrom_path, url))
						{
							CRevisionEntry * reventry = new CRevisionEntry();
							reventry->revision = currentrev;
							reventry->author = logentry->author;
							reventry->date = logentry->time;
							reventry->message = logentry->msg;
							reventry->url = key;
							reventry->action = val->action;
							reventry->level = m_nRecurseLevel;
							if (val->copyfrom_path)
							{
								reventry->pathfrom = val->copyfrom_path;
								reventry->revisionfrom = val->copyfrom_rev;
							}
							m_arEntryPtrs.Add(reventry);
							TRACE("revision entry: %ld - %s\n", reventry->revision, reventry->url);
							AnalyzeRevisions(key, currentrev+1, m_lHeadRevision);
						}
					}
				}
			}
		}
	}
	m_nRecurseLevel--;
	return TRUE;
}

BOOL CRevisionGraph::CheckForwardCopies()
{
	CString temp, temp2;
	temp.LoadString(IDS_REVGRAPH_PROGCHECKFORWARD);
	for (INT_PTR i=0; i<m_arEntryPtrs.GetCount(); ++i)
	{
		temp2.Format(IDS_REVGRAPH_PROGCHECKFORWARDREV, i);
		if (!ProgressCallback(temp, temp2, i, m_arEntryPtrs.GetCount()))
			return FALSE;
		CRevisionEntry * logentry = (CRevisionEntry*)m_arEntryPtrs.GetAt(i);
		if ((logentry->pathfrom)&&(logentry->revisionfrom))
		{
			//this entry was copied from somewhere.
			//try to find that revision entry
			BOOL found = FALSE;
			for (INT_PTR j=0; j<m_arEntryPtrs.GetCount(); ++j)
			{
				CRevisionEntry * e = (CRevisionEntry*)m_arEntryPtrs.GetAt(j);
				if (e->revision == logentry->revisionfrom)
				{
					e->revisionto = logentry->revision;
					e->pathto = logentry->url;
					found = TRUE;
					break;
				}
			}
			if (!found)
			{
				//create a new entry as a starting point
				log_entry * origentry = APR_ARRAY_IDX(m_logdata, logentry->revisionfrom-1, log_entry *);
				CRevisionEntry * reventry = new CRevisionEntry();
				svn_log_changed_path_t *val;
				apr_hash_index_t* hi = apr_hash_first (pool, origentry->ch_paths);
				if (hi)
				{
					apr_hash_this(hi, (const void**)&reventry->url, NULL, (void**)&val);
					reventry->revision = origentry->rev;
					reventry->author = origentry->author;
					reventry->date = origentry->time;
					reventry->message = origentry->msg;
					reventry->action = val->action;
					if (val->copyfrom_path)
					{
						reventry->pathfrom = val->copyfrom_path;
						reventry->revisionfrom = val->copyfrom_rev;
					}
					m_arEntryPtrs.Add(reventry);
				}
			}
		}
	}
	return TRUE;
}

BOOL CRevisionGraph::IsParentOrItself(const char * parent, const char * child)
{
	if (strncmp(parent, child, strlen(parent))==0)
	{
		size_t len = strlen(parent);
		if ((child[len]=='/')||(child[len]==0))
			return TRUE;
	}
	return FALSE;
}

CString CRevisionGraph::GetLastErrorMessage()
{
	CString msg;
	CString temp;
	char errbuf[256];

	if (Err != NULL)
	{
		svn_error_t * ErrPtr = Err;
		if (ErrPtr->message)
			msg = CUnicodeUtils::GetUnicode(ErrPtr->message);
		else
		{
			/* Is this a Subversion-specific error code? */
			if ((ErrPtr->apr_err > APR_OS_START_USEERR)
				&& (ErrPtr->apr_err <= APR_OS_START_CANONERR))
				msg = svn_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf));
			/* Otherwise, this must be an APR error code. */
			else
			{
				svn_error_t *temp_err = NULL;
				const char * err_string = NULL;
				temp_err = svn_utf_cstring_to_utf8(&err_string, apr_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf)), ErrPtr->pool);
				if (temp_err)
				{
					svn_error_clear (temp_err);
					msg = _T("Can't recode error string from APR");
				}
				else
				{
					msg = CUnicodeUtils::GetUnicode(err_string);
				}
			}
		}
		while (ErrPtr->child)
		{
			ErrPtr = ErrPtr->child;
			msg += _T("\n");
			msg += CUnicodeUtils::GetUnicode(ErrPtr->message);
		}
	}
	return _T("");
}

BOOL CRevisionGraph::GetRepositoryRoot(CStringA& url)
{
	svn_ra_plugin_t *ra_lib;
	void *ra_baton, *session;
	const char * returl;

	apr_pool_t * localpool = svn_pool_create(pool);
	/* Get the RA library that handles URL. */
	if ((Err = svn_ra_init_ra_libs (&ra_baton, localpool))!=0)
		return FALSE;
	if ((Err = svn_ra_get_ra_library (&ra_lib, ra_baton, url, localpool))!=0)
		return FALSE;

	/* Open a repository session to the URL. */
	if ((Err = svn_client__open_ra_session (&session, ra_lib, url, NULL, NULL, NULL, FALSE, FALSE, &ctx, localpool))!=0)
		return FALSE;

	if ((Err = ra_lib->get_repos_root(session, &returl, localpool))!=0)
		return FALSE;
	url = CStringA(returl);
	svn_pool_clear(localpool);
	return TRUE;
}
