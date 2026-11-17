//////////////////////////////////////////////////////////////////////
// $Id: DialogType.cpp 9566 2026-09-21 13:11:26Z mellinger $
// Authors: hill@neurotechcenter.org
// Description: Scripting extension for prompting the user with
//              customizable dialogs.
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
#include "DialogType.h"

#include "CustomDialog.h"
#include "ParserToken.h"
#include "MainWindow.h"
#include "EnvVariable.h"

#include <QMetaType>
#include <QMetaObject>
#include <QGenericArgument>
#include <QString>


namespace ScriptingExtensions {
    DialogType sDialogTypeInstance;
    const ObjectType::MethodEntry* DialogType::MethodTable() const
    {
        static const ObjectType::MethodEntry table[] = { {"Custom", &Custom}, {0} };
        return table;
    }
    bool DialogType::Custom(std::istream& in, std::ostream& out, std::ostream& err)
    {
        QString qResult;
        CustomDialogSpec spec;
        
        if(!spec.Build(in, err)) return false;
        std::lock_guard<std::mutex> waitFor(MainWindow::Instance()->GetModalDialogMutex());
        if (MainWindow::Instance()->Terminating()) return true;
        QMetaObject::invokeMethod(
            MainWindow::Instance(),
            "CustomDialog",
            Qt::BlockingQueuedConnection,
            Q_RETURN_ARG(QString, qResult),
            Q_ARG(CustomDialogSpec*, &spec)
            // NB: passing a custom class pointer as a Q_ARG requires
            //     Q_DECLARE_METATYPE(CustomDialogSpec*)
            // in the header, and one module needs to do
            //     static int _ev_ptr_id = qRegisterMetaType<CustomDialogSpec*>("CustomDialogSpec*");
        );
        std::string result = qResult.toUtf8().constData();
        out << result;
        return true;
    }
}
