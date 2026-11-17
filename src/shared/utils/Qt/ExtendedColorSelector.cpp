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
#include "ExtendedColorSelector.h"

#include <QColorDialog>
#include <QBoxLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QPainter>

#include <random>
#include <set>
#include "Color.h"

namespace {
    struct CustomColorDialog : QColorDialog
    {
        void showEvent(QShowEvent*) override
        {
            layout()->setContentsMargins(0, 0, 0, 0);
        }
        void hideEvent(QHideEvent* iopEvent) override
        {
            show();
            QWidget* pWidget = this;
            while (pWidget && pWidget->parentWidget())
                pWidget = pWidget->parentWidget();
            if (pWidget)
                pWidget->close();
        }
    };
} // namespace

class ExtendedColorSelector::PreviewWidget : public QWidget
{
public:
    PreviewWidget()
    {
        if (sExampleData.empty()) {
            sExampleData.reserve(cExampleDataCount);
            std::default_random_engine generator;
            std::normal_distribution<float> distribution(0.0, 0.25);
            while(sExampleData.size() < cExampleDataCount)
                sExampleData.push_back(distribution(generator));
        }
    }

    QSize sizeHint() const override
    {
        return QSize(160, 120);
    }

    void setForeground(const QColor& c)
    {
        RGBColor background = RGBColor::FromQRgb(mBackground.rgb()) & 0xffffff;
        RGBColor foreground = RGBColor::FromQRgb(c.rgb() & 0xffffff);
        foreground = RGBColor::EnsureColorContrast(foreground, background);
        mForeground = foreground.ToQRgb();
        update();
    }

    void setBackground(const QColor& c)
    {
        mBackground = c;
        update();
    }

protected:
    void paintEvent(QPaintEvent* iopEvent) override
    {
        QPainter painter(this);
        painter.fillRect(iopEvent->rect(), mBackground);
        float height = iopEvent->rect().height();
        float width = iopEvent->rect().width();
        QPointF* pPoints = new QPointF[sExampleData.size()];
        for (int i = 0; i < sExampleData.size(); ++i) {
            pPoints[i].setX(i * width / (sExampleData.size() - 1));
            pPoints[i].setY(height / 2 + height / 4 * sExampleData[i]);
        }
        painter.setPen(mForeground);
        painter.drawPolyline(pPoints, sExampleData.size());
        delete[] pPoints;
    }

private:
    static const size_t cExampleDataCount = 512;
    static std::vector<float> sExampleData;
    QColor mBackground = Qt::black, mForeground = Qt::white;
};

std::vector<float> ExtendedColorSelector::PreviewWidget::sExampleData;

static std::set<ExtendedColorSelector*> sInstances;

ExtendedColorSelector::ExtendedColorSelector(QWidget* parent)
: QWidget(parent)
{
    sInstances.insert(this);

    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    mpColorDialog = new CustomColorDialog;
    mpColorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
    connect(mpColorDialog, &QColorDialog::currentColorChanged, this, &ExtendedColorSelector::onCurrentColorChanged);

    QVBoxLayout* pLayout = new QVBoxLayout;
    QGroupBox* pGroupBox = new QGroupBox("Color chooser");
    pLayout->addWidget(pGroupBox);
    QBoxLayout* pSubLayout = new QVBoxLayout;
    pSubLayout->addWidget(mpColorDialog);
    QBoxLayout* pSubSubLayout = new QHBoxLayout;

    QGroupBox* pGroupBox2 = new QGroupBox("Dark mode preview");
    pGroupBox2->setLayout(new QHBoxLayout);
    mpDarkPreview = new PreviewWidget;
    mpDarkPreview->setBackground(RGBColor::Black);
    mpDarkPreview->setForeground(mpColorDialog->currentColor());
    mpDarkPreview->setToolTip(
        "This is a preview of the color how it appears in dark mode.\n"
        "If you select black as a signal color, it will appear white to ensure contrast."
    );
    pGroupBox2->layout()->addWidget(mpDarkPreview);
    pSubSubLayout->addWidget(pGroupBox2);

    pGroupBox2 = new QGroupBox("Light mode preview");
    pGroupBox2->setLayout(new QHBoxLayout);
    mpLightPreview = new PreviewWidget;
    mpLightPreview->setBackground(RGBColor::LtYellow);
    mpLightPreview->setForeground(mpColorDialog->currentColor());
    mpLightPreview->setToolTip(
        "This is a preview of the color how it appears in light mode.\n"
        "Colors close to the background color are modified to ensure contrast."
    );
    pGroupBox2->layout()->addWidget(mpLightPreview);
    pSubSubLayout->addWidget(pGroupBox2);

    pSubLayout->addLayout(pSubSubLayout);
    mpApplyColor = new QPushButton("Apply Color to Selected Channels");
    connect(mpApplyColor, &QPushButton::clicked, this, &ExtendedColorSelector::onPushButtonClicked);
    pSubLayout->addWidget(mpApplyColor);
    pGroupBox->setLayout(pSubLayout);

    pSubLayout = new QVBoxLayout;
    pSubSubLayout = new QHBoxLayout;
    pSubSubLayout->addWidget(new QLabel("Use colors of first"));
    mpComboBox = new QComboBox;
    mpComboBox->addItems({"2", "3", "4", "5", "8", "12", "16"});
    mpComboBox->setCurrentText("5");
    pSubSubLayout->addWidget(mpComboBox);
    pSubSubLayout->addWidget(new QLabel("channels repetitively"));
    pSubSubLayout->addStretch();
    pSubLayout->addLayout(pSubSubLayout);
    mpApplyColorPattern = new QPushButton("Apply Pattern to All Channels");
    connect(mpApplyColorPattern, &QPushButton::clicked, this, &ExtendedColorSelector::onPushButtonClicked);
    pSubLayout->addWidget(mpApplyColorPattern);

    pGroupBox = new QGroupBox("Color pattern");
    pGroupBox->setLayout(pSubLayout);
    pLayout->addWidget(pGroupBox);

    pSubLayout = new QHBoxLayout;
    pGroupBox = new QGroupBox("Undo changes");
    pGroupBox->setLayout(pSubLayout);
    pLayout->addWidget(pGroupBox);
    QPushButton* pButton = new QPushButton("Undo All Channel Color Changes");
    pSubLayout->addWidget(pButton);
    connect(pButton, &QPushButton::clicked, this, &ExtendedColorSelector::resetClicked);

    setLayout(pLayout);
}

ExtendedColorSelector::~ExtendedColorSelector()
{
    sInstances.erase(this);
}

void ExtendedColorSelector::setCustomColors(const std::vector<QColor>& colors)
{
    for (int i = 0; i < std::min<int>(colors.size(), QColorDialog::customCount()); ++i)
        QColorDialog::setCustomColor(i, colors[i]);
    for (int i = colors.size(); i < QColorDialog::customCount(); ++i)
        QColorDialog::setCustomColor(i, Qt::transparent);
}

std::vector<QColor> ExtendedColorSelector::customColors()
{
    std::vector<QColor> colors;
    colors.reserve(QColorDialog::customCount());
    for (int i = 0; i < QColorDialog::customCount(); ++i)
        colors.push_back(QColorDialog::customColor(i));
    return colors;
}

void ExtendedColorSelector::setApplyColorButtonEnabled(bool enabled)
{
    mpApplyColor->setEnabled(enabled);
}

void ExtendedColorSelector::onPushButtonClicked()
{
    if (sender() == mpApplyColorPattern)
        emit colorPatternApplied(mpComboBox->currentText());
    else if (sender() == mpApplyColor)
        emit colorApplied(mpColorDialog->currentColor());
}

void ExtendedColorSelector::onCurrentColorChanged(const QColor& c)
{
    mpDarkPreview->setForeground(c);
    mpLightPreview->setForeground(c);
}

void ExtendedColorSelector::showEvent(QShowEvent*)
{
    for (auto pInstance : sInstances) {
        if (pInstance != this && pInstance->pos() == this->pos())
            move(this->pos() + QPoint(10, 10));
    }
}

void ExtendedColorSelector::closeEvent(QCloseEvent* iopEvent)
{
    iopEvent->accept();
    emit closed();
}

void ExtendedColorSelector::keyPressEvent(QKeyEvent* iopEvent)
{
    if (iopEvent->key() == Qt::Key_Escape) {
        iopEvent->accept();
        close();
        return;
    }

    QWidget::keyPressEvent(iopEvent);
}

