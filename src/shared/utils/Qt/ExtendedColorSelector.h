////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A color chooser dialog augmented with elements useful for
//   selecting channel colors.
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
#ifndef EXTENDED_COLOR_SELECTOR_H
#define EXTENDED_COLOR_SELECTOR_H

#include <QWidget>

class ExtendedColorSelector : public QWidget
{
    Q_OBJECT

public:
    ExtendedColorSelector(QWidget* parent = nullptr);
    ~ExtendedColorSelector();
    static void setCustomColors(const std::vector<QColor>&);
    static std::vector<QColor> customColors();

signals:
    void colorApplied(const QColor&);
    void colorPatternApplied(const QString&);
    void resetClicked();
    void closed();

public slots:
    void setApplyColorButtonEnabled(bool);

private slots:
    void onPushButtonClicked();
    void onCurrentColorChanged(const QColor&);

protected:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    class QPushButton* mpApplyColor, *mpApplyColorPattern;
    class QColorDialog* mpColorDialog;
    class QComboBox* mpComboBox;
    class PreviewWidget;
    PreviewWidget* mpDarkPreview, *mpLightPreview;
};

#endif // EXTENDED_COLOR_SELECTOR_H
