//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: hill@neurotechcenter.org
// Description: Scripting extension for prompting the user to select
//              files and/or directories.
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
#include "FileSystemTypes.h"

#include "ParserToken.h"
#include "MainWindow.h"
#include "FileUtils.h"
#include <QFileDialog>
#include <iostream> // TODO: remove

std::string CallGUI(const std::string & what, const std::string & prompt, const std::string & startDir, const std::string & fileFilters)
{
    QString qResult;
    QString qWhat        = QString(what.c_str());
    QString qPrompt      = QObject::tr(prompt.c_str());
    QString qStartDir    = QString(startDir.c_str()); // NB: not QObject::tr()
    QString qFileFilters = QObject::tr(fileFilters.c_str());
    std::lock_guard<std::mutex> waitFor(MainWindow::Instance()->GetModalDialogMutex());
    if (MainWindow::Instance()->Terminating()) return "";
    QMetaObject::invokeMethod(
        MainWindow::Instance(),
        "Choose",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(QString, qResult),
        Q_ARG(QString, qWhat),
        Q_ARG(QString, qPrompt),
        Q_ARG(QString, qStartDir),
        Q_ARG(QString, qFileFilters)
    );
    return qResult.toUtf8().constData();
}

bool ParseChooseArgs(std::istream& in, std::ostream& err, const std::string & singularFullName, bool & plural, std::string & prompt, std::string & startDir, std::string & fileFilters)
{
#   define MATCHES(S, X) (!::stricmp(S.c_str(), X))
#   define FAIL(X) {err << "Choose " << singularFullName << (plural ? "s" : "") << ": " << X; return false;}

    // So far we have one of the following:
    // choose directory
    // choose output (may optionally be followed by "file")
    // choose input  (may optionally be followed by "file" or "files")
    // choose file   (synonymous with "choose input" or "choose input file")
    // choose files  (synonymous with "choose input files")

    // The following clauses are then optional (but if you supply more than one, they must be in this order):
    // [of] type  <pattern-or-extension> [<pattern-or-extension-2> ...]
    // [starting] {at|from|in} <starting-directory> 
    // [with] prompt <prompt-string>
    
    ParserToken token;
    in >> token;
    if(MATCHES(singularFullName, "output file"))
    { // The parser is working just with the noun "output" so far. Optionally this may be followed by "file".
        if(     MATCHES(token, "file" )) { plural = false; in >> token; }
    }
    if(MATCHES(singularFullName, "input file"))
    { // The parser is working just with the noun "input" so far. Optionally this may be followed by "file" or "files".
        if(     MATCHES(token, "file" )) { plural = false; in >> token; }
        else if(MATCHES(token, "files")) { plural = true;  in >> token; }
    }
    if(MATCHES(token, "of") && !MATCHES(singularFullName, "directory"))
    { // "of" can be optionally inserted before "type" (unless we're asking for a directory) 
        ParserToken nextToken;
        in >> nextToken;
        if(!MATCHES(nextToken, "type") && !MATCHES(nextToken, "types")) FAIL("expected 'type' after '" << token << "'");
        token = nextToken;
    }
    if((MATCHES(token, "type") || MATCHES(token, "types")) && !MATCHES(singularFullName, "directory"))
    { // "[of] type" must be followed by one or more file extensions (each starting with a dot) and/or glob patterns (each containing a star)
        ParserToken pattern;
        in >> pattern;
        if(!pattern.length()) FAIL("expected file pattern after '" << token << "'");
        fileFilters = "";
        while(true)
        {
            if(pattern.find('*') != std::string::npos || pattern.substr(0, 1) == "." )
            {
                if(pattern.find('*') == std::string::npos) pattern = "*" + pattern;
                std::string description = pattern;
                if(description.substr(0, 1) == "*") description = description.substr(1); 
                //if(description.substr(0, 1) == ".") description = description.substr(1); 
                if(     MATCHES(pattern, "*.dat")) description = "BCI2000 Data Files (*.dat)";
                else if(MATCHES(pattern, "*.prm")) description = "BCI2000 Parameter Files (*.prm)";
                else if(MATCHES(pattern, "*.txt")) description = "Text Files (*.txt)";
                else if(MATCHES(pattern, "*.*") | MATCHES(pattern, "*")) description = "All Files (*)";
                else description += " Files (" + pattern + ")";
                fileFilters += fileFilters.length() ? ";;" : ""; fileFilters += description;
                in >> pattern;
                if(in.fail()) break;
            }
            else
            {
                token = pattern;
                break;
            }
        }
    }
    if(MATCHES(token, "starting"))
    { // "starting" can be optionally inserted before "at"/"from"/"in"
        ParserToken nextToken;
        in >> nextToken;
        if(!MATCHES(nextToken, "at") && !MATCHES(nextToken, "from") && !MATCHES(nextToken, "in"))
            FAIL("expected 'at' / 'from' / 'in' after '" << token << "'");
        token = nextToken;
    }
    if(MATCHES(token, "at") || MATCHES(token, "from") || MATCHES(token, "in"))
    { // "[starting] {at|from|in}" must be followed by a directory path
        ParserToken value;
        in >> value;
        if(!value.length()) FAIL("expected directory path after '" << token << "'");
        startDir = value;
        in >> token;
    }
    if(MATCHES(token, "with"))
    { // "with" can be optionally inserted before "prompt"
        ParserToken nextToken;
        in >> nextToken;
        if(!MATCHES(nextToken, "prompt")) FAIL("expected 'prompt' after '" << token << "'");
        token = nextToken;
    }
    if(MATCHES(token, "prompt"))
    { // "[with] prompt" must be followed by a single-token string that will be used as the prompt
        ParserToken value;
        in >> value;
        if(!value.length()) FAIL("expected prompt string after '" << token << "'");
        prompt = value;
        in >> token;
    }
    if(!in.fail()) FAIL("unexpected token '" << std::string(token) << "'");
    if(!startDir.length()) startDir = ".";
    
    std::string workingDir;
    const char * tmp = bci::BCI_GetPerThreadWD();
    workingDir = tmp;
    bci::BCI_ReleaseObject(tmp);
    startDir = FileUtils::AbsolutePath(startDir, workingDir);
    startDir = FileUtils::RealPath(startDir);
    if(!fileFilters.length()) fileFilters = "All Files (*)";
    return true;
}


#define CHOOSE( NAME, S_IN, S_OUT, S_ERR ) \
    NAME ## Type     s ## NAME ## TypeInstance; /* Create singleton of <NAME>Type, called s<NAME>TypeInstance */ \
    const ObjectType::MethodEntry* NAME ## Type::MethodTable() const { static const ObjectType::MethodEntry table[] = { {"Choose", &Choose}, {0} }; return table; } \
    bool NAME ## Type::Choose(std::istream& S_IN, std::ostream& S_OUT, std::ostream& S_ERR) /* start the Choose() method */

#define REPORT  (plural ? "S" : "") << " OF TYPE \"" << fileFilters << "\" STARTING AT \"" << startDir << "\" WITH PROMPT \"" << prompt << "\""

namespace ScriptingExtensions {
    
    CHOOSE(Directory, in, out, err)
    {
        bool plural = false; // ParseChooseArgs will not allow this to change
        std::string prompt, startDir, fileFilters;
        if(!ParseChooseArgs(in, err, "directory", plural, prompt, startDir, fileFilters)) return false;
        std::string result = CallGUI("directory", prompt, startDir, fileFilters);
        //out << "CHOOSE DIRECTORY" << REPORT << " -> "; // TODO: remove
        out << result;
        return true;
    }
    CHOOSE(Output, in, out, err)
    {
        bool plural = false; // ParseChooseArgs will not allow this to change
        std::string prompt, startDir, fileFilters;
        if(!ParseChooseArgs(in, err, "output file", plural, prompt, startDir, fileFilters)) return false;
        std::string result = CallGUI("output file", prompt, startDir, fileFilters);
        //out << "CHOOSE OUTPUT FILE" << REPORT << " -> "; // TODO: remove
        out << result;
        return true;
    }
    CHOOSE(Input, in, out, err)
    {
        bool plural = false; // ParseChooseArgs will change this if the first token in the input stream is "files"
        std::string prompt, startDir, fileFilters;
        if(!ParseChooseArgs(in, err, "input file", plural, prompt, startDir, fileFilters)) return false;
        std::string result = CallGUI(plural ? "input files" : "input file", prompt, startDir, fileFilters);
        //out << "CHOOSE INPUT FILE" << REPORT << " -> "; // TODO: remove
        out << result;
        return true;
    }
    CHOOSE(File, in, out, err)
    {
        bool plural = false; // ParseChooseArgs will not allow this to change
        std::string prompt, startDir, fileFilters;
        if( !ParseChooseArgs(in, err, "file", plural, prompt, startDir, fileFilters)) return false;
        std::string result = CallGUI("input file", prompt, startDir, fileFilters);
        //out << "CHOOSE INPUT FILE" << REPORT << " -> "; // TODO: remove
        out << result;
        return true;
    }
    CHOOSE(Files, in, out, err)
    {
        bool plural = true; // ParseChooseArgs will not allow this to change
        std::string prompt, startDir, fileFilters;
        if(!ParseChooseArgs(in, err, "file", plural, prompt, startDir, fileFilters)) return false;
        std::string result = CallGUI("input files", prompt, startDir, fileFilters);
        //out << "CHOOSE INPUT FILE" << REPORT << " -> "; // TODO: remove
        out << result;
        return true;
    }
}
