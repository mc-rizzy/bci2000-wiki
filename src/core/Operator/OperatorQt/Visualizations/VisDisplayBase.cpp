////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayBase.cpp 9479 2026-07-21 09:17:50Z mellinger $
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
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
#include "VisDisplayBase.h"

#include "CfgID.h"
#include "GenericSignal.h"
#include "Settings.h"
#include "VisDisplayWindow.h"
#include "Preferences.h"
#include "Resources.h"

#include <set>
#include <QHideEvent>

VisDisplayBase::VisContainer VisDisplayBase::sVisuals;
VisDisplayBase::ConfigContainer VisDisplayBase::sVisconfigs;

////////////////////////////////////////////////////////////////////////////////
void VisDisplayBase::ConfigContainer::Save()
{
    Settings settings(Preferences::Instance()->mInstancePrefix);
    settings.beginGroup("Visualization");
    for (auto i = begin(); i != end(); ++i)
    {
        settings.beginGroup(i->first.c_str());
        std::set<CfgID> userDefinedCfgIDs;
        for (ConfigSettings::iterator j = i->second.begin(); j != i->second.end(); ++j)
            if (i->second.State(j->first) == UserDefined)
                userDefinedCfgIDs.insert(j->first);

        if (!userDefinedCfgIDs.empty())
        {
            // We add a Title entry to make it easier for the user to understand entries in the ini file.
            // Note that this entry is not named WindowTitle, so it is never restored from the ini file but
            // always determined by current BCI2000 module code.
            settings.setValue("Title", i->second[CfgID::WindowTitle].c_str());
            for (std::set<CfgID>::const_iterator j = userDefinedCfgIDs.begin(); j != userDefinedCfgIDs.end(); ++j)
                settings.setValue(std::string(*j).c_str(), i->second[*j].c_str());
        }
        settings.endGroup();
    }
}

void VisDisplayBase::ConfigContainer::Restore()
{
    Settings settings(Preferences::Instance()->mInstancePrefix);
    settings.beginGroup("Visualization");
    QStringList visIDs = settings.childGroups();
    for (const auto &visID : std::as_const(visIDs))
    {
        ConfigSettings &c = (*this)[visID.toUtf8().constData()];
        WithLock(c)
        {
            settings.beginGroup(visID);
            QStringList cfgIDs = settings.childKeys();
            for (const auto &cfgIDstring : std::as_const(cfgIDs))
            {
                CfgID cfgID(cfgIDstring.toUtf8().constData());
                if (cfgID != CfgID::None)
                {
                    std::string value = settings.value(cfgIDstring).toString().toUtf8().constData();
                    c.Put(cfgID, value, OnceUserDefined);
                }
            }
        }
        settings.endGroup();
    }
}

////////////////////////////////////////////////////////////////////////////////
void VisDisplayBase::Initialize()
{
    sVisconfigs.Restore();
}

void VisDisplayBase::Clear()
{
    sVisconfigs.Save();
    sVisuals.Clear();
}

const std::vector<VisID>& VisDisplayBase::Dependants() const
{
    return mDependants;
}

VisID VisDisplayBase::GetUniqueDependantID(const std::string& pipeElement) const
{
    VisID visID = mVisID.PipelineDependantID(pipeElement);
    int n = 1;
    while (std::find(Dependants().begin(), Dependants().end(), visID) != Dependants().end()) {
        visID = mVisID.PipelineDependantID(pipeElement + std::to_string(++n));
    }
    return visID;
}

std::string VisDisplayBase::CreateDependantWindowTitle(const VisID& dependantVisID) const
{
    auto pipeline = dependantVisID.Pipeline();
    if (pipeline.size() < 2)
        return "";

    std::string windowTitle;
    Visconfigs()[dependantVisID.PipelineParentID()].Get(CfgID::WindowTitle, windowTitle);

    std::string pipeElement = pipeline.back();
    int i = 0;
    while (i < pipeElement.length() && !std::isdigit(pipeElement[i])) {
        ++i;
    }
    if (i == pipeElement.length()) {
        windowTitle = pipeElement + " of " + windowTitle;
    }
    else {
        std::string num = pipeElement.substr(i);
        windowTitle = pipeElement.substr(0, i) + "(" + num + ") of " + windowTitle;
    }
    return windowTitle;
}

/////////////////////////////////////////////////////////
namespace {

    bool IsPaletteDark(const QPalette& palette)
    {
        return palette.text().color().lightness() > palette.window().color().lightness();
    }

    void InvertIcon(QPixmap& icon) // only works for greyscale images
    {
        QImage img = icon.toImage();
        for (int x = 0; x < img.width(); ++x) {
            for (int y = 0; y < img.height(); ++y) {
                QColor color = img.pixelColor(x, y);
                int h, s, v, a;
                color.getHsv(&h, &s, &v, &a);
                v = 255 - v;
                img.setPixelColor(x, y, QColor::fromHsv(h, s, v, a));
            }
        }
        icon = QPixmap::fromImage(img);
    }

}

bool VisDisplayBase::IsPaletteDark() const
{
    return ::IsPaletteDark(this->palette());
}

QPixmap VisDisplayBase::LoadIcon(bci::Resource& res, bool invert)
{
    QPixmap icon;
    icon.loadFromData(reinterpret_cast<const uchar*>(res.data), res.length);
    if (invert && ::IsPaletteDark(this->palette()))
        InvertIcon(icon);
    return icon;
}

/////////////////////////////////////////////////////////
VisDisplayBase::VisContainer &VisDisplayBase::Visuals()
{
    return sVisuals;
}

VisDisplayBase::ConfigContainer &VisDisplayBase::Visconfigs()
{
    return sVisconfigs;
}

void VisDisplayBase::Destroy(const VisID& inVisID)
{
    auto p = sVisuals[inVisID];
    if (p) {
        p->deleteLater();
        VisID parentID = inVisID.PipelineParentID();
        if (!parentID.Empty())
            HandleDependantDestroyed(parentID, inVisID);
    }
}

VisDisplayBase::VisDisplayBase(const VisID &inVisID) : QWidget(NULL), mVisID(inVisID)
{
    if (inVisID.IsLayer())
    {
        if (!VisDisplayBase::Visuals()[inVisID.WindowID()])
            Create<VisDisplayWindow>(inVisID.WindowID().c_str());
        this->setParent(VisDisplayBase::Visuals()[inVisID.WindowID()]);
    }
    setAttribute(Qt::WA_QuitOnClose, false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenu(QPoint)));
}

VisDisplayBase::~VisDisplayBase()
{
    sVisuals.erase(mVisID);
}

const VisID& VisDisplayBase::ID() const
{
    return mVisID;
}

void VisDisplayBase::SetVisible(bool b)
{
    QWidget::setVisible(b);
}

void VisDisplayBase::VisContainer::Clear()
{
    while (!empty())
    {
        VisDisplayBase *pVis = nullptr;
        iterator i = begin();
        while (begin() != end() && !begin()->second)
            erase(begin());
        if (begin() != end())
            pVis = begin()->second;
        delete pVis;
    }
}

void VisDisplayBase::SetConfig(ConfigSettings &inConfig)
{
    OnSetConfig(inConfig);
}

void VisDisplayBase::HandleSignal(const GenericSignal& signal)
{
    OnSignal(signal);
}

void VisDisplayBase::HandleMemo(const QString &inText)
{
    OnMemo(inText);
}

void VisDisplayBase::HandleBitmap(const BitmapImage &inBitmap)
{
    OnBitmap(inBitmap);
}

void VisDisplayBase::HandleVideoFrame(const VideoFrame& inFrame)
{
    OnVideoFrame(inFrame);
}

void VisDisplayBase::HandleProperty(CfgID cfgId, const char *value, ConfigState state)
{
    OnProperty(cfgId, value, state);
}

void VisDisplayBase::HandleDependantCreated(const VisID& dependant)
{
    Assert(std::find(mDependants.begin(), mDependants.end(), dependant) == mDependants.end());
    mDependants.push_back(dependant);
    OnDependantCreated(dependant);
}

void VisDisplayBase::HandleDependantDestroyed(const VisID& dependant)
{
    auto i = std::find(mDependants.begin(), mDependants.end(), dependant);
    if (i != mDependants.end())
        mDependants.erase(i);
    OnDependantDestroyed(dependant);
}

void VisDisplayBase::ContextMenu(const QPoint &p)
{
    OnContextMenu(p);
}

void VisDisplayBase::OnSetConfig(ConfigSettings &inConfig)
{
    bool visible = true;
    inConfig.Get(CfgID::Visible, visible);
    this->SetVisible(visible);
}

void VisDisplayBase::HandleSignal(const VisID &inVisID, const GenericSignal& inSignal)
{
    VisDisplayBase *p = Visuals()[inVisID];
    if (p) {
        p->HandleSignal(inSignal);
    }
}

void VisDisplayBase::HandleMemo(const VisID &inVisID, const std::string &inText)
{
    VisDisplayBase *p = Visuals()[inVisID];
    if (p) {
        p->HandleMemo(inText.c_str());
    }
}

void VisDisplayBase::HandleBitmap(const VisID &inVisID, const BitmapImage &inBitmap)
{
    VisDisplayBase *p = Visuals()[inVisID];
    if (p) {
        p->HandleBitmap(inBitmap);
    }
}

void VisDisplayBase::HandleVideoFrame(const VisID& inVisID, const VideoFrame& inVideoFrame)
{
    VisDisplayBase* p = Visuals()[inVisID];
    if (p) {
        p->HandleVideoFrame(inVideoFrame);
    }
}

void VisDisplayBase::HandleProperty(const VisID &inVisID, CfgID inCfgID, const std::string &inValue, int inState)
{
    ConfigSettings &c = Visconfigs()[inVisID];
    WithLock(c)
    {
        // If the special property ActivateBuffer=1 is sent, wait for ActivateBuffer=0 before
        // calling SetConfig() on the visualization object.
        // This is to avoid flickering of visualization windows.
        if (inCfgID == CfgID::ActivateBuffer) {
            if (::atoi(inValue.c_str()))
                c.IncBufferActive();
            else
                c.DecBufferActive();
        }
        else {
            c.Put(inCfgID, inValue, ConfigState(inState));
        }
        VisDisplayBase* p = Visuals()[inVisID];
        if (p && !c.BufferActive())
            p->SetConfig(c);
    }
}

void VisDisplayBase::HandlePropertyReset(const VisID& inVisID, CfgID inCfgID)
{
    ConfigSettings& c = Visconfigs()[inVisID];
    WithLock(c)
    {
        c.Erase(inCfgID);
        VisDisplayBase* p = Visuals()[inVisID];
        if (p)
            p->SetConfig(c);
    }
}

void VisDisplayBase::HandleDependantCreated(const VisID& inVisID, const VisID& inDependantVisID)
{
    VisDisplayBase* p = Visuals()[inVisID];
    if (p) {
        p->HandleDependantCreated(inDependantVisID);
    }
}

void VisDisplayBase::HandleDependantDestroyed(const VisID& inVisID, const VisID& inDependantVisID)
{
    VisDisplayBase* p = Visuals()[inVisID];
    if (p) {
        p->HandleDependantDestroyed(inDependantVisID);
    }
}

bool VisDisplayBase::ConfigSettings::Get(CfgID id, std::string &s, ConfigState minState)
{
    const_iterator i = find(id);
    if (i == end())
        return false;
    if (State(id) < minState)
        return false;
    s = i->second;
    return true;
}

bool VisDisplayBase::ConfigSettings::Put(CfgID id, const std::string &s, ConfigState state)
{
    if (State(id) > state)
        return false;
    State(id) = state;
    (*this)[id] = s;
    return true;
}

void VisDisplayBase::ConfigSettings::Erase(CfgID id)
{
    mStates.erase(id);
    erase(id);
}
