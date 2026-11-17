////////////////////////////////////////////////////////////////////////////////
// Authors: Luciano Branco - CNELAB - Fri May 26 14:06:46 2023
// Description: BehvLogger connects and reads behavioral data from the 
// application originally described in
// https://ieeexplore.ieee.org/document/8037190
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_BEHVLOGGER_H  // makes sure this header is not included more than once
#define INCLUDED_BEHVLOGGER_H

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>//needs to be above windows.h to avoid redefinition from inside windows.h ... (https://stackoverflow.com/questions/22517036/socket-errors-cant-get-functions-in-winsock2-h)
#include <ws2tcpip.h> 
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#include "Environment.h"
//#include "SerialPort.h"
#include "Thread.h"
#include <atomic>
#include <valarray>
#include <vector>
#include <string>

#define CNEL_BEHV_PROTOCOL_MAX_MSG (4+64)     // 4 default + 64 max teensy channels
// Include any library headers specific to your hardware here:
//#include "BehvLogger.h"

class BehvLogger : public EnvironmentExtension, public Thread
{
public:
	BehvLogger();
	~BehvLogger();
	void Publish() override;
	void Preflight() const override;
	void Initialize() override;
	void StartRun() override;
	void StopRun() override;
	void Halt() override;
	void Process() override;

private:
	// SerialPort* mDevice;
	// Use this space to declare any BehvLogger-specific methods and member variables you'll need.
	// Note that you need to deal with multithreading, so make sure that variables used across
	// threads are of type std::atomic, or use a mutex.
	// FOR RECEIVER
	bool mLogBehvLogger;
	bool mRunning;
	std::atomic<int> mPreviousPos;
	std::thread m_thread;
	std::atomic_bool m_kill_thread;
	const int SenderAddrSize;
	double recvData[CNEL_BEHV_PROTOCOL_MAX_MSG];
	int bytesRecvd;

	// FOR SERVERSENDER
	bool sRunning;
	std::thread m_threadServer;
	std::atomic_bool m_kill_threadServer;
	std::string ServerEventName;
	double currEvent = 0;
	int numRows;
	bool initalSleep;
	bool isParsed;

	typedef std::valarray<double> Vector;       
	typedef std::vector<Vector> Matrix;         // Matrix of numerical vectors
	std::vector<double> hasSent;
	std::vector<double> mMatrixCodes;           // Vector of doubles
	std::vector<std::string> mMatrixExpressions; // Vector of strings
	std::vector<std::string> mMatrixTitles;      // Another vector of strings

	// For Screenshot Timer in server
	bool doScreenshot;
	double screenshotS; // how often to screenshot in milliseconds
	std::string ScreenshotDirectory; // default to run directory and use with filename
	std::string RecordingFilename;


	// If you can register a callback function to asynchronously receive input or hardware data,
	// declare it here as static.
	static void BehvLoggerCallback(int position, void* userData);
	void AcquireData(int _thread);

	void CnelBehvConnect(const std::string _UDPstring);
	void CnelBehvDisconnect();

	bool IsConnectionActive();
	bool init_udp(int);
	int getNextUDPPackage(double*);

	SOCKET udpSocket;
	struct sockaddr_in _udpReceiver;

	// FOR SERVERSENDER
	void SendServerString(int _thread);

	void CnelBehvServerConnect(const std::string _UDPstring);
	void CnelBehvServerDisconnect();
	bool init_udp_server(int);
	bool IsServerConnectionActive();
	bool parseServerMatrix();

	SOCKET udpSocketServer;
	struct sockaddr_in _udpReceiverServer;
};

#endif // INCLUDED_BEHVLOGGER_H
