// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2011, 2013, 2015-2016 - TortoiseSVN

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
#include "../SmartHandle.h"
#include "scintilla.h"
#include "SciLexer.h"
#include "../../../ext/hunspell/hunspell.hxx"
#include "../../../ext/hunspell/mythes.hxx"
#include "ProjectProperties.h"
#include "PersonalDictionary.h"
#include <regex>
#include <spellcheck.h>
#include "LruCache.h"

#define AUTOCOMPLETE_SPELLING       0
#define AUTOCOMPLETE_FILENAME       1
#define AUTOCOMPLETE_PROGRAMCODE    2
#define AUTOCOMPLETE_SNIPPET        3

_COM_SMARTPTR_TYPEDEF(ISpellCheckerFactory, __uuidof(ISpellCheckerFactory));
_COM_SMARTPTR_TYPEDEF(ISpellChecker, __uuidof(ISpellChecker));
_COM_SMARTPTR_TYPEDEF(IEnumSpellingError, __uuidof(IEnumSpellingError));
_COM_SMARTPTR_TYPEDEF(ISpellingError, __uuidof(ISpellingError));

//forward declaration
class CSciEdit;

/**
 * \ingroup Utils
 * This class acts as an interface so that CSciEdit can call these methods
 * on other objects which implement this interface.
 * Classes implementing this interface must call RegisterContextMenuHandler()
 * in CSciEdit to register themselves.
 */
class CSciEditContextMenuInterface
{
public:
    /**
     * When the handler is called with this method, it can add entries
     * to the \a mPopup context menu itself. The \a nCmd param is the command
     * ID number the handler must use for its commands. For every added command,
     * the handler is responsible to increment the \a nCmd param by one.
     */
    virtual void        InsertMenuItems(CMenu& mPopup, int& nCmd);

    /**
     * The handler is called when the user clicks on any context menu entry
     * which isn't handled by CSciEdit itself. That means the handler might
     * be called for entries it hasn't added itself!
     * \remark the handler should return \a true if it handled the call, otherwise
     * it should return \a false
     */
    virtual bool        HandleMenuItemClick(int cmd, CSciEdit * pSciEdit);
    virtual void        HandleSnippet(int type, const CString &text, CSciEdit *pSciEdit);
};

/**
 * \ingroup Utils
 * Encapsulates the Scintilla edit control. Usable as a replacement for the
 * MFC CEdit control, but not a drop-in replacement!
 * Also provides additional features like spell checking, auto completion, ...
 */
class CSciEdit : public CWnd
{
    DECLARE_DYNAMIC(CSciEdit)
public:
    CSciEdit(void);
    ~CSciEdit(void);
    /**
     * Initialize the scintilla control. Must be called prior to any other
     * method!
     */
    void        Init(const ProjectProperties& props);
    void        Init(LONG lLanguage = 0);
    void        SetIcon(const std::map<int, UINT> &icons);

    /**
     * Execute a scintilla command, e.g. SCI_GETLINE.
     */
    LRESULT     Call(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
    /**
     * The specified text is written to the scintilla control.
     */
    void        SetText(const CString& sText);
    /**
     * The specified text is inserted at the cursor position. If a text is
     * selected, that text is replaced.
     * \param sText test to insert
     * \param bNewLine if set to true, a newline is appended.
     */
    void        InsertText(const CString& sText, bool bNewLine = false);
    /**
     * Retrieves the text in the scintilla control.
     */
    CString     GetText(void);
    /**
     * Sets the font for the control.
     */
    void        SetFont(CString sFontName, int iFontSizeInPoints);
    /**
     * Adds a list of words for use in auto completion.
     */
    void        SetAutoCompletionList(std::map<CString, int>&& list, TCHAR separator = ';', TCHAR typeSeparator = '?');
    /**
     * Returns the word located under the cursor.
     */
    CString     GetWordUnderCursor(bool bSelectWord = false);

    void        RegisterContextMenuHandler(CSciEditContextMenuInterface * object) {m_arContextHandlers.Add(object);}

    CStringA    StringForControl(const CString& text);
    CString     StringFromControl(const CStringA& text);

    void        SetRepositoryRoot(const CString& url) {m_sRepositoryRoot = url;}

    void        RestyleBugIDs();
private:
    CAutoLibrary m_hModule;
    LRESULT     m_DirectFunction;
    LRESULT     m_DirectPointer;
    std::unique_ptr<Hunspell> pChecker;
    std::unique_ptr<MyThes>   pThesaur;
    UINT        m_spellcodepage;
    std::map<CString, int> m_autolist;
    TCHAR       m_separator;
    TCHAR       m_typeSeparator;
    CStringA    m_sCommand;
    CStringA    m_sBugID;
    CString     m_sUrl;
    CString     m_sRepositoryRoot;
    CArray<CSciEditContextMenuInterface *, CSciEditContextMenuInterface *> m_arContextHandlers;
    CPersonalDictionary m_personalDict;
    bool        m_bDoStyle;
    int         m_nAutoCompleteMinChars;
    ISpellCheckerFactoryPtr     m_spellCheckerFactory;
    ISpellCheckerPtr            m_SpellChecker;
    LruCache<std::wstring, BOOL> m_SpellingCache;

    static bool IsValidURLChar(unsigned char ch);
protected:
    virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    void        CheckSpelling(int startpos, int endpos);
    void        SuggestSpellingAlternatives(void);
    void        DoAutoCompletion(int nMinPrefixLength);
    BOOL        LoadDictionaries(LONG lLanguageID);
    BOOL        MarkEnteredBugID(int startstylepos, int endstylepos);
    bool        StyleEnteredText(int startstylepos, int endstylepos);
    void        StyleURLs(int startstylepos, int endstylepos);
    bool        WrapLines(int startpos, int endpos);
    bool        FindStyleChars(const char * line, char styler, int& start, int& end);
    void        AdvanceUTF8(const char * str, int& pos);
    BOOL        CheckWordSpelling(const CString & sWord);
    BOOL        IsMisspelled(const CString& sWord);
    DWORD       GetStyleAt(int pos) { return (DWORD)Call(SCI_GETSTYLEAT, pos) & 0x1f; }
    bool        IsUrl(const CStringA& sText);
    CStringA    GetWordForSpellChecker(const CString& sWord);
    CString     GetWordFromSpellChecker(const CStringA& sWordA);

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    DECLARE_MESSAGE_MAP()
};
