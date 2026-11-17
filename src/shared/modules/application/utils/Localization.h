////////////////////////////////////////////////////////////////////////////////
// $Id: Localization.h 9266 2026-02-11 16:15:55Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An Environment Extension class that provides facilities for
//         localizing (translating) application modules.
//         This class is instantiated by itself at initialization time.
//         There is at most one instance of this class.
//
//         To add translations to an existing application module, take care of the
//         following:
//         - There are two parameters that govern the behavior of the
//           Localization class:
//           "Language" defines the language to translate the strings
//           into; if its value matches one of the "LocalizedStrings" row
//           labels, translations will be taken from that row; otherwise,
//           strings will not be translated.
//           "LocalizedStrings" defines string translations. Strings that
//           don't appear as a column label will not be translated.
//           Also, strings with an empty translation entry in "LocalizedStrings"
//           will not be translated.
//         - To provide your translations in a filter constructor, add a line
//             LANGUAGES "Italian", "French",
//           to enumerate the languages you provide translations for,
//           followed by
//             BEGIN_LOCALIZED_STRINGS
//           and a list of translated strings where each entry has the form
//             "Original string", "Italian translation", "French translation",
//           and finally a line
//             END_LOCALIZED_STRINGS
//           to end the enumeration.
//           Even if you don't provide translations this way, operator users may
//           still add their own translations for any language by editing
//           the LocalizedStrings parameter.
//         - Mark localizable string constants by the symbol
//           LocalizableString, i.e.
//             TellUser( LocalizableString( "Well done!" ) );
//           instead of
//             TellUser( "Well done!" );
//         - For GUI elements (e.g. VCL forms) visible to the subject, put a call
//             ApplyLocalizations( pointerToTheGUIElement );
//           inside the Initialize() method of the form's associated
//           GenericFilter.
//         - You should not use LocalizableString on string constants used
//           before the first call to GenericFilter::Initialize() or for
//           initializing static objects of any kind because localization
//           information used will not be valid, or the string may not be
//           updated appropriately then.
//         - Language names are case-insensitive. You may use any legal C
//           identifier for a language name but as a convention we suggest
//           its most common English name, as in
//             Italian, Dutch, French, German
//           with international country abbreviations as optional regional
//           qualifiers as in
//             EnglishUS, EnglishGB, GermanD, GermanCH
//           if that should ever be necessary.
//         - For now, encoding of non-ASCII characters follows the UTF8
//           convention. To ensure platform independent readability of source
//           code files, please use the macros that define HTML character names
//           to their UTF8 encoded string as in
//             "Dansk Sm" oslash "rrebr" oslash "d".
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
////////////////////////////////////////////////////////////////////////////////
#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <map>
#include <string>
#include <vector>

#include "Environment.h"

// Some non-ASCII characters with their HTML names defined to their UTF-8
// encodings.
#define Agrave "\xc3\x80"
#define Aacute "\xc3\x81"
#define Acirc  "\xc3\x82"
#define Atilde "\xc3\x83"
#define Auml   "\xc3\x84"
#define Aring  "\xc3\x85"
#define AElig  "\xc3\x86"
#define Ccedil "\xc3\x87"
#define Egrave "\xc3\x88"
#define Eacute "\xc3\x89"
#define Ecirc  "\xc3\x8a"
#define Euml   "\xc3\x8b"
#define Igrave "\xc3\x8c"
#define Iacute "\xc3\x8d"
#define Icirc  "\xc3\x8e"
#define Iuml   "\xc3\x8f"
#define ETH    "\xc3\x90"
#define Ntilde "\xc3\x91"
#define Ograve "\xc3\x92"
#define Oacute "\xc3\x93"
#define Ocirc  "\xc3\x94"
#define Otilde "\xc3\x95"
#define Ouml   "\xc3\x96"

#define Oslash "\xc3\x98"
#define Ugrave "\xc3\x99"
#define Uacute "\xc3\x9a"
#define Ucirc  "\xc3\x9b"
#define Uuml   "\xc3\x9c"
#define Yacute "\xc3\x9d"
#define THORN  "\xc3\x9e"
#define szlig  "\xc3\x9f"
#define agrave "\xc3\xa0"
#define aacute "\xc3\xa1"
#define acirc  "\xc3\xa2"
#define atilde "\xc3\xa3"
#define auml   "\xc3\xa4"
#define aring  "\xc3\xa5"
#define aelig  "\xc3\xa6"
#define ccedil "\xc3\xa7"
#define egrave "\xc3\xa8"
#define eacute "\xc3\xa9"
#define ecirc  "\xc3\xaa"
#define euml   "\xc3\xab"
#define igrave "\xc3\xac"
#define iacute "\xc3\xad"
#define icirc  "\xc3\xae"
#define iuml   "\xc3\xaf"
#define eth    "\xc3\xb0"
#define ntilde "\xc3\xb1"
#define ograve "\xc3\xb2"
#define oacute "\xc3\xb3"
#define ocirc  "\xc3\xb4"
#define otilde "\xc3\xb5"
#define ouml   "\xc3\xb6"

#define oslash "\xc3\xb8"
#define ugrave "\xc3\xb9"
#define uacute "\xc3\xba"
#define ucirc  "\xc3\xbb"
#define uuml   "\xc3\xbc"
#define yacute "\xc3\xbd"
#define thorn  "\xc3\xbe"
#define yuml   "\xc3\xbf"

// Macros that allow for convenient addition of translations.
#define LANGUAGES                                                                                                      \
    {                                                                                                                  \
        const char *languages_[] = {
#define BEGIN_LOCALIZED_STRINGS                                                                                        \
    }                                                                                                                  \
    ;                                                                                                                  \
    const char *localizations_[] = {
#define END_LOCALIZED_STRINGS                                                                                          \
    }                                                                                                                  \
    ;                                                                                                                  \
    Localization::AddLocalizations(languages_, sizeof(languages_) / sizeof(*languages_), localizations_,               \
                                   sizeof(localizations_) / sizeof(*localizations_));                                  \
    }

class Localization : public EnvironmentExtension
{
  private: // This class instantiates itself once.
    Localization()
    {
    }
    virtual ~Localization()
    {
    }

    // The EnvironmentExtension interface called from EnvironmentBase.
  protected:
    void Publish() override;
    void Preflight() const override;
    void Initialize() override;
    void Process() override
    {
    }

    // The public interface for filter programmers.
  public:
    static const char *LocalizableString(const char *);
    static void ApplyLocalizations(void *);
    static void AddLocalizations(const char **, int, const char **, int);

  private:
    static Localization sInstance;
    void AddLocalizations_(const char **, int, const char **, int);
    typedef std::map<void *, std::vector<std::string>> GUIObjectStringsContainer;
    static GUIObjectStringsContainer &GUIObjectStrings();
    typedef std::map<std::string, std::string> LocalizedStringsContainer;
    static LocalizedStringsContainer &LocalizedStrings();
};

inline const char *LocalizableString(const char *s)
{
    return Localization::LocalizableString(s);
}

inline void ApplyLocalizations(void *p)
{
    Localization::ApplyLocalizations(p);
}

#endif // LOCALIZATION_H
