/***************************************************************************
 *   Copyright (C) 2007 by J. Adam Wilson   *
 *   jawilson@cae.wisc.edu   *
 *   juergen.mellinger@uni-tuebingen.de: fixed a parsing issue,
 *                                       added receiving thread.
 *   mellinger@neurotechcenter.org: changed underlying logic to use
 *                                  BCI2000Remote interface
 *                                                                         *
 * $BEGIN_BCI2000_LICENSE$
 *
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
 *
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_BCI2000_LICENSE$
 ***************************************************************************/

#include "mainUI.h"
#include <sstream>

mainUI::mainUI()
{
    // setupUI is used to create the form and layouts
    setupUI();

    //in this program, the two major components are two group boxes
    //this first one handles the connection to BCI2000, and the second
    //provides info about the states being read.
    //Here we organize these in a vertical box layout, so that the connection
    //group appears above the state info group.
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(connectGroup, 0);
    mainLayout->addWidget(statesGroup, 0);
    setLayout(mainLayout);
    setWindowTitle("Watches Example");
    resize(320, 800);
}

mainUI::~mainUI()
{
}

//---------------------------------------------------------------
void mainUI::connectBtnCallback()
{
    //this SLOT is called when the connect button is clicked
    //the connections and sockets are first cleared and closed.
    //then the program attempts to connect to BCI2000 using the
    //IP and ports specified.
    mConnection.Disconnect();
    mConnection.SetOperatorPath(""); // don't start BCI2000 if it is not running
    mConnection.SetTelnetAddress(mpAddressBox->text().toLocal8Bit().constData());
    if (!mConnection.Connect()) {
        mpStatusBox->setText("Could not connect to Operator address");
        return;
    }

    mpStatusBox->setText("Connected.");
    std::string result;
    if (!mConnection.Execute("LIST STATES", nullptr, &result)) {
        mpStatusBox->setText("Could not retrieve list of states");
        return;
    }
    std::istringstream iss(result);
    std::vector<std::string> names;
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream linestream(line);
        std::string name;
        linestream >> name;
        names.push_back(name);
    }

    double value;
    mStates.clear();
    for (const auto& name : names) {
        StateEntry& entry = mStates[name];
        entry.pParent = this;
        entry.readonly = false;
        if (mConnection.GetStateVariable(name, value)) {
            std::ostringstream oss;
            oss << value;
            entry.value = oss.str();
        }
        entry.pWatch.reset(mConnection.NewWatch(name, &onWatchUpdate, &entry));
    }

    double channels, elements;
    if (mConnection.GetControlSignalChannels(channels) && mConnection.GetControlSignalElements(elements)) {
        for (int ch = 1; ch <= channels; ++ch) {
            for (int el = 1; el <= elements; ++el) {
                std::ostringstream name;
                name << "Signal(" << ch << "," << el << ")";
                StateEntry& entry = mStates[name.str()];
                entry.pParent = this;
                entry.readonly = true;
                if (mConnection.GetControlSignal(ch, el, value)) {
                    std::ostringstream oss;
                    oss << value;
                    entry.value = oss.str();
                }
                entry.pWatch.reset(mConnection.NewWatch(name.str(), &onWatchUpdate, &entry));
            }
        }
    }

    //if we successfully connected, then start the timer
    timer->start();
}

void mainUI::timerUpdate()
{
    //update state display
    std::unique_lock<std::mutex> lock(mMutex);
    if (mStates.size() != mpStateTable->rowCount())
    {
        mpStateTable->setRowCount(static_cast<int>(mStates.size()));
        for (int row = 0; row < mpStateTable->rowCount(); ++row)
        {
            mpStateTable->setItem(row, 0, new QTableWidgetItem);
            mpStateTable->setItem(row, 1, new QTableWidgetItem);
        }
    }
    int row = 0;
    for (const auto& state : mStates)
    {
        mpStateTable->item(row, 0)->setText(state.first.c_str());
        mpStateTable->item(row, 1)->setText(state.second.value.c_str());
        ++row;
    }
}

// Callback function for watch update
void STDCALL mainUI::onWatchUpdate(void* data, const char* value)
{
    while (*value && *value != '\t')
        ++value;
    if (*value == '\t')
        ++value;
    auto pEntry = static_cast<mainUI::StateEntry*>(data);
    std::unique_lock<std::mutex> lock(pEntry->pParent->mMutex);
    pEntry->value.clear();
    while (*value && *value != '\r')
        pEntry->value += *value++;
}

//---------------------------------------------------------------
void mainUI::writeValue(const std::string& name, double value)
{
    //this function writes a value to a state in BCI2000
    //Isn't this easy?!
    mConnection.SetStateVariable(name, value);
}

//----------------------------------------------
void mainUI::changeStateValue(int row, int column)
{
    //we only want to send a new value, not change the state name
    mpStatusBox->setText("Changing value...");
    if (column == 0)
        return;
    bool ok1, ok2;

    if (!mpStateTable->item(row, 0) || !mpStateTable->item(row, 1))
        return;

    std::string name = mpStateTable->item(row, 0)->text().toLocal8Bit().constData();
    std::unique_lock<std::mutex> lock(mMutex);
    // the lock may not be held when QMessageBox runs its exec() event loop
    // because that will call timerUpdate() which then wants to acquire the lock again
    bool isReadonly = mStates[name].readonly;
    lock.unlock();
    if (isReadonly) {
        mpStatusBox->setText("Read-only entry...");
        QMessageBox::information(this, "Error", "This entry is read-only");
        return;
    }

    std::stringstream str1;
    short value = mpStateTable->item(row, 1)->text().toShort(&ok1);

    str1 << "Enter a new value for " << name;

    short newValue = QInputDialog::getInt(this, "", str1.str().c_str(), value, -32767, 32767, 1, &ok2);

    if (ok1 && ok2)
    {
        writeValue(name, newValue);
        std::stringstream str;
        str << "Wrote " << name << ": " << newValue << std::endl;
        mpStatusBox->setText(str.str().c_str());
    }
    else
        mpStatusBox->setText("There was an error...");

}

void mainUI::setupUI()
{
    //this function sets up both group boxes, and sets the timer interval
    setupConnectionGroup();
    setupStatesGroup();
    resize(minimumSizeHint());

    timer = new QTimer(this);
    //here, the timer SIGNAL is connected to the timerUpdate SLOT
    timer->connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->setInterval(100);
    timer->stop();
}

void mainUI::setupConnectionGroup()
{
    //create the connection group box and setup the signals/slots
    connectGroup = new QGroupBox("Operator Connection");
    //connectGroup->setGeometry(QRect(10, 180, 150, 150));

    connectLayout = new QVBoxLayout;

    mpAddressLabel = new QLabel("Operator IP:Port");
    mpAddressBox = new QLineEdit();
    mpAddressBox->setText("localhost:3999");
    pConnectBtn = new QPushButton("Create Connection");
    mpStatusBox = new QLineEdit;
    connect(pConnectBtn, SIGNAL(clicked()), this, SLOT(connectBtnCallback()));

    connectLayout->addWidget(mpAddressLabel, 0);
    connectLayout->addWidget(mpAddressBox, 0);
    connectLayout->addWidget(pConnectBtn, 0, Qt::Alignment(4));
    connectLayout->addWidget(mpStatusBox);

    connectGroup->setLayout(connectLayout);
}

void mainUI::setupStatesGroup()
{
    statesGroup = new QGroupBox("States");
    //statesGroup->setGeometry(QRect(10, 10, 251, 171));

    statesLayout = new QGridLayout;
    statesLayout->setSpacing(6);
    statesLayout->setContentsMargins(0, 0, 0, 0);

    mpStateTable = new QTableWidget;
    mpStateTable->setColumnCount(2);
    mpStateTable->setRowCount(1);
    QStringList headers;
    headers << "State" << "Value";
    mpStateTable->setHorizontalHeaderLabels(headers);
    mpStateTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //add the signal/slot so that if a table value is double clicked, a dialog will appear
    mpStateTable->connect(mpStateTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(changeStateValue(int, int)));

    statesLayout->addWidget(mpStateTable, 0, 0);

    statesGroup->setLayout(statesLayout);
}
