////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayBase.h 9479 2026-07-21 09:17:50Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for visualization displays.
//   Also handles message dispatching and storage of visualization properties.
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
#ifndef VIS_DISPLAY_BASE_H
#define VIS_DISPLAY_BASE_H

#include "VisDisplay.h"

#include "Lockable.h"
#include "VisID.h"
#include <QWidget>
#include <map>
#include <sstream>
#include <string>
#include <atomic>

class GenericSignal;
class BitmapImage;
class VideoFrame;

namespace bci { class Resource; }

class VisDisplayBase : public QWidget
{
    Q_OBJECT

  public:
    typedef enum // Possible states of properties ("configs").
    {
        Default = 0,     // May be overridden by a message or by user settings.
        OnceUserDefined, // A previous user setting, read from persistent storage.
        UserDefined,     // Set by user.
        MessageDefined,  // Set by a message, overrides user.
    } ConfigState;

    template <class T> static void Create(const VisID &);
    static void Destroy(const VisID&);

  protected:
    VisDisplayBase(const VisID &inVisID);

  public:
    virtual ~VisDisplayBase();

    const VisID& ID() const;

  public slots:
    virtual void SetVisible(bool);

  public:
    static void Initialize();
    static void Clear();

    static void HandleSignal(const VisID &, const GenericSignal&);
    static void HandleMemo(const VisID &, const std::string &);
    static void HandleBitmap(const VisID &, const BitmapImage &);
    static void HandleVideoFrame(const VisID &, const VideoFrame &);
    static void HandleProperty(const VisID &, CfgID, const std::string &value, int configState);
    static void HandlePropertyReset(const VisID &, CfgID);

    static void HandleDependantCreated(const VisID&, const VisID& dependant);
    static void HandleDependantDestroyed(const VisID&, const VisID& dependant);

  protected:
    bool IsPaletteDark() const;
    QPixmap LoadIcon(bci::Resource&, bool = true);

  protected:
    struct ConfigSettings;
    virtual void OnSetConfig(ConfigSettings &);
    virtual void OnContextMenu(const QPoint &)
    {
    }
    virtual void OnSignal(const GenericSignal &)
    {
    }
    virtual void OnMemo(const QString &)
    {
    }
    virtual void OnBitmap(const BitmapImage &)
    {
    }
    virtual void OnVideoFrame(const VideoFrame &)
    {
    }
    virtual void OnProperty(CfgID, const char *, ConfigState)
    {
    }
    virtual void OnDependantCreated(const VisID& dependant)
    {
    }
    virtual void OnDependantDestroyed(const VisID& dependant)
    {
    }

  private:
    void HandleSignal(const GenericSignal&);
    void HandleMemo(const QString &);
    void HandleBitmap(const BitmapImage &);
    void HandleVideoFrame(const VideoFrame &);
    void HandleProperty(CfgID, const char *, ConfigState);

    void HandleDependantCreated(const VisID& dependant);
    void HandleDependantDestroyed(const VisID& dependant);

    void SetConfig(VisDisplayBase::ConfigSettings&);

  private slots:
    void ContextMenu(const QPoint &);

  protected:
    VisID mVisID;
  
  private:
    std::vector<VisID> mDependants;

  protected:
    const std::vector<VisID>& Dependants() const;
    VisID GetUniqueDependantID(const std::string& pipeElement) const;
    std::string CreateDependantWindowTitle(const VisID&) const;

    // visID->display instance
    struct VisContainer : std::map<VisID, VisDisplayBase *>
    {
        void Clear();
    };
    static VisContainer &Visuals();

  protected:
    // configID->value
    typedef std::map<CfgID, std::string> ConfigSettingsBase;
    class ConfigSettings : public ConfigSettingsBase, public Lockable<>
    {
      public:
        bool Get(CfgID id, std::string &, ConfigState minState = Default);
        bool Put(CfgID id, const std::string &, ConfigState state);
        void Erase(CfgID);
        template <typename T> bool Get(CfgID id, T &, ConfigState minState = Default);
        template <typename T> bool Put(CfgID id, const T &, ConfigState state);
        ConfigState &State(CfgID id)
        {
            return mStates[id];
        }
        void IncBufferActive()
        {
            ++mBufferActive;
        }
        void DecBufferActive()
        {
            mBufferActive = std::max(--mBufferActive, 0);
        }
        bool BufferActive() const
        {
            return mBufferActive;
        }
      private:
        std::map<CfgID, ConfigState> mStates;
        int mBufferActive = 0;
    };

    // visID->config information
    class ConfigContainer : public std::map<std::string, ConfigSettings>
    {
      public:
        void Save();
        void Restore();
    };
    static ConfigContainer &Visconfigs();

  private:
    static VisContainer sVisuals;
    static ConfigContainer sVisconfigs;
};

template <class T> void VisDisplayBase::Create(const VisID &inVisID)
{
    VisDisplayBase *pExisting = VisDisplayBase::Visuals()[inVisID];
    delete pExisting;
    VisDisplayBase *pObject = new T(inVisID);
    VisDisplayBase::Visuals()[inVisID] = pObject;
    pObject->SetConfig(Visconfigs()[inVisID]);
    auto pipelineParentID = inVisID.PipelineParentID();
    if (!pipelineParentID.Empty()) {
            HandleDependantCreated(pipelineParentID, inVisID);
    }
}

template <typename T> bool VisDisplayBase::ConfigSettings::Get(CfgID id, T &t, ConfigState minState)
{
    std::string s;
    if (!Get(id, s, minState))
        return false;
    if (s.empty())
        return true;
    std::istringstream is(s);
    T value;
    if (is >> value)
        t = value;
    return !is.fail();
}

template <typename T> bool VisDisplayBase::ConfigSettings::Put(CfgID id, const T &t, ConfigState state)
{
    std::ostringstream os;
    os << t;
    return !os.fail() && Put(id, os.str(), state);
}

#endif // VIS_DISPLAY_BASE_H
