////////////////////////////////////////////////////////////////////////////////
// Authors: Luciano Branco - CNELAB - Fri May 26 14:06:46 2023 - LAST EDIT 04082025
//          Matthew Hall   - CNELAB - LAST EDIT 04222025
// Description: BehvLogger connects and reads behavioral data from the - LB
// application originally described in
// https://ieeexplore.ieee.org/document/8037190
// Communication to Device Command works through Server - MH
// IP addresses can be remote computers - MH
// Event Watch can be a variable - MH
////////////////////////////////////////////////////////////////////////////////

#include "BehvLogger.h"
#include "BCIEvent.h"
#include "BCIStream.h"
#include <chrono>
using namespace std;


#if 1 // remove the code in this #if statement when you are done
// This is an example API which does nothing. It is just there to make the example
// code compile.
void BehvLogger::CnelBehvConnect(const std::string _UDPstring)
{
    if (init_udp(stoi(_UDPstring)))
        bciwarn << "Successfully connected UDPPortCnelBehv to " << _UDPstring;
    else
        bciwarn << "Failed to connect UDPPortCnelBehv to " << _UDPstring;

    //m_thread.join();
    if (!mRunning)
        m_thread = std::thread(&BehvLogger::AcquireData, this, 0);
}
void BehvLogger::CnelBehvDisconnect()
{
    mRunning = false;
    m_kill_thread = true;
    if (m_thread.joinable()) // avoid program abort if thread is not running
        m_thread.join();
}

//Server Connection
void BehvLogger::CnelBehvServerConnect(const std::string _UDPstring)
{
    if (init_udp_server(stoi(_UDPstring)))
        bciwarn << "Successfully connected UDPPortCnelBehvServer to " << _UDPstring;
    else
        bciwarn << "Failed to connect UDPPortCnelBehvServer to " << _UDPstring;

    if (!sRunning)
        m_threadServer = std::thread(&BehvLogger::SendServerString, this, 1);
}

void BehvLogger::CnelBehvServerDisconnect()
{
    sRunning = false;
    m_kill_threadServer = true;
    if (m_threadServer.joinable()) // avoid program abort if thread is not running
        m_threadServer.join();
}
static void ExampleAPIRegisterCallback(void*, void (*)(int, void*), void*) {}
static void ExampleAPIUnregisterCallback(void*, void (*)(int, void*), void*) {}
#endif

// Register the extension with the framework.
Extension(BehvLogger);

// C++ does not initialize simple types such as numbers, or pointers, by default.
// Rather, they will have random values.
// Take care to initialize any member variables here, so they have predictable values
// when used for the first time.
BehvLogger::BehvLogger()
    : mLogBehvLogger(false),
    m_kill_thread(false),
    mRunning(false),
    SenderAddrSize(sizeof(_udpReceiver)),
    bytesRecvd(0),
    sRunning(false),//server options
    m_kill_threadServer(false),
    initalSleep(true),
    doScreenshot(false)
{
    mPreviousPos = -1;
    udpSocket = 0;
    udpSocketServer = 0;
}

BehvLogger::~BehvLogger()
{
    CnelBehvDisconnect();
    CnelBehvServerDisconnect();
    // If you have allocated any memory with malloc(), call free() here.
    // If you have allocated any memory with new[], call delete[] here.
    // If you have created any object using new, call delete here.
    // Either kind of deallocation will silently ignore null pointers, so all
    // you need to do is to initialize those to zero in the constructor, and
    // deallocate them here.
    
}

void
BehvLogger::Publish()
{
    if (OptionalParameter("/LogBehv") > 0)
    {
        BEGIN_PARAMETER_DEFINITIONS
            "Source:LogBehv int /LogBehv= 1 0 0 1 // Connect and record from Cnel Behv (boolean)",
            "Source:LogBehv string IPCnelBehv= 127.0.0.1  // IP for Cnel Behv Server (from win.ini file)",
            "Source:LogBehv string UDPPortCnelBehv= 27004  // UDP Port from Cnel Behv (from win.ini file)",
            "Source:LogBehv int /ConnectServerBehv= 1 0 0 1 // Send commands to Cnel Behv (boolean)",
            "Source:LogBehv string IPCnelBehvServer= 127.0.0.1  // IP for Cnel Behv Server (from win.ini file)",
            "Source:LogBehv string UDPPortCnelBehvServer= 27100  // UDP Port for Cnel Behv Server (from win.ini file)",
            "Source:LogBehv string CnelServerStateName= CnelServerState  // The State to watch for Code Match in CnelServerMatrix",
            "Source:LogBehv matrix CnelServerMatrix= 0 [ Code Expression Title] // State to Expression Code String",
            "Source:LogBehv int /doScreenshot= 1 0 0 1 // Turn on and off periodic Screenshots (boolean)",
            "Source:LogBehv int ScreenshotSeconds= 5 0 0 3600 // How often to screenshot in Seconds",
            "Source:LogBehv string ScreenshotDirectory= C:/RecordingTest/Screenshot/  // Directory to save screenshots",
        END_PARAMETER_DEFINITIONS

        

        // declare CNEL_BEHV_PROTOCOL_MAX_MSG Cnel Behv channels
        for (int i = 0; i < CNEL_BEHV_PROTOCOL_MAX_MSG; i++)
        {
            std::ostringstream definition;
            definition << "CnelBehv" << i << " 32 0 0 0" << std::flush;
            BEGIN_EVENT_DEFINITIONS
                definition.str().c_str(),
            END_EVENT_DEFINITIONS
        }
        /* // not working early definition of state name
        std::ostringstream statedefinition;
        statedefinition << Parameter("CnelServerStateName") << " 32 0 0 0" << std::flush;
        BEGIN_STATE_DEFINITIONS
            statedefinition.str().c_str(),
        END_STATE_DEFINITIONS
        */
        BEGIN_STATE_DEFINITIONS
            "CnelServerState 32 0 0 0"
        END_STATE_DEFINITIONS

    }
}

void
BehvLogger::Preflight() const
{
    if (OptionalParameter("/LogBehv") == 0) return;
    // The user has pressed "Set Config" and we need to sanity-check everything.
    // For example, check that all necessary parameters and states are accessible,
    // also check that the values of any parameters are sane:
    //
    // if( (float)Parameter( "Denominator" ) == 0.0f )
    //      bcierr << "Denominator cannot be zero";
    //
    // Errors issued in this way, during Preflight, still allow the user to open
    // the Config dialog box, fix bad parameters and re-try.  By contrast, errors
    // and C++ exceptions at any other stage (outside Preflight) will make the
    // system stop, such that BCI2000 will need to be relaunched entirely.

    // Note that the BehvLogger instance itself, and its members, are read-only during
    // this phase, due to the "const" at the end of the Preflight prototype above.
    // Any methods called by Preflight must also be "const" in the same way.

    // In this example, we have no configuration parameters other than on/off, so
    // there is nothing to do here.

    if (((int)Parameter("UDPPortCnelBehv")) <= 0)
        bcierr << "UDPPortCnelBehv cant be equal or below zero.";
    Parameter("UDPPortCnelBehv");
    Parameter("IPCnelBehv");

    if (((int)Parameter("UDPPortCnelBehvServer")) <= 0)
        bcierr << "UDPPortCnelBehvServer cant be equal or below zero.";
    Parameter("UDPPortCnelBehvServer");
    Parameter("IPCnelBehvServer");
    
    Parameter("CnelServerStateName");
    Parameter("CnelServerMatrix");

    State("CnelServerState");
    State(Parameter("CnelServerStateName"));
    State("Running");
    Parameter("/doScreenshot");
    Parameter("ScreenshotSeconds");
    Parameter("ScreenshotDirectory");
}

void
BehvLogger::Initialize()
{
    if (OptionalParameter("/LogBehv") == 0) return;
    // The user has pressed "Set Config" and all Preflight checks have passed.
    // The const limitation has gone, so the BehvLogger instance itself can be modified.
    // If your hardware requires a connection, it is now time to connect.
    // Note that Initialize() may be called multiple times in a row, so don't open the connection
    // if it still exists.
    
    if (!udpSocket && !IsConnectionActive())
        CnelBehvConnect(Parameter("UDPPortCnelBehv").ToString());

    
    isParsed = parseServerMatrix(); // must parse before thread bind
    ServerEventName = Parameter("CnelServerStateName");
    currEvent = 0;
    State("CnelServerState") = 100;
    if (isParsed && !udpSocketServer && !IsServerConnectionActive())
        CnelBehvServerConnect(Parameter("UDPPortCnelBehvServer").ToString());
    if (Parameter("/doScreenshot")) {
        doScreenshot = true;
    } else {
        doScreenshot = false;
    }
    
    screenshotS = Parameter("ScreenshotSeconds"); // how often to screenshot in milliseconds
    ScreenshotDirectory= Parameter("ScreenshotDirectory"); // default to run directory and use with filename
    RecordingFilename;
    if (doScreenshot) { //create screenshot folder

        std::string currentPath;
        size_t pos = 0;

        while ((pos = ScreenshotDirectory.find_first_of("\\/", pos)) != std::string::npos)
        {
            currentPath = ScreenshotDirectory.substr(0, pos++);
            if (currentPath.empty()) continue;

            CreateDirectoryA(currentPath.c_str(), NULL);
        }
    }
}

bool
BehvLogger::IsConnectionActive()
{
    char byte;
    int iResult = recvfrom(udpSocket,
        &byte, 1, MSG_PEEK, (SOCKADDR*)&_udpReceiver, &(int)SenderAddrSize);
    // by using MSG_PEEK we dont remove data in this test read.

    if (iResult == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return true; // No data, but not a failure
        // Optional: log or handle other errors
        return false;
    }

    return true;
}
bool
BehvLogger::IsServerConnectionActive()
{
    char byte;
    int iResult = recvfrom(udpSocketServer,
        &byte, 1, MSG_PEEK, (SOCKADDR*)&_udpReceiverServer, &(int)SenderAddrSize);
    // by using MSG_PEEK we dont remove data in this test read.

    if (iResult == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return true; // No data, but not a failure
        // Optional: log or handle other errors
        return false;
    }
    return true;
}

bool
BehvLogger::init_udp(const int port)
{
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        bcierr << "Failed to create UDP Socket" << endl;
    }

    //// 1. Set Non-Blocking Mode
    //u_long mode = 1; // 1 to enable non-blocking mode
    //if (ioctlsocket(udpSocket, FIONBIO, &mode) != NO_ERROR)
    //{
    //    closesocket(udpSocket);
    //    bcierr << "Failed to set non-blocking mode" << endl;
    //}

    //// 2. Set Receive Buffer Size (optional, adjust as needed)
    //int recvBufSize = 65535; // Example size; adjust based on your needs
    //if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBufSize, sizeof(recvBufSize)) == SOCKET_ERROR)
    //{
    //    closesocket(udpSocket);
    //    bcierr << "Failed to set receive buffer size" << endl;
    //}

    _udpReceiver.sin_family = AF_INET;
    _udpReceiver.sin_port = htons(port);
    _udpReceiver.sin_addr.s_addr = htonl(INADDR_ANY);
    std::string ipString = Parameter("IPCnelBehv");
    std::wstring wIp(ipString.begin(), ipString.end());

    int res = InetPtonW(AF_INET, wIp.c_str(), &_udpReceiverServer.sin_addr);
    if (res <= 0)
    {
        bcierr << "Invalid IP address: " << ipString;
        return false;
    }
    bciout << "Behv IP address: " << ipString;
    int iResult = bind(udpSocket, (SOCKADDR*)&_udpReceiver, sizeof(_udpReceiver));
    if (iResult == SOCKET_ERROR)
    {
        std::string errormsg = "Failed to bind UDP Socket: " + std::to_string(WSAGetLastError());
        return false;
    }
    return true;
}
bool
BehvLogger::init_udp_server(const int port)
{
    udpSocketServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocketServer == INVALID_SOCKET)
    {
        bcierr << "Failed to create UDP Socket for server " << endl;
        return false;
    }

    memset(&_udpReceiverServer, 0, sizeof(_udpReceiverServer));
    _udpReceiverServer.sin_family = AF_INET;
    _udpReceiverServer.sin_port = htons(port); //
    std::string ipString = Parameter("IPCnelBehvServer");
    std::wstring wIp(ipString.begin(), ipString.end());
    bciout << "Behv Server IP address: " << ipString;
    int res = InetPtonW(AF_INET, wIp.c_str(), &_udpReceiverServer.sin_addr);
    if (res <= 0)
    {
        bcierr << "Invalid IP address: " << ipString;
        return false;
    }
    return true;
}


int
BehvLogger::getNextUDPPackage(double* recvData)
{
    if (udpSocket == INVALID_SOCKET)
    {
        bcierr << "Tried to receive a UDP package while no active connection is available!" << endl;
    }

    char RecvBuff[CNEL_BEHV_PROTOCOL_MAX_MSG * 8];  // 8 bytes per received Behv channel (double)
    int BuffLen = CNEL_BEHV_PROTOCOL_MAX_MSG * 8;

    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams on the bound socket.
    int iResult = recvfrom(udpSocket,
        RecvBuff, BuffLen, 0, (SOCKADDR*)&_udpReceiver, &(int)SenderAddrSize);

    if (iResult == SOCKET_ERROR) {
        bcierr << "Error reading Cnel UDP \n" << endl;
    }

    // Converts char* to double* -- since every 8 bytes == 1 double
    memcpy(recvData, &RecvBuff, sizeof(double) * (iResult / 8));

    return iResult;
}


void
BehvLogger::AcquireData(int _thread)
{
    if (OptionalParameter("/LogBehv") == 0) return;

    while (!m_kill_thread)
    {
        //bciwarn << "thread" << endl;
        if (mRunning) {

            bytesRecvd = getNextUDPPackage(recvData);

            if (bytesRecvd == 0)
                continue;

            // We have data here! Send to Events.

            //bcievent << "BehvLoggerPos " << recvData[5];
            for (int i = 0; i < (bytesRecvd/8); i++) {
                if (!mRunning)
                    continue; // Skip this read.
                bcievent << "CnelBehv" << i << " " << recvData[i];
            }

            Sleep(1);
            // Should be able to sleep a bit here to save computational resources.
            // The reason is that since we just got a valid UDP msg, we wont receive another one in a while.
            // However, should still be way smaller than sampling period so we can catch up and avoid delays.
        }
        else {
            //mDevice->readSerialPort(largeBuff, 1024);
            //Sleep(0);
        }
    }
}
bool
BehvLogger::parseServerMatrix()
{
    
    if (Parameter("CnelServerMatrix")->NumValues() > 0)
    {
        ParamRef CnelServerMatrix = Parameter("CnelServerMatrix");
        

        if (CnelServerMatrix->NumColumns() < 3)
        {
            bcierr << "CnelServerMatrix must have at least 3 columns per row.";
            return false;
        }

        numRows = CnelServerMatrix->NumRows();

        mMatrixCodes.resize(numRows);
        hasSent.resize(numRows);

        mMatrixExpressions.resize(numRows);
        mMatrixTitles.resize(numRows);

        for (int i = 0; i < numRows; ++i)
        {
            double incode = CnelServerMatrix(i, "Code"); // code
            std::string inExpression = CnelServerMatrix(i, "Expression"); // expression
            std::string inTitle = CnelServerMatrix(i, "Title"); // title

            mMatrixCodes[i] = incode;
            mMatrixExpressions[i] = inExpression;
            mMatrixTitles[i] = inTitle;
            hasSent[i] = 0;

            bciout << "BehvServer(" << inTitle << "):"
                << " Event Code " << incode
                << " sends " << inExpression;
        }

        return true;
    }
    if (Parameter("doScreenshot")->NumValues() > 0) {
        return true;
    }
    bciwarn << "No Values in CnelServerMatrix and no screenshot, Server Disconnected";
    return false;
}
void BehvLogger::Process() {
    if (OptionalParameter("/LogBehv") == 0) return;
    if (!isParsed || OptionalParameter("/ConnectServerBehv") == 0 || mMatrixCodes.empty())
        return;
    currEvent = State(ServerEventName); // get the current Event code
}

void BehvLogger::SendServerString(int _thread)
{
    auto lastTime = std::chrono::steady_clock::now();
    //int iV = 1;
    if ((!isParsed || OptionalParameter("/ConnectServerBehv") == 0 || mMatrixCodes.empty() ) && !doScreenshot)
        return;
    
    while (!m_kill_threadServer)
    {
        if (initalSleep) {
            Sleep(300);
            initalSleep = false;
        }
        else if (!sRunning) {
            Sleep(30);
        } else
        {
            /*
            // alternating non blocking loop
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - lastTime;

            if (elapsed.count() >= 2.0)
            {
                currEvent = static_cast<double>(iV);
                iV = (iV + 1) % 3;
                lastTime = now;
            }
            
            */
            if (!currEvent) {
            
            }
            else {
                for (size_t i = 0; i < mMatrixCodes.size(); ++i)
                {
                    double incode = mMatrixCodes[i]; // incode is a possible code in the matrix

                    if (currEvent == incode && hasSent[i] == 0) // we need to send since code=event and hasnt sent yet
                    {

                        const std::string& inExpression = mMatrixExpressions[i];
                        const std::string& inTitle = mMatrixTitles[i];



                        // Send to UDP server
                        /*
                        int sendResult = sendto(udpSocketServer,
                            inExpression.c_str(),
                            static_cast<int>(inExpression.length()),
                            0,
                            (sockaddr*)&_udpReceiverServer,
                            sizeof(_udpReceiverServer));
                        */
                        
                        
                        size_t expressionLength = inExpression.length();
                        static const int BuffLen = CNEL_BEHV_PROTOCOL_MAX_MSG * 8;
                        char sendBuff[CNEL_BEHV_PROTOCOL_MAX_MSG];
                        std::memcpy(sendBuff, inExpression.c_str(), expressionLength);

                        /*
                        bciwarn << "BehvServer TryingSend: " << inTitle
                            << " | Event length: " << BuffLen
                            << " | Expression: " << sendBuff;
                        */

                        int sendResult = sendto(udpSocketServer,
                            sendBuff,
                            expressionLength,
                            0,
                            (sockaddr*)&_udpReceiverServer,
                            sizeof(_udpReceiverServer));


                        //LocalFree(sendBuff);

                        if (sendResult == SOCKET_ERROR)
                        {
                            bcierr << "Failed to send BehvServer message ERR: " << WSAGetLastError();
                        }
                        else
                        {
                            bciout << "BehvServer: " << inTitle
                                << " | Event Code: " << incode
                                << " | Sent Expression: " << inExpression;
                            hasSent[i] = 1;
                        }
                    }
                    else if (currEvent != incode)
                    {
                        hasSent[i] = 0; // reset when event changes
                    }
                }
            }


            // screenshot on a timer
            if (doScreenshot) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - lastTime;

                if (elapsed.count() >= screenshotS)
                {
               

                    const std::string& inExpression = "Screenshot: FileName="+ ScreenshotDirectory;


                    size_t expressionLength = inExpression.length();
                    static const int BuffLen = CNEL_BEHV_PROTOCOL_MAX_MSG * 8;
                    char sendBuff[CNEL_BEHV_PROTOCOL_MAX_MSG];
                    std::memcpy(sendBuff, inExpression.c_str(), expressionLength);

                    /*
                    bciwarn << "BehvServer TryingSend: " << inTitle
                        << " | Event length: " << BuffLen
                        << " | Expression: " << sendBuff;
                    */

                    int sendResult = sendto(udpSocketServer,
                        sendBuff,
                        expressionLength,
                        0,
                        (sockaddr*)&_udpReceiverServer,
                        sizeof(_udpReceiverServer));

                    lastTime = now;
                }


            }

            Sleep(20); // Prevent CPU overusage
        }

    }
}


void
BehvLogger::StartRun()
{
    if (OptionalParameter("/LogBehv") == 0) return;
    // The user has just pressed "Start" (or "Resume").
    // Initialize mPreviousPos to a value that you know will never occur:
    mPreviousPos = -1;
    // It is now time to register our callback.
    // Typically, the hardware API is a C API and knows nothing about classes,
    // but allows a user-defined pointer variable as a second argument to the callback.
    // We specify the static callback function, and a pointer to the current class instance.
    //ExampleAPIRegisterCallback(mBehvLoggerHandle, &BehvLoggerCallback, this);
    mRunning = true;
    sRunning = true;
}

void
BehvLogger::StopRun()
{
    //if (OptionalParameter("/LogBehv") == 0) return;
    // The Running state has been set to 0, either because the user has pressed "Suspend",
    // or because the run has reached its natural end.
    // It is now time to unregister the callback because we cannot record events any more.
    //ExampleAPIUnregisterCallback(mBehvLoggerHandle, &BehvLoggerCallback, this);
    mRunning = false;
    sRunning = false;
}

void
BehvLogger::Halt()
{

    if (IsConnectionActive())
    {
        int iResult = closesocket(udpSocket);
        if (iResult == SOCKET_ERROR)
        {
            bcierr << "Could not close UDP socket " << WSAGetLastError() << endl;
        }
        udpSocket = INVALID_SOCKET;
    }
    /*
    if (udpSocketServer != INVALID_SOCKET)
    {
        int iResult = closesocket(udpSocket);
        if (iResult == SOCKET_ERROR)
        {
            bcierr << "Could not close UDP Server socket " << WSAGetLastError() << endl;
        }
        udpSocketServer = INVALID_SOCKET;
    }
    */
    // Stop any threads or other asynchronous activity.
    // If you registered any callback, it is likely to be called from a different thread, too,
    // so call StopRun() to be sure it's unregistered.
    StopRun();
}

void
BehvLogger::BehvLoggerCallback(int position, void* userData)
{
    // This is the callback function. It does nothing else than forward the call
    // to our handler function, which is a member function and cannot be specified as
    // a callback function.
    //static_cast<BehvLogger*>(userData)->OnBehvLoggerPosition(position);
}
