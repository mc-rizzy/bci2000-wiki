//////////////////////////////////////////////////////////////////////
// $Id: CustomDialog.h 9238 2026-01-23 02:10:17Z jhill $
// Authors: hill@neurotechcenter.org
// Description: Implementation for the `custom dialog` scripting extension
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
///////////////////////////////////////////////////////////////////////
#ifndef    CUSTOM_DIALOG_H
#define    CUSTOM_DIALOG_H

#include <vector>
#include <string>
#include <iostream>
#include <QWidget>
#include <QMetaType>

QString ChooserDialog(QWidget * parent,
    const QString &what,     const QString &prompt,
    const QString &startDir, const QString &fileFilters);

class CustomDialogEntry
{
    public:
        CustomDialogEntry( const std::string & variableName="" ) { mVariableName = variableName; mMinimum = 0; mHidden = false;  mStatic = false; }
        ~CustomDialogEntry() {}
        
        bool  ReadOptions(std::istream& in, std::ostream& err);
            
        std::string              mVariableName; // if mVariableName is empty, it's a message (text content is in mLabel, everything else is ignored).
        std::string              mLabel;        // visible label for text fields and dropdowns; ignored if mVariableName is "*buttons"; for other directives that start with a `*`, it's the parameter (*message *title *height *width *left *top *spacing)
        std::vector<std::string> mOptions;      // if mVariableName is "*buttons", this is the left-to-right list of button labels; otherwise, if empty, render the entry is a labelled text field with the mDefault text pre-filled; if non-empty, render as a labelled drop-down menu with the mDefault pre-selected.
        std::string              mDefault;      // if mVariableName is "*buttons", this specifies which button is bound to the return/enter key; otherwise, if mOptions is empty, this is the text to pre-fill into the textbox; if mOptions is non-empty, this is the pre-selected menu option (ignored if it doesn't match an item in mOptions)
        std::string              mEscape;       // ignored unless mVariableName is "*buttons" and this string appears in the mOptions list (defines which button is bound to the escape key, and which button causes all variables to be left unchanged)
        unsigned int             mMinimum;      // unless the typed text (or selected drop-down option) has at least this many characters, the non-escape buttons will be disabled
        std::string              mResult;       // 
        std::string              mCatalogName;  // 
        bool                     mHidden;       // 
        bool                     mStatic;       // 
};
class CustomDialogSpec : public std::vector<CustomDialogEntry>
{
    public:
        bool        Build(std::istream& in, std::ostream& err);
        std::string Execute(QWidget * parent);
        std::string mWorkingDirectory;
};

Q_DECLARE_METATYPE(CustomDialogSpec*) // the corresponding qRegisterMetaType call is in CustomDialog.h

#endif // CUSTOM_DIALOG_H
