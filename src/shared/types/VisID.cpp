////////////////////////////////////////////////////////////////////////////////
// $Id: VisID.cpp 9470 2026-07-18 20:56:43Z mellinger $
// Author: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
// Description: A class representing a Visualization ID.
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
#include "VisID.h"

#include "BCIStream.h"

static const char kLayerSeparator = ':';
static const char kPipelineSeparator = '|';

VisID::VisID()
{
}

VisID::VisID(const std::string &s) : EncodedString(s)
{
}

VisID::VisID(const char *s) : EncodedString(s)
{
}

VisID &VisID::SetWindowID(const std::string &s)
{
    if (s.find(kLayerSeparator) != std::string::npos)
        bcierr << "Window IDs may not contain the '" << kLayerSeparator << "' character";

    if (IsLayer())
        *this = s + kLayerSeparator + LayerID();
    else
        *this = s;
    return *this;
}

std::string VisID::WindowID() const
{
    return Substr(0, Find(kLayerSeparator));
}

VisID &VisID::SetLayerID(const std::string &s)
{
    return *this = WindowID() + kLayerSeparator + s;
}

std::string VisID::LayerID() const
{
    size_t pos = Find(kLayerSeparator);
    if (pos != std::string::npos)
        return Substr(pos).Substr(1);
    return std::string();
}

bool VisID::IsLayer() const
{
    return Find(kLayerSeparator) != std::string::npos;
}

VisID &VisID::ToLayer()
{
    return SetLayerID(LayerID());
}

bool VisID::IsDependant() const
{
    return Pipeline().size() > 1;
}

VisID& VisID::SetPipeline(const std::vector<std::string>& elements)
{
    if (elements.empty())
        bcierr << "Pipeline cannot be empty";

    std::string sep(1, kPipelineSeparator);
    auto pipeline = StringUtils::Join(sep, elements);
    SetWindowID(pipeline);
    return *this;
}

std::vector<std::string> VisID::Pipeline() const
{
    std::string sep(1, kPipelineSeparator);
    return StringUtils::Split(sep, WindowID());
}

VisID VisID::PipelineParentID() const
{
    auto pipeline = Pipeline();
    if (pipeline.size() < 2) {
        return VisID();
    }
    pipeline.pop_back();
    VisID result = *this;
    result.SetPipeline(pipeline);
    return result;
}

VisID VisID::PipelineDependantID(const std::string& dependant) const
{
    auto pipeline = Pipeline();
    Assert(!dependant.empty());
    pipeline.push_back(dependant);
    VisID result = *this;
    result.SetPipeline(pipeline);
    return result;
}
