//////////////////////////////////////////////////////////////////////
// $Id: CustomDialog.cpp 9597 2026-10-13 15:12:32Z mellinger $
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
#include "CustomDialog.h"
#include "ParserToken.h"
#include "EnvVariable.h"
#include "FileUtils.h"
#include "BCI_OperatorLib.h"

#include <sstream>

#include <QString>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QShortcut>
#include <QWindow>
#include <QGuiApplication>
#include <QScreen>
#include <QMetaType>
#include <QAbstractItemView>
#include <QProxyStyle>
#include <QStyle>
#include <QFontDatabase>
#include <QPointer>
#include <QFileDialog>
#include <QRegularExpression>

static int _ev_ptr_id = qRegisterMetaType<CustomDialogSpec*>("CustomDialogSpec*");

#define FAIL(X) { err << "Custom dialog: " << X; return false; }
#define MATCHES(S, X) (!::stricmp(S.c_str(), X))
#define REPLACE(STRING, TARGET, REPLACEMENT) \
    for (size_t startPosition = 0, targetLength = std::string(TARGET).length(); \
         (startPosition = STRING.find(TARGET, startPosition)) != std::string::npos; \
         startPosition++) STRING.replace(startPosition, targetLength, REPLACEMENT);

const char OPTIONS_BEGIN = '{';
const char OPTIONS_SEP   =   '|';
const char OPTIONS_END   =     '}';

bool CustomDialogEntry::ReadOptions(std::istream& in, std::ostream& err)
{
    mOptions.clear();
    std::string option;
    bool closed = false;
    char c = ' ';
    while(!in.fail() && ::isspace(c)) in.get(c);
    if(c != OPTIONS_BEGIN) FAIL("expected options list starting with the '" << OPTIONS_BEGIN << "' character" << ", got '" << c << "' instead");
    for(in.get(c); !in.fail(); in.get(c))
    {
        if(c == OPTIONS_SEP || c == OPTIONS_END)
        {
            if(option.length() || c == OPTIONS_SEP || mOptions.size()) mOptions.push_back(option);
            option = "";
        }
        else option += c;
        if(c == OPTIONS_END) { closed = true; break; }
    }
    if(!closed) FAIL("found no '" << OPTIONS_END << "' character to terminate the options list");
    return true;
}

bool IsLegalVariableName(const std::string & arg)
{
    if(!arg.length()) return false;
    if(!::isalpha(arg[0]) && arg[0] != '_') return false;
    for(int i = 1; i < arg.length(); i++ )
        if(!::isalnum(arg[i]) && arg[i] != '_')
            return false;
    return true;
}

bool CustomDialogSpec::Build(std::istream& in, std::ostream& err)
{
    const char * tmpWD = BCI_GetPerThreadWD();
    mWorkingDirectory = tmpWD;
    BCI_ReleaseObject(tmpWD);
    
    this->clear();
    this->emplace_back("*buttons");
    CustomDialogEntry * pButtons = &this->front();
    pButtons->mDefault = "OK";
    pButtons->mEscape = "Cancel";

    bool buttonsOverridden = false, gotVars = false;
    CustomDialogEntry * pEntry = NULL;
    ParserToken token;
    in >> token;
    std::string additionalExpectations = "";
    while(!in.fail())
    {
        if(MATCHES(token, "message") || MATCHES(token, "title") || MATCHES(token, "spacing") || MATCHES(token, "fontsize")
        || MATCHES(token, "width") || MATCHES(token, "height") || MATCHES(token, "left") || MATCHES(token, "top") || MATCHES(token, "cachefile") )
        {
            std::string varName = "*";
            varName += token.c_str();
            this->emplace_back(varName);
            pEntry = &this->back();
            ParserToken arg;
            in >> arg;
            if(in.fail()) FAIL("expected a string after '" << token << "'");
            pEntry->mLabel = arg;
            additionalExpectations = "";
            in >> arg;
            if(MATCHES(token, "message") && !in.fail())
            {
                if(MATCHES(arg, "hidden") || MATCHES(arg, "invisible") || MATCHES(arg, "visible=false"))
                {
                    pEntry->mHidden = true;
                    in >> arg;
                }
                else if(MATCHES(arg, "visible") || MATCHES(arg, "visible=true"))
                {
                    pEntry->mHidden = false;
                    in >> arg;
                }
            }
            token = arg;
            continue;
        }
        if(MATCHES(token, "buttons"))
        {
            pEntry = &this->front();
            buttonsOverridden = true;
            additionalExpectations = "'default' or 'escape', or possibly a new ";
        }
        else if(MATCHES(token, "var"))
        {
            ParserToken arg;
            in >> arg;
            if(!arg.length()) FAIL("expected a variable name after '" << token << "'");
            if(!IsLegalVariableName(arg)) FAIL("illegal variable name '" << arg << "'");
            this->emplace_back(arg);
            pEntry = &this->back();
            in >> arg;
            if(in.fail() || (arg.length() && arg[0] == OPTIONS_BEGIN)) FAIL("expected a label after '" << token << " " << pEntry->mVariableName << "'");
            pEntry->mLabel = arg;
            gotVars = true;
            additionalExpectations = "'default' or 'minimum', or possibly a new ";
        }
        else FAIL("expected " << additionalExpectations << "'message', 'var', 'buttons' or other directive, but got '" << token << "'");
        if(!pEntry || !pEntry->ReadOptions(in, err)) return false;
        in >> token;
        bool workingOnButtons = (pEntry == &this->front());
        while(!in.fail())
        {
            if(MATCHES(token, "default"))
            {
                ParserToken arg;
                in >> arg;
                if(in.fail()) FAIL("expected a string after '" << token << "'");
                pEntry->mDefault = arg;
                in >> token;
            }
            else if(MATCHES(token, "escape") && workingOnButtons)
            {
                ParserToken arg;
                in >> arg;
                if(in.fail()) FAIL("expected a string after '" << token << "'");
                pEntry->mEscape = arg;
                in >> token;
            }
            else if(MATCHES(token, "minimum") && !workingOnButtons)
            {
                ParserToken arg;
                in >> arg;
                if(in.fail()) FAIL("expected a string after '" << token << "'");
                unsigned int nChars;
                std::stringstream ss; ss.str(arg); ss >> nChars; if(ss.fail() || !ss.eof()) FAIL("failed to interpret " << arg << " as a string-length parameter");
                pEntry->mMinimum = nChars;
                in >> token;
            }
            else if(MATCHES(token, "catalog") && !workingOnButtons)
            {
                ParserToken arg;
                in >> arg;
                if(MATCHES(arg, "in") || MATCHES(arg, "under") || MATCHES(arg, "as")) in >> arg;
                if(in.fail()) FAIL("expected a variable name after '" << token << "'");
                if(!IsLegalVariableName(arg)) FAIL("illegal variable name '" << arg << "'");
                pEntry->mCatalogName = arg;
                in >> token;
            }
            else if((MATCHES(token, "visible") || MATCHES(token, "visible=true")) && !workingOnButtons)
            {
                pEntry->mHidden = false;
                in >> token;
            }
            else if((MATCHES(token, "hidden") || MATCHES(token, "invisible") || MATCHES(token, "visible=false")) && !workingOnButtons)
            {
                pEntry->mHidden = true;
                in >> token;
            }
            else if((MATCHES(token, "fixed") || MATCHES(token, "fixed=true")) && !workingOnButtons)
            {
                pEntry->mStatic = true;
                in >> token;
            }
            else if((MATCHES(token, "fixed=false")) && !workingOnButtons)
            {
                pEntry->mStatic = false;
                in >> token;
            }
            else break;
        }
    }
    if(!buttonsOverridden)
    {
        pButtons = &this->front();
        if(gotVars) pButtons->mOptions.emplace_back("Cancel");
        pButtons->mOptions.emplace_back("OK");
    }
    return true;
}

/////////////////////////////////////////////

static inline int FindString( const std::vector<std::string> &v, const std::string &needle )
{
    for( size_t i = 0; i < v.size(); ++i )
    {
        if( v[i] == needle ) return static_cast<int>(i);
    }
    return -1;
}

static inline void AddLabeledWidget( QVBoxLayout *layout, const QString &label, QWidget *widgetPtr, int vpad=0 )
{
    auto *row = new QWidget(layout->parentWidget());
    auto *box = new QHBoxLayout(row);
    box->setContentsMargins(0, vpad, 0, vpad);
    row->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    box->addWidget(new QLabel(label, row));
    box->addWidget(widgetPtr, 1);
    layout->addWidget(row);
}

class FontScaledButtonStyle final : public QProxyStyle
{
    public:
        explicit FontScaledButtonStyle(QStyle *base, qreal emPx) : QProxyStyle(base), m_emPx(emPx) {}
        int pixelMetric(PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *w = nullptr) const override
        {
            int v = QProxyStyle::pixelMetric(m, opt, w);
            switch (m)
            {
                case QStyle::PM_ButtonMargin:
                    v += qRound(m_emPx * 0.25); // Add ~0.25em to the platform’s margin (scales with font).
                    break;
                default:
                    break;
            }
            return v;
        }
    private:
        qreal m_emPx; // fontMetrics.height() of the dialog font
};

class CloseWatcher : public QObject 
{ // Helper class for ensuring the dialog closes when the parent or app tries to close
    public:
        explicit CloseWatcher(QDialog *d) : QObject(d), mDialog(d)
        {
            if (mDialog)
                QObject::connect(qApp,   &QCoreApplication::aboutToQuit, mDialog, &QDialog::reject, Qt::DirectConnection);
                // Catches programmatic quits (QCoreApplication::quit()), OS shutdown sequences, or other routes where a window close might not reach the parent first
                
            if (mDialog && mDialog->parent())
            {
                mDialog->parent()->installEventFilter(this); // cancel when parent starts to close
                QObject::connect(mDialog->parent(), &QObject::destroyed, mDialog, &QDialog::reject, Qt::DirectConnection); // if the parent gets destroyed through some path where you don’t see a Close first (rare), this tries to reject
            }
        }
    protected:
        bool eventFilter(QObject *obj, QEvent *ev) override
        {
            if (ev->type() == QEvent::Close && mDialog) mDialog->reject();
            return false;
        }
    private:
        QPointer<QDialog> mDialog;
};

std::string CustomDialogSpec::Execute(QWidget * parent)
{ // runs in the main GUI thread - called from MainWindow::CustomDialog()
  // which in turn is called as QMetaObject::invokeMethod(MainWindow::Instance(), "CustomDialog", Qt::BlockingQueuedConnection, ... )

    if( this->empty() ) return std::string();

    // Require *buttons somewhere (typically first)
    const CustomDialogEntry *buttonsPtr = nullptr;
    for( const auto &entry : *this )
        if(MATCHES(entry.mVariableName, "*buttons")) buttonsPtr = &entry;
    if( !buttonsPtr ) return std::string();
    const CustomDialogEntry &buttons = *buttonsPtr;

    QDialog dialog(parent);
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_QuitOnClose, false);
    dialog.setWindowTitle("BCI2000");
    dialog.setMinimumWidth(150);
    
    auto *layoutPtr = new QVBoxLayout(&dialog);
    int left = INT_MAX, top = INT_MAX;
    int fontsize = 15;
    
    std::string cacheFile, cacheFileEncoded, cacheFileLoadingCommand, cacheFileSavingCommand;
    
    // Apply *title, *spacing, etc. if present (last one wins if multiple)
    for( const auto &entry : *this )
    {
#       define TRANSLATE_PARAM(STRING, ARG) { std::stringstream ss; ss.str(STRING); ss >> ARG; if(ss.fail()) continue; }
        
        if(MATCHES(entry.mVariableName, "*title"))
        {
            dialog.setWindowTitle(QString::fromUtf8(entry.mLabel.c_str()));
        }
        else if(MATCHES(entry.mVariableName, "*spacing"))
        {
            unsigned int arg; 
            TRANSLATE_PARAM( entry.mLabel, arg );
            layoutPtr->setSpacing(arg);
        }
        else if(MATCHES(entry.mVariableName, "*width"))
        {
            unsigned int arg;
            TRANSLATE_PARAM( entry.mLabel, arg );
            dialog.setMinimumWidth(arg);
        }
        else if(MATCHES(entry.mVariableName, "*height"))
        {
            unsigned int arg;
            TRANSLATE_PARAM( entry.mLabel, arg );
            dialog.setMinimumHeight(arg);
        }
        else if(MATCHES(entry.mVariableName, "*left"))
        {
            int arg;
            TRANSLATE_PARAM( entry.mLabel, arg );
            left = arg;
        }
        else if(MATCHES(entry.mVariableName, "*top"))
        {
            int arg;
            TRANSLATE_PARAM( entry.mLabel, arg );
            top = arg;
        }
        else if(MATCHES(entry.mVariableName, "*fontsize"))
        {
            int arg;
            TRANSLATE_PARAM( entry.mLabel, arg );
            if(arg > 0) fontsize = arg;
        }
        else if(MATCHES(entry.mVariableName, "*cachefile"))
        {
            cacheFile = entry.mLabel;
            cacheFile = FileUtils::AbsolutePath(cacheFile, mWorkingDirectory);
            
            cacheFileEncoded = cacheFile;
            REPLACE(cacheFileEncoded,  "%", "%25");
            REPLACE(cacheFileEncoded,  "$", "%24");
            REPLACE(cacheFileEncoded,  "\"", "%22");
            
            cacheFileLoadingCommand = "execute script \"" + cacheFileEncoded + "\"";
        }
    }
    
    if( fontsize > 0 )
    {
        QFont f = dialog.font();
        f.setPointSize(fontsize);
        dialog.setFont(f);
        
        const QFontMetrics fm(f);
        dialog.setStyle(new FontScaledButtonStyle(dialog.style(), fm.height()));
    }
    
    struct Field
    {
        size_t index = 0;     // index into *this
        QWidget *widgetPtr = nullptr;
        bool isCombo = false;
        size_t minChars = 0;
    };
    std::vector<Field> fields;
    std::map<std::string, bool> catalogStarted;

    if (cacheFile.length() && FileUtils::IsFile(cacheFile))
    {
        std::string UNCHANGED_BY_LOAD = "\x01";
        std::map<std::string, std::string> preserved;
        for( size_t iEntry = 0; iEntry < this->size(); ++iEntry )
        {
            const CustomDialogEntry &entry = (*this)[iEntry];
            if (entry.mVariableName.empty() || entry.mVariableName[0] == '*') continue;
            std::string preservedValue;
            EnvVariable::Get(entry.mVariableName, preservedValue);
            preserved[entry.mVariableName] = preservedValue;
            EnvVariable::Set(entry.mVariableName, UNCHANGED_BY_LOAD);
        }
        BCI_ExecuteScript(cacheFileLoadingCommand.c_str());
        for( size_t iEntry = 0; iEntry < this->size(); ++iEntry )
        {
            CustomDialogEntry &entry = (*this)[iEntry];
            if (entry.mVariableName.empty() || entry.mVariableName[0] == '*') continue;
            std::string valueAfterLoading;
            EnvVariable::Get(entry.mVariableName, valueAfterLoading);
            if (valueAfterLoading == UNCHANGED_BY_LOAD) EnvVariable::Set(entry.mVariableName, preserved[entry.mVariableName]); // if the variable was not in the cache file, set it back how it was
            else entry.mDefault = valueAfterLoading; // if the variable *was* saved in the cache file, use the new loaded value as the dialog default instead of whatever was specified in the command
        }
    }
    
    // Build body widgets
    for( size_t iEntry = 0; iEntry < this->size(); ++iEntry )
    {
        const CustomDialogEntry &entry = (*this)[iEntry];

        if(entry.mVariableName.empty() || MATCHES(entry.mVariableName, "*message"))
        {
            if(entry.mHidden) continue;
            bool allDashes = !entry.mLabel.empty() && std::all_of(entry.mLabel.begin(), entry.mLabel.end(), [](unsigned char c){ return c == '-'; });
            if( allDashes )
            {
                auto *line = new QFrame(&dialog);
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                line->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
                layoutPtr->addWidget(line);
            }
            else
            {
                auto *lab = new QLabel(QString::fromUtf8(entry.mLabel.c_str()), &dialog);
                lab->setWordWrap(true);
                lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
                layoutPtr->addWidget(lab);
            }
            continue;
        }
        if( entry.mVariableName[0] == '*' ) continue;
        
        if (cacheFile.length())
        {
            if (!cacheFileSavingCommand.length()) cacheFileSavingCommand = "write environment \"" + cacheFileEncoded + "\"";
            cacheFileSavingCommand += " " + entry.mVariableName;
        }
        if(catalogStarted[entry.mCatalogName])
        {
            std::string previousValue;
            EnvVariable::Get(entry.mCatalogName, previousValue);
            EnvVariable::Set(entry.mCatalogName, previousValue + "\n" + entry.mVariableName);
        }
        else
        {
            EnvVariable::Set(entry.mCatalogName, entry.mVariableName);
            catalogStarted[entry.mCatalogName] = true;
        }
        
        if( entry.mHidden ) continue;
        
        if( entry.mStatic )
        {
            std::string content = entry.mLabel + "  " + entry.mDefault;
            auto *lab = new QLabel(QString::fromUtf8(content.c_str()), &dialog);
            lab->setWordWrap(true);
            lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
            layoutPtr->addWidget(lab);
        }
        else if( entry.mOptions.empty() )
        {
            auto *edit = new QLineEdit(&dialog);
            {
                QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                mono.setPointSize(dialog.font().pointSize());   // match *fontsize
                edit->setFont(mono);
            }
            edit->setText(QString::fromUtf8(entry.mDefault.c_str()));
            AddLabeledWidget(layoutPtr, QString::fromUtf8(entry.mLabel.c_str()), edit);
            fields.push_back( Field{ iEntry, edit, false, entry.mMinimum } );
        }
        else
        {
            auto *combo = new QComboBox(&dialog);
            for( const auto &opt : entry.mOptions )
                combo->addItem(QString::fromUtf8(opt.c_str()));
            int defIdx = FindString(entry.mOptions, entry.mDefault);
            if( defIdx >= 0 ) combo->setCurrentIndex(defIdx);
            if( fontsize > 0 ) { QFont cf = combo->font(); cf.setPointSize(fontsize); combo->setFont(cf); if (auto *v = combo->view()) v->setFont(cf); }
            AddLabeledWidget(layoutPtr, QString::fromUtf8(entry.mLabel.c_str()), combo);
            fields.push_back( Field{ iEntry, combo, true, entry.mMinimum } );
        }
    }

    layoutPtr->addWidget(new QLabel(QString(), &dialog)); // final spacer (empty message label)

    // Buttons
    QString pressed;
    const QString qDefaultButtonLabel = QString::fromUtf8(buttons.mDefault.c_str());
    const QString qEscapeButtonLabel  = QString::fromUtf8(buttons.mEscape.c_str());

    auto *box = new QDialogButtonBox(&dialog);
    QPushButton *defaultButtonPtr = nullptr;
    QPushButton *escapeButtonPtr  = nullptr;
    
    for( const auto &buttonLabel : buttons.mOptions )
    {
        const QString qButtonLabel = QString::fromUtf8(buttonLabel.c_str());
        auto *buttonPtr = new QPushButton(qButtonLabel, &dialog);
        buttonPtr->setDefault(false);
        buttonPtr->setAutoDefault(false);
        box->addButton(buttonPtr, QDialogButtonBox::ActionRole);
        QObject::connect(buttonPtr, &QPushButton::clicked, &dialog, [&dialog, &pressed, qButtonLabel]()
        {
            pressed = qButtonLabel;
            dialog.accept();
        });
        if( !qDefaultButtonLabel.isEmpty() && qButtonLabel.compare(qDefaultButtonLabel, Qt::CaseInsensitive) == 0 )
        {
            buttonPtr->setDefault(true);
            buttonPtr->setAutoDefault(true);
            defaultButtonPtr = buttonPtr;
        }
        if( !qEscapeButtonLabel.isEmpty() && qButtonLabel.compare(qEscapeButtonLabel, Qt::CaseInsensitive) == 0 )
        {
            escapeButtonPtr = buttonPtr;
        }
    }
    
    int maxW = 0;
    for (QAbstractButton *b : box->buttons())
    {
        b->ensurePolished();               // use final font/style
        maxW = qMax(maxW, b->sizeHint().width());
    }
    for (QAbstractButton *b : box->buttons())
        b->setMinimumWidth(maxW);

    // Esc binding
    if( escapeButtonPtr )
    {
        auto *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), &dialog);
        QObject::connect(shortcut, &QShortcut::activated, &dialog, [escapeButtonPtr]() { escapeButtonPtr->click(); } );
    }
    
    auto Validate = [&]() {
        bool ok = true;
        for (const auto &field : fields)
        {
            if(!field.minChars) continue;
            QString val( field.isCombo ? static_cast<QComboBox*>(field.widgetPtr)->currentText() : static_cast<QLineEdit*>(field.widgetPtr)->text() );
            if(val.length() < field.minChars) { ok = false; break; }
        }
        for (auto *b : box->buttons()) if(b != escapeButtonPtr) b->setEnabled(ok);
    };
    
    for (const auto &field : fields)
    {
        if (field.isCombo) QObject::connect(static_cast<QComboBox*>(field.widgetPtr), qOverload<int>(&QComboBox::currentIndexChanged), &dialog, Validate);
        else               QObject::connect(static_cast<QLineEdit*>(field.widgetPtr), &QLineEdit::textChanged,                         &dialog, Validate);
    }
    layoutPtr->addStretch(1);
    layoutPtr->addWidget(box);
    Validate();
    dialog.resize(dialog.sizeHint());
    dialog.show();
    QScreen *screen = dialog.windowHandle() ? dialog.windowHandle()->screen() : QGuiApplication::primaryScreen();
    dialog.move(screen->geometry().center() - dialog.rect().center());
    if(left != INT_MAX) dialog.move(left, dialog.y());
    if(top  != INT_MAX) dialog.move(dialog.x(), top);

    auto *watcher = new CloseWatcher(&dialog); // owned by dialog due to QObject(&dialog) initialization: dialog will deallocate it automatically
    int execResult = dialog.exec(); // actually run the dialog

    // Cancel semantics: empty pressed OR pressed equals escape
    const bool wasCancelled = (
           execResult != QDialog::Accepted
        || pressed.isEmpty()
        || (!qEscapeButtonLabel.isEmpty() && pressed.compare(qEscapeButtonLabel, Qt::CaseInsensitive) == 0)
    );
    if( wasCancelled ) return std::string();

    for( const auto &field : fields )
    {
        CustomDialogEntry & entry = (*this)[field.index];

        if( entry.mHidden || entry.mStatic ) entry.mResult = entry.mDefault;
        else
        {
            QString val;
            if( field.isCombo ) val = static_cast<QComboBox*>(field.widgetPtr)->currentText();
            else                val = static_cast<QLineEdit*>(field.widgetPtr)->text();
            entry.mResult = val.toUtf8().constData();
        }
        EnvVariable::Set(entry.mVariableName, entry.mResult);
    }
    if (cacheFileSavingCommand.length()) BCI_ExecuteScript(cacheFileSavingCommand.c_str());
    return pressed.toUtf8().constData();
}

QString ChooserDialog( QWidget * parent,
    const QString &what,     const QString &prompt,
    const QString &startDir, const QString &fileFilters )
{ // runs in the main GUI thread - called from MainWindow::Choose()
  // which in turn is called as QMetaObject::invokeMethod(MainWindow::Instance(), "Choose", Qt::BlockingQueuedConnection, ... )
  
    QString result;
    QFileDialog dialog(parent, prompt, startDir, fileFilters);
    dialog.setAttribute(Qt::WA_QuitOnClose, false);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    // we disable native dialogs because they may omit some features,
    // like customizable prompts and overwrite confirmation
    
    if(     what == QStringLiteral("directory"))
    {
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption  (QFileDialog::ShowDirsOnly, true);
    }
    else if(what == QStringLiteral("output file"))
    {
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode  (QFileDialog::AnyFile );
        dialog.setOption    (QFileDialog::DontConfirmOverwrite, false);
    }
    else if(what == QStringLiteral("input file"))
    {
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.setFileMode  (QFileDialog::ExistingFile);
    }
    else if(what == QStringLiteral("input files"))
    {
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.setFileMode  (QFileDialog::ExistingFiles);
    }
    else return result; // unknown request – return empty

    dialog.resize(dialog.sizeHint());
    dialog.show();
    QScreen *pScreen = dialog.windowHandle() ? dialog.windowHandle()->screen() : QGuiApplication::primaryScreen();
    dialog.move(pScreen->geometry().center() - dialog.rect().center());

    auto *watcher = new CloseWatcher(&dialog); // owned by dialog due to QObject(&dialog) initialization: dialog will deallocate it automatically
    if(dialog.exec() != QDialog::Accepted) return result; // user pressed cancel - return empty
    const QStringList selection = dialog.selectedFiles();
    if(selection.isEmpty()) return result; // user selected zero files - return empty

    result = selection.join(QChar(u'\n')); // if there are multiple paths, they should be newline-delimited

    if(what == QStringLiteral("output file") && QFileInfo(result).suffix().isEmpty())
    { // If the user specified an "output file" name without a file extension, append the first extension (if any) of the selected filter
        const QString filter = dialog.selectedNameFilter();   // e.g. "CSV (*.csv *.tsv)"
        static const QRegularExpression re("(\\*\\.\\s*([A-Za-z0-9]+))");
        QRegularExpressionMatch match = re.match(filter);
        if(match.hasMatch())
        {
            const QString extension = match.captured(2);
            result += u'.' + extension;
        }
    }
    return result;
}
