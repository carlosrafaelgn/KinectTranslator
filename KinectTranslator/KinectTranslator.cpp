//
// KinectTranslator.cpp is distributed under the FreeBSD License
//
// Copyright (c) 2012, Carlos Rafael Gimenes das Neves
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer. 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those
// of the authors and should not be interpreted as representing official policies, 
// either expressed or implied, of the FreeBSD Project.
//
// https://github.com/carlosrafaelgn/KinectTranslator/blob/master/KinectTranslator/KinectTranslator.cpp
//
#include "stdafx.h"
#include "KinectTranslator.h"
#include "Resource.h"

static const TCHAR* const KINECTTRANSLATORWND = TEXT("KINECTTRANSLATORWND");
static const TCHAR* const KINECTTRANSLATORANGLEWND = TEXT("KINECTTRANSLATORANGLEWND");

static const TCHAR* const SensorBodyPartNames[BodyPartCount] = {
	TEXT("CenterHip"),
	TEXT("Spine"),
	TEXT("CenterShoulder"),
	TEXT("Head"),
	TEXT("LeftShoulder"),
	TEXT("LeftElbow"),
	TEXT("LeftWrist"),
	TEXT("LeftHand"),
	TEXT("RightShoulder"),
	TEXT("RightElbow"),
	TEXT("RightWrist"),
	TEXT("RightHand"),
	TEXT("LeftHip"),
	TEXT("LeftKnee"),
	TEXT("LeftAnkle"),
	TEXT("LeftFoot"),
	TEXT("RightHip"),
	TEXT("RightKnee"),
	TEXT("RightAnkle"),
	TEXT("RightFoot")
};

static const TCHAR* const DefaultMsgs[MsgCount] = {
	TEXT("Center Hip"),
	TEXT("Spine"),
	TEXT("Center Shoulder"),
	TEXT("Head"),
	TEXT("Left Shoulder"),
	TEXT("Left Elbow"),
	TEXT("Left Wrist"),
	TEXT("Left Hand"),
	TEXT("Right Shoulder"),
	TEXT("Right Elbow"),
	TEXT("Right Wrist"),
	TEXT("Right Hand"),
	TEXT("Left Hip"),
	TEXT("Left Knee"),
	TEXT("Left Ankle"),
	TEXT("Left Foot"),
	TEXT("Right Hip"),
	TEXT("Right Knee"),
	TEXT("Right Ankle"),
	TEXT("Right Foot"),
	TEXT("Present"),
	TEXT("Kinect Translator"),
	TEXT("&Start"),
	TEXT("S&top"),
	TEXT("&Edit configuration file..."),
	TEXT("&About..."),
	TEXT("E&xit"),
	TEXT("About..."),
	TEXT("After editing the configuration file the program will be automatically restarted. Do you want to continue?"),
	TEXT("It will not be possible to restart the program automatically. Please, restart it manually after editing the file."),
	TEXT("Oops..."),
	TEXT("An error has occurred while creating the main window. [Code: %d]"),
	TEXT("An error has occurred while initializing the network structures. [Code: %d]"),
	TEXT("An error has occurred while creating the communication event structures. [Code: %d]"),
	TEXT("An error has occurred while creating the network thread. [Code: %d]"),
	TEXT("The connection with Scratch has been lost! [Code: %d]"),
	TEXT("An error has occurred while obtaining the sensor count. [Code: %08X]"),
	TEXT("No sensors have been found!"),
	TEXT("The sensor requested in the configuration file was unavailable. Would you like to proceed with the first available sensor?"),
	TEXT("Apparently there is at least one sensor connected to the system, but none is available! Please, make sure the sensors are properly powered and that they are not being used by any other application."),
	TEXT("The sensor could not be properly initialized. Sometimes it takes a bit longer than the usual to initialize. If this is the first time you see this message, please, try again in a few moments. [Code: %08X]"),
	TEXT("An error has occurred while setting the sensor\'s tracking mode. [Code: %08X]"),
	TEXT("An error has occurred while creating the network socket. [Code: %d]"),
	TEXT("An error has occurred while connecting to Scratch. [Code: %d]"),
	TEXT("Configure sensor's elevation angle..."),
	TEXT("Configure sensor's elevation angle"),
	TEXT("Please, wait before changing the angle again..."),
	TEXT("Click and drag to adjust the current angle:"),
	TEXT("An error has occurred while creating the main internal structures. [Code: %d]"),
	TEXT("Kinect Translator is already running! Please, terminate that instance before starting a new one.")
};
const unsigned char Axes[AxisCount] = { 'X', 'Y', 'Z' };

#define PacketHeaderLen 17
static const char PacketHeader[] = "\0\0\0\0sensor-update";
static const char* const BooleanMsgs[] = { "0", "1" };
static const int BooleanMsgsLen[] = { 1, 1 };

HINSTANCE gInst;
HWND gWnd, gWndAngle;
int gPlayerData[MaxPlayerCount][BodyPartCount][AxisCount], gNetworkPlayerData[MaxPlayerCount][BodyPartCount][AxisCount];
char *gPlayerBodyParts[MaxPlayerCount][BodyPartCount][AxisCount], *gPlayerPresentMsg[MaxPlayerCount], *gPlayerPresentBroadcast[MaxPlayerCount], gPlayerPresent[MaxPlayerCount], gNetworkPlayerPresent[MaxPlayerCount];
int gPlayerBodyPartsLen[MaxPlayerCount][BodyPartCount][AxisCount], gPlayerPresentMsgLen[MaxPlayerCount], gPlayerPresentBroadcastLen[MaxPlayerCount], gPlayersSkeleton[MaxPlayerCount], gSkeletonsPlayer[NUI_SKELETON_COUNT];
BOOL gSendableValues[BodyPartCount][AxisCount];
TCHAR *gMsgs[MsgCount], *gLanguageFileExt;
int gMinValues[AxisCount], gMaxValues[AxisCount];
float gDelta[AxisCount];
int gKinectIndex, gPlayerCount, gFirstPlayerIndex, gMaximumPacketLen;
__int64 gLastFrameTime;
unsigned short gPort;
HMENU gMnu, gPopMnu;
HICON gMainIconSmall, gMainIconLarge, gMainIconSmallRunning, gMainIconSmallStopped;
BOOL gInMenu, gNetworkAlive, gNetworkDataReady, gUseUDP;
INuiSensor *gSensor;
HANDLE gMutex, gStream, gEventNextAvailable, gNetworkThread, gNetworkDataAvailable;
SOCKET gNetworkSocket;

//---------------------------------------
//angle configuration stuff
BOOL gWndAngleWaiting, gWndAngleChanging;
HPEN gWndAnglePen;
HFONT gWndAngleFont;
LONG gWndAngleValue, gWndAngleValueLast;
RECT gWndAngleClientRect;
//---------------------------------------

BOOL InitInstance(HINSTANCE hInstance, BOOL firstTime);
void TerminateInstance(BOOL full);

int ShowMessage(const TCHAR* msg, const TCHAR* title, BOOL showYesNo, BOOL useErrorIcon)
{
	return MessageBox(HWND_DESKTOP, msg, title, MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST | (useErrorIcon ? MB_ICONERROR : (showYesNo ? MB_ICONQUESTION : MB_ICONINFORMATION)) | (showYesNo ? MB_YESNO : 0));
}

BOOL ShowError(int msgId, int errorCode)
{
	TCHAR* tmp = (TCHAR*)malloc(sizeof(TCHAR) * 2048);
	wsprintf(tmp, gMsgs[msgId], errorCode);
	ShowMessage(tmp, gMsgs[MsgTitleError], false, true);
	free(tmp);
	return false;
}

BOOL ShowSimpleError(int msgId)
{
	ShowMessage(gMsgs[msgId], gMsgs[MsgTitleError], false, true);
	return false;
}

TCHAR *GetCfgFilename(TCHAR *filename, BOOL languageFile)
{
	TCHAR *ptr;
	GetModuleFileName(gInst, filename, 1024);
	ptr = _tcsrchr(filename, '.');
	if (!ptr)
		ptr = filename + wcslen(filename);
	ptr[0] = '.';
	if (languageFile)
	{
		if (gLanguageFileExt)
			_tcscpy_s(ptr + 1, 16, gLanguageFileExt);
		else
			ptr[1] = 0;
	}
	else
	{
		ptr[1] = 't';
		ptr[2] = 'x';
		ptr[3] = 't';
		ptr[4] = 0;
	}
	return filename;
}

int Int2DecChar(int value, char* dstBuf)
{
	int l;
	unsigned int v;
	if ((value & 0x80000000))
	{
		*dstBuf = '-';
		v = ((unsigned int)-value);
		l = 1;
	}
	else
	{
		l = 0;
		v = value;
	}
	if (v < 10)
		l += 1;
	else if (v < 100)
		l += 2;
	else if (v < 1000)
		l += 3;
	else if (v < 10000)
		l += 4;
	else if (v < 100000)
		l += 5;
	else if (v < 1000000)
		l += 6;
	else if (v < 10000000)
		l += 7;
	else if (v < 100000000)
		l += 8;
	else if (v < 1000000000)
		l += 9;
	else
		l += 10;
	dstBuf += l;
	do
	{
		*(--dstBuf) = '0' + (char)(v % 10);
		v /= 10;
	} while (v);
	return l;
}

BOOL InitSocket(BOOL showErrorMsgs)
{
	sockaddr_in addr;
	
	gNetworkSocket = (gUseUDP ? socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) : socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	if (!gNetworkSocket || gNetworkSocket == INVALID_SOCKET)
	{
		gNetworkSocket = 0;
		if (showErrorMsgs)
			ShowError(MsgErrorCreatingSocket, WSAGetLastError());
		return false;
	}

	//try to connect to Scratch (use connect even for UDP connections just to save the default address)
	addr.sin_family = AF_INET;
	addr.sin_port = htons(gPort);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(gNetworkSocket, (SOCKADDR*)&addr, sizeof(sockaddr_in)))
	{
		if (showErrorMsgs)
			ShowError(MsgErrorConnecting, WSAGetLastError());
		return false;
	}

	return true;
}

BOOL InitSensor()
{
	int i, count = 0;
	HRESULT hr = NuiGetSensorCount(&count);
	if (FAILED(hr))
		return ShowError(MsgErrorGettingSensorCount, hr);
	if (!count)
		return ShowError(MsgErrorZeroSensorCount, 0);

	if (gKinectIndex >= 0)
	{
		hr = NuiCreateSensorByIndex(gKinectIndex, &gSensor);
		if (FAILED(hr))
		{
			if (ShowMessage(gMsgs[MsgErrorSelectedSensorUnavailable], gMsgs[MsgTitleError], true, false) == IDNO)
				return false;
		}
		if (FAILED(gSensor->NuiStatus()))
		{
			gSensor->Release();
			gSensor = 0;
			if (ShowMessage(gMsgs[MsgErrorSelectedSensorUnavailable], gMsgs[MsgTitleError], true, false) == IDNO)
				return false;
		}
	}

	if (!gSensor)
	{
		for (i = 0; i < count; ++i)
		{
			hr = NuiCreateSensorByIndex(i, &gSensor);
			if (FAILED(hr))
				continue;
			if (SUCCEEDED(gSensor->NuiStatus()))
				break;
			gSensor->Release();
			gSensor = 0;
		}
		if (!gSensor)
			return ShowError(MsgErrorNoSensorsAvailable, 0);
	}

	hr = gSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
	if (SUCCEEDED(hr))
	{
		ResetEvent(gEventNextAvailable);
		hr = gSensor->NuiSkeletonTrackingEnable(gEventNextAvailable, NUI_SKELETON_TRACKING_FLAG_SUPPRESS_NO_FRAME_DATA); //NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT
		if (FAILED(hr))
			return ShowError(MsgErrorEnablingSensorTracking, hr);
	}
	else
	{
		return ShowError(MsgErrorInitializingSensor, hr);
	}

	return true;
}

void TerminateSocket()
{
	if (gNetworkSocket)
	{
		//DisconnectEx(gNetworkSocket, 0, 0, 0);
		shutdown(gNetworkSocket, SD_BOTH);
		closesocket(gNetworkSocket);
		gNetworkSocket = 0;
	}
}

void TerminateSensor()
{
	if (gSensor)
	{
		gSensor->NuiShutdown();
		gSensor->Release();
		gSensor = 0;
	}
}

void Stop()
{
	NOTIFYICONDATA nid;

	gNetworkAlive = false;
	gNetworkDataReady = false;

	//wake up the network thread
	if (gNetworkDataAvailable)
		SetEvent(gNetworkDataAvailable);

	if (gNetworkThread)
	{
		//give some time for the network thread to die...
		WaitForSingleObject(gNetworkThread, 20000);
		CloseHandle(gNetworkThread);
		gNetworkThread = 0;
	}

	TerminateSocket();

	TerminateSensor();

	if (gEventNextAvailable)
		ResetEvent(gEventNextAvailable);

	if (gWnd && gMainIconSmallStopped)
	{
		memset(&nid, 0, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.hWnd = gWnd;
		nid.uID = 100;
		nid.uFlags = NIF_ICON;
		nid.hIcon = gMainIconSmallStopped;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}

LRESULT CALLBACK ProcNetworkThread(void* v)
{
	int p, i, l, a, packetLen;
	BOOL broadcastNeeded[MaxPlayerCount] = {false};
	int lastData[MaxPlayerCount][BodyPartCount][AxisCount];
	int data[MaxPlayerCount][BodyPartCount][AxisCount];
	char lastPresent[MaxPlayerCount];
	char present[MaxPlayerCount];
	char* packet = (char*)malloc(gMaximumPacketLen), *tmp;

	//force the first packet to be sent
	for (i = (MaxPlayerCount * BodyPartCount * AxisCount) - 1; i >= 0; i--)
		((int*)lastData)[i] = MININT;
	memset(data, 0, sizeof(int) * MaxPlayerCount * BodyPartCount * AxisCount);
	memset(lastPresent, -1, MaxPlayerCount);
	memset(present, 0, MaxPlayerCount);

	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
	SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

	while (gNetworkAlive)
	{
		if (!gNetworkDataReady)
			WaitForSingleObject(gNetworkDataAvailable, INFINITE);
		if (!gNetworkAlive)
			break;

		memcpy(present, gNetworkPlayerPresent, gPlayerCount);
		memcpy(data, gNetworkPlayerData, sizeof(int) * gPlayerCount * BodyPartCount * AxisCount);
		gNetworkDataReady = false;

		//build the packet
		packetLen = 0;
		tmp = packet + PacketHeaderLen;
		for (p = gPlayerCount - 1; p >= 0; p--)
		{
			//send the "Present" value for the current player
			if (present[p] != lastPresent[p])
			{
				broadcastNeeded[p] = true;
				//name
				l = gPlayerPresentMsgLen[p];
				memcpy(tmp, gPlayerPresentMsg[p], l);
				tmp += l;
				packetLen += l;
				//boolean value (0 or 1)
				*tmp = (present[p] ? '1' : '0');
				tmp++;
				packetLen++;
				lastPresent[p] = present[p];
			}
			//send the other values for the current player
			for (i = BodyPartCount - 1; i >= 0; i--)
			{
				for (a = AxisCount - 1; a >= 0; a--)
				{
					if (gSendableValues[i][a])
					{
						if (data[p][i][a] != lastData[p][i][a])
						{
							//name
							l = gPlayerBodyPartsLen[p][i][a];
							memcpy(tmp, gPlayerBodyParts[p][i][a], l);
							tmp += l;
							packetLen += l;
							//integer value
							l = Int2DecChar(data[p][i][a], tmp);
							tmp += l;
							packetLen += l;
							lastData[p][i][a] = data[p][i][a];
						}
					}
				}
			}
		}
		if (packetLen)
		{
			memcpy(packet + 4, PacketHeader + 4, PacketHeaderLen - 4);
			packetLen += (PacketHeaderLen - 4);
			packet[0] = (char)((packetLen >> 24) & 0xFF);
			packet[1] = (char)((packetLen >> 16) & 0xFF);
			packet[2] = (char)((packetLen >> 8 ) & 0xFF);
			packet[3] = (char)((packetLen      ) & 0xFF);
			if (send(gNetworkSocket, packet, packetLen + 4, 0) == SOCKET_ERROR)
			{
				//reset the socket before warning the user about the error (this usually happens when
				//the user changes the current project in Scratch)
				TerminateSocket();
				if (!InitSocket(false) || send(gNetworkSocket, packet, packetLen + 4, 0) == SOCKET_ERROR)
				{
					PostMessage(gWnd, WM_CONNECTIONLOST, WSAGetLastError(), 0);
					break;
				}
			}
		}
		//send all pending broadcasts AFTER sending the sensor values, so that when
		//the broadcast arrives, the sensor will be reporting the correct value
		for (p = gPlayerCount - 1; p >= 0; p--)
		{
			if (broadcastNeeded[p])
			{
				broadcastNeeded[p] = false;
				if (send(gNetworkSocket, gPlayerPresentBroadcast[p], gPlayerPresentBroadcastLen[p], 0) == SOCKET_ERROR)
				{
					//reset the socket before warning the user about the error(this usually happens when
					//the user changes the current project in Scratch)
					TerminateSocket();
					if (!InitSocket(false) || send(gNetworkSocket, packet, packetLen + 4, 0) == SOCKET_ERROR)
					{
						PostMessage(gWnd, WM_CONNECTIONLOST, WSAGetLastError(), 0);
						break;
					}
				}
			}
		}
	}

	SetThreadExecutionState(ES_CONTINUOUS);

	free(packet);

	return 0;
}

void Start()
{
	unsigned long threadId;
	NOTIFYICONDATA nid;

	Stop();

	ResetEvent(gEventNextAvailable);
	ResetEvent(gNetworkDataAvailable);

	if (!InitSocket(true) || !InitSensor())
	{
		Stop();
		return;
	}

	gNetworkAlive = true;
	gNetworkThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ProcNetworkThread, 0, 0, &threadId);
	if (!gNetworkThread || gNetworkThread == INVALID_HANDLE_VALUE)
	{
		gNetworkAlive = false;
		gNetworkThread = 0;
		ShowError(MsgErrorCreatingThread, GetLastError());
		Stop();
		return;
	}

	gLastFrameTime = 0;

	if (gWnd && gMainIconSmallRunning)
	{
		memset(&nid, 0, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.hWnd = gWnd;
		nid.uID = 100;
		nid.uFlags = NIF_ICON;
		nid.hIcon = gMainIconSmallRunning;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}

void ProcessData()
{
	int i, p;
	__int64 frameTime = 0;
	NUI_SKELETON_DATA* data;
	Vector4* pos;

	if (!gSensor)
		return;

	//confirm that it is time to process data
    if (WaitForSingleObject(gEventNextAvailable, 0) != WAIT_OBJECT_0)
        return;

	NUI_SKELETON_FRAME frame;
	if (FAILED(gSensor->NuiSkeletonGetNextFrame(0, &frame)))
		return;
	gSensor->NuiTransformSmooth(&frame, 0);

	//try to associate each player to one available skeleton
	//1. check for any skeletons that have just become invisible
	for (i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		data = &(frame.SkeletonData[i]);
		if (gSkeletonsPlayer[i] >= 0 && data->eTrackingState != NUI_SKELETON_TRACKED)
		{
			//this skeleton no longer belongs to any player
			gPlayerPresent[gSkeletonsPlayer[i]] = 0;
			//keep gPlayersSkeleton[gSkeletonsPlayer[i]] untouched for reference when returning
			gSkeletonsPlayer[i] = -1;
		}
	}
	//2. try to give back to the players their original skeletons
	for (p = 0; p < gPlayerCount; p++)
	{
		if (!gPlayerPresent[p])
		{
			//try to get back the last skeleton used, if there was a previous skeleton and if it is still available
			i = gPlayersSkeleton[p];
			if (i >= 0 && gSkeletonsPlayer[i] < 0 && frame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
			{
				//made it! the skeleton now belongs to the same player as it used to!
				gPlayerPresent[p] = 1;
				gSkeletonsPlayer[i] = p;
			}
		}
	}
	//3. try to find an available skeleton for each player still without a skeleton
	for (p = 0; p < gPlayerCount; p++)
	{
		if (!gPlayerPresent[p])
		{
			//try to get the first available skeleton for this player...
			for (i = 0; i < NUI_SKELETON_COUNT; i++)
			{
				if (gSkeletonsPlayer[i] < 0 && frame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
				{
					//made it! this player now has an associated skeleton!
					gPlayersSkeleton[p] = i;
					gPlayerPresent[p] = 1;
					gSkeletonsPlayer[i] = p;
				}
			}
		}
	}
	//4. process each player's skeleton data
	for (p = gPlayerCount - 1; p >= 0; p--)
	{
		if (!gPlayerPresent[p])
		{
			//when one player's skeleton is not visible, just reset all values to their minimum values
			for (i = BodyPartCount - 1; i >= 0; i--)
				memcpy(gPlayerData[p][i], gMinValues, sizeof(int) * AxisCount);
		}
		else
		{
			frameTime = frame.liTimeStamp.QuadPart;

			data = &(frame.SkeletonData[gPlayersSkeleton[p]]);
			for (i = BodyPartCount - 1; i >= 0; i--)
			{
				//convert the coordinates into our range
				//according to the docs, values coming from SkeletonPositions are in the range below:
				//x: from approximately -2.2 to +2.2
				//y: from approximately -1.6 to +1.6
				//z: from 0.0 to 4.0
				//all values are in meters
				pos = &(data->SkeletonPositions[i]);
				if (pos->z > FLT_EPSILON)
				{
					//pos->x * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / pos->z gives a value in the range -160 ... 160
					if (gSendableValues[i][0])
						gPlayerData[p][i][0] = math_ftol((((pos->x * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / pos->z) + 160) * gDelta[0]) / 320) + gMinValues[0];

					//pos->y * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / pos->z gives a value in the range -120 ... 120
					if (gSendableValues[i][1])
						gPlayerData[p][i][1] = math_ftol((((pos->y * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / pos->z) + 120) * gDelta[1]) / 240) + gMinValues[1];

					//pos->z * 0.25f gives a value in the range 0 ... 1
					if (gSendableValues[i][2])
						gPlayerData[p][i][2] = math_ftol(pos->z * 0.25f * gDelta[2]) + gMinValues[2];
				}
				else
				{
					//there is nothing to do...
					memcpy(gPlayerData[p][i], gMinValues, sizeof(int) * AxisCount);
				}
			}
		}
	}

	//the frames could come at an amaizing rate, so don't hog system resources
	if ((frameTime - gLastFrameTime) > 5)
		Sleep(5);
	gLastFrameTime = frameTime;

	//don't override existing data
	if (!gNetworkDataReady)
	{
		memcpy(gNetworkPlayerPresent, gPlayerPresent, gPlayerCount);
		memcpy(gNetworkPlayerData, gPlayerData, sizeof(int) * gPlayerCount * BodyPartCount * AxisCount);
		gNetworkDataReady = true;
		SetEvent(gNetworkDataAvailable);
	}
}

void DoMenu()
{
	POINT pt;
	TCHAR name[1024];
	SHELLEXECUTEINFO sei;
	if (gInMenu)
		return;
	gInMenu = true;
	SetFocus(gWnd);
	SetForegroundWindow(gWnd);
	GetCursorPos(&pt);
	//enable/disable the menus accordingly...
	EnableMenuItem(gPopMnu, IDM_START, MF_BYCOMMAND | (gNetworkAlive ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(gPopMnu, IDM_STOP, MF_BYCOMMAND | (gNetworkAlive ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(gPopMnu, IDM_ANGLE, MF_BYCOMMAND | (gSensor ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(gPopMnu, IDM_EDITCFGFILE, MF_BYCOMMAND | (gNetworkAlive ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(gPopMnu, IDM_ABOUT, MF_BYCOMMAND | (gNetworkAlive ? MF_GRAYED : MF_ENABLED));
	switch (TrackPopupMenu(gPopMnu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, 0, gWnd, 0))
	{
	case IDM_START:
		Start();
		break;
	case IDM_STOP:
		Stop();
		break;
	case IDM_ANGLE:
		if (!gWndAngle)
		{
			gWndAngle = CreateWindowEx(0, KINECTTRANSLATORANGLEWND, gMsgs[MsgTitleConfigureAngle], WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_DESKTOP, 0, gInst, 0);
			ShowWindow(gWndAngle, SW_SHOW);
			SetForegroundWindow(gWndAngle);
		}
		else
		{
			ShowWindow(gWndAngle, SW_SHOW);
			SetForegroundWindow(gWndAngle);
		}
		break;
	case IDM_EDITCFGFILE:
		if (ShowMessage(gMsgs[MsgConfirmReset], gMsgs[MsgTitleError], true, false) == IDNO)
			break;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.hwnd = gWnd;
		sei.lpVerb = TEXT("open");
		sei.lpFile = GetCfgFilename(name, false);
		sei.nShow = SW_SHOWMAXIMIZED;
		ShellExecuteEx(&sei);
		//ShellExecute(gWnd, TEXT("open"), GetCfgFilename(name), 0, 0, SW_SHOW);
		if (sei.hProcess && sei.hProcess != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(sei.hProcess, INFINITE);
			CloseHandle(sei.hProcess);
			TerminateInstance(false);
			InitInstance(gInst, false);
		}
		else
		{
			ShowError(MsgResetError, 0);
		}
		break;
	case IDM_ABOUT:
		ShowMessage(TEXT("Kinect Translator 1.0.1\n\nCopyright © 2012 BandTec\n\nCreated by Carlos Rafael Gimenes das Neves\nBugs? Ideas? Contact me: carlos.rafael@bandtec.com.br\n\n\nFor information on usage, license and updates, visit:\nhttp://www.bandtec.com.br\nhttp://carlosrafaelgn.com.br/Scratch\n\n\nThis software intends to be a lighter version of the original one:\nKinect2Scratch by Stephen Howell\nhttp://scratch.saorog.com"), gMsgs[MsgTitleAbout], false, false);
		break;
	case IDM_EXIT:
		DestroyWindow(gWnd);
		break;
	}
	gInMenu = false;
}

LRESULT ProcWndAngle_Paint()
{
	//position the points within a virtual 2 x 2 rectangle
	POINTFLOAT sensor[7] = { { 0.0f, 0.0f }, { 0.5f, 0.5f }, { 0.5f, -0.5f }, { -0.55f, -0.75f }, { -0.45f, -0.75f }, { -0.75f, -0.75f }, { 0.75f, -0.75f } };
	PAINTSTRUCT ps;
	float x, y, scale, theta = (float)gWndAngleValue * 0.01745329251994329576923690768489f; //degrees to radians (+gWndAngleValue -> CCW)
	float costheta = cos(theta);
	float sintheta = sin(theta);
	int i, x0, y0;
	char txt[16];

	//this drawing code causes some flickering, nevertheless, I don't think that justifies the need for double-buffer technics...
	HDC hdc = BeginPaint(gWndAngle, &ps);
	SelectPen(hdc, gWndAnglePen);
	SelectFont(hdc, gWndAngleFont);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, OPAQUE);
	SetTextColor(hdc, RGB(255, 255, 255));
	SetTextAlign(hdc, TA_TOP | TA_LEFT | TA_UPDATECP);

	if (gWndAngleClientRect.right > 0 && gWndAngleClientRect.bottom > 0)
	{
		MoveToEx(hdc, 10, 10, 0);
		if (gWndAngleWaiting)
		{
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &gWndAngleClientRect, gMsgs[MsgWaitBeforeNextAngle], (int)_tcslen(gMsgs[MsgWaitBeforeNextAngle]), 0);
		}
		else
		{
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &gWndAngleClientRect, gMsgs[MsgAngle], (int)_tcslen(gMsgs[MsgAngle]), 0);
			TextOut(hdc, 0, 0, TEXT(" "), 1);
			TextOutA(hdc, 0, 0, txt, Int2DecChar((int)gWndAngleValue, txt)); // :)
		}

		//rotate and translate the sensor points
		for (i = 0; i < 3; i++)
		{
			x = sensor[i].x;
			y = sensor[i].y;
			sensor[i].x = (x * costheta) - (y * sintheta) - 0.5f;
			sensor[i].y = (x * sintheta) + (y * costheta);
		}

		scale = (float)((gWndAngleClientRect.right > gWndAngleClientRect.bottom) ? gWndAngleClientRect.bottom : gWndAngleClientRect.right) * 0.5f;

		//scale the points
		for (i = 0; i < 7; i++)
		{
			sensor[i].x *= scale;
			sensor[i].y *= scale;
		}

		//draw the sensor
		x0 = gWndAngleClientRect.right >> 1;
		y0 = gWndAngleClientRect.bottom >> 1;
		MoveToEx(hdc, math_ftol(sensor[0].x) + x0, y0 - math_ftol(sensor[0].y), 0);
		LineTo(hdc, math_ftol(sensor[1].x) + x0, y0 - math_ftol(sensor[1].y));
		LineTo(hdc, math_ftol(sensor[2].x) + x0, y0 - math_ftol(sensor[2].y));
		LineTo(hdc, math_ftol(sensor[0].x) + x0, y0 - math_ftol(sensor[0].y));
		LineTo(hdc, math_ftol(sensor[3].x) + x0, y0 - math_ftol(sensor[3].y));
		MoveToEx(hdc, math_ftol(sensor[4].x) + x0, y0 - math_ftol(sensor[4].y), 0);
		LineTo(hdc, math_ftol(sensor[0].x) + x0, y0 - math_ftol(sensor[0].y));
		MoveToEx(hdc, math_ftol(sensor[5].x) + x0, y0 - math_ftol(sensor[5].y), 0);
		LineTo(hdc, math_ftol(sensor[6].x) + x0, y0 - math_ftol(sensor[6].y));
	}

	EndPaint(gWndAngle, &ps);
	return 0;
}

LRESULT ProcWndAngle_MouseMove(int x, int y)
{
	float x0, y0, xc, yc, scale;
	int angle;
	if (gWndAngleChanging && gWndAngleClientRect.right > 0 && gWndAngleClientRect.bottom > 0)
	{
		scale = (float)((gWndAngleClientRect.right > gWndAngleClientRect.bottom) ? gWndAngleClientRect.bottom : gWndAngleClientRect.right) * 0.5f;

		x0 = -0.5f * scale;
		y0 = 0.0f;
		xc = (float)(x - (gWndAngleClientRect.right >> 1));
		yc = (float)((gWndAngleClientRect.bottom >> 1) - y);

		angle = math_ftol(atan2(yc - y0, xc - x0) * 57.295779513082320876798154814105f); //radians to degrees

		if (angle > NUI_CAMERA_ELEVATION_MAXIMUM) angle = NUI_CAMERA_ELEVATION_MAXIMUM;
		else if (angle < NUI_CAMERA_ELEVATION_MINIMUM) angle = NUI_CAMERA_ELEVATION_MINIMUM;

		if (angle != gWndAngleValue)
		{
			gWndAngleValue = angle;
			InvalidateRect(gWndAngle, &gWndAngleClientRect, false);
		}
	}
	return 0;
}

LRESULT ProcWndAngle_LButtonDown(int x, int y)
{
	if (!gWndAngleWaiting)
	{
		gWndAngleChanging = true;
		ReleaseCapture();
		SetCapture(gWndAngle);
		ProcWndAngle_MouseMove(x, y);
	}
	return 0;
}

LRESULT ProcWndAngle_LButtonUp(int x, int y)
{
	if (gWndAngleChanging)
	{
		gWndAngleChanging = false;
		ReleaseCapture();
		if (gWndAngleValueLast != gWndAngleValue)
		{
			gWndAngleValueLast = gWndAngleValue;
			if (gSensor)
			{
				if (SUCCEEDED(gSensor->NuiCameraElevationSetAngle(gWndAngleValue)))
				{
					gWndAngleWaiting = true;
					SetTimer(gWndAngle, 0, 5000, 0);
					InvalidateRect(gWndAngle, &gWndAngleClientRect, false);
				}
			}
		}
	}
	return 0;
}

LRESULT ProcWndAngle_PosChanged(WINDOWPOS* pos)
{
	GetClientRect(gWndAngle, &gWndAngleClientRect);
	return 0;
}

LRESULT ProcWndAngle_Timer()
{
	gWndAngleWaiting = false;
	KillTimer(gWndAngle, 0);
	InvalidateRect(gWndAngle, &gWndAngleClientRect, false);
	return 0;
}

void ProcWndAngle_NCCreate(CREATESTRUCT* info)
{
	gWndAngleWaiting = false;
	gWndAngleChanging = false;
	gWndAnglePen = GetStockPen(WHITE_PEN);
	gWndAngleFont = GetStockFont(DEFAULT_GUI_FONT);
	GetClientRect(gWndAngle, &gWndAngleClientRect);
	if (!gSensor || FAILED(gSensor->NuiCameraElevationGetAngle(&gWndAngleValue)))
		gWndAngleValue = NUI_CAMERA_ELEVATION_MINIMUM;
	gWndAngleValueLast = gWndAngleValue;
}

LRESULT CALLBACK ProcWndAngle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		return ProcWndAngle_Paint();
	case WM_MOUSEMOVE:
		return ProcWndAngle_MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	case WM_LBUTTONDOWN:
		return ProcWndAngle_LButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	case WM_LBUTTONUP:
		return ProcWndAngle_LButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	case WM_WINDOWPOSCHANGED:
		return ProcWndAngle_PosChanged((WINDOWPOS*)lParam);
	case WM_TIMER:
		return ProcWndAngle_Timer();
	case WM_DESTROY:
		gWndAngle = 0;
		break;
	case WM_NCCREATE:
		ProcWndAngle_NCCreate((CREATESTRUCT*)lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK ProcWnd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TRAYNOTIFICATION:
		switch (LOWORD(lParam))
		{
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			DoMenu();
			break;
		}
		break;
	case WM_CONNECTIONLOST:
		Stop();
		ShowError(MsgErrorConnectionLost, (int)wParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		if (gNetworkAlive && (LOWORD(wParam) == SC_MONITORPOWER || LOWORD(wParam) == SC_SCREENSAVE))
			break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

TCHAR __stdcall GetCharInverted(FILE* file)
{
	const TCHAR c = (((unsigned short)fgetc(file)) & 0xFF) << 8;
	const int r = fgetc(file);
	return ((r == EOF) ? 0 : (c | (r & 0xFF)));
}

TCHAR __stdcall GetCharNormal(FILE* file)
{
	const TCHAR c = ((unsigned short)fgetc(file)) & 0xFF;
	const int r = fgetc(file);
	return ((r == EOF) ? 0 : (c | ((r & 0xFF) << 8)));
}

BOOL ReadValue(FILE* file, TCHAR (__stdcall* getChar)(FILE*), TCHAR* name, TCHAR* value)
{
	TCHAR c;
	int len = 0;
	BOOL comment;

	*name = 0;
	*value = 0;

	do
	{
		comment = false;
		//fill in the name skipping any white spaces on both left and right sides
		do
		{
		} while ((c = getChar(file)) && iswspace(c));
		if (c)
		{
			if (c == ';') //comment
			{
				comment = true;
				do
				{
				} while ((c = getChar(file)) && (c != 10) && (c != 13));
			}
			else
			{
				do
				{
					if (iswspace(c) || c == '=')
						break;
					name[len++] = c;
				} while ((c = getChar(file)) && len < 1023);
			}
		}

		if (!c) return false; //useless to continue
	} while (comment);

	name[len] = 0;

	//try to get to the = symbol
	while (c && c != '=')
		c = getChar(file);

	if (!c) return false; //useless to continue

	//do the same for the value, except that the trailing spaces are removed later (in order to preserve internal white spaces)
	len = 0;
	do
	{
	} while ((c = getChar(file)) && iswspace(c) && (c != 10) && (c != 13)); //cr, lf, crlf... who still cares?! :)
	if (c)
	{
		do
		{
			if (c == 10 || c == 13)
				break;
			value[len++] = c;
		} while ((c = getChar(file)) && len < 1023);
	}

	value[len] = 0;

	//remove trailing white spaces
	while (len > 0 && iswspace(value[len - 1]))
		value[--len] = 0;

	return true;
}

BOOL Text2Int(const TCHAR* text, int* value, int defaultValue)
{
	TCHAR *end;
	int r = _tcstol(text, &end, 10);
	if (!end || end == text)
	{
		*value = defaultValue;
		return false;
	}
	*value = r;
	return true;
}

void InitConfig()
{
	int i, p, l, a;
	FILE* file;
	TCHAR (__stdcall* getChar)(FILE*);
	TCHAR c, *name = (TCHAR*)malloc(1024 * sizeof(TCHAR)), *value = (TCHAR*)malloc(1024 * sizeof(TCHAR));

	//load the config file
	GetCfgFilename(name, false);

	file = 0;
	_tfopen_s(&file, name, TEXT("rb"));
	//if the file has not been found, or could not be opened, just go with the default values
	if (file)
	{
		getChar = ((GetCharNormal(file) == 0xFFFE) ? GetCharInverted : GetCharNormal);
		while (ReadValue(file, getChar, name, value))
		{
			if ((*name) && (*value))
			{
				//now it is safe to process the pair
				if (!_tcsicmp(name, TEXT("KinectIndex")))
				{
					Text2Int(value, &gKinectIndex, -1);
					if (gKinectIndex < -1)
						gKinectIndex = -1;
				}
				else if (!_tcsicmp(name, TEXT("PlayerCount")))
				{
					Text2Int(value, &gPlayerCount, 1);
					if (gPlayerCount > MaxPlayerCount)
						gPlayerCount = MaxPlayerCount;
					else if (gPlayerCount < 1)
						gPlayerCount = 1;
				}
				else if (!_tcsicmp(name, TEXT("Port")))
				{
					Text2Int(value, &i, 42001);
					gPort = (unsigned short)i;
				}
				else if (!_tcsicmp(name, TEXT("UseUDP")))
				{
					Text2Int(value, &i, 0);
					gUseUDP = (i != 0);
				}
				else if (!_tcsicmp(name, TEXT("Language")))
				{
					if (!gLanguageFileExt && value[0])
						gLanguageFileExt = _tcsdup(value);
				}
				else if (!_tcsicmp(name, TEXT("FirstPlayerIndex")))
				{
					Text2Int(value, &gFirstPlayerIndex, 1);
				}
				else if (name[0] == 'm' || name[0] == 'M')
				{
					//must be the configuration of a limit
					if (Text2Int(value, &i, -1))
					{
						if (!_tcsicmp(name, TEXT("MinX"))) gMinValues[0] = i;
						else if (!_tcsicmp(name, TEXT("MinY"))) gMinValues[1] = i;
						else if (!_tcsicmp(name, TEXT("MinZ"))) gMinValues[2] = i;
						else if (!_tcsicmp(name, TEXT("MaxX"))) gMaxValues[0] = i;
						else if (!_tcsicmp(name, TEXT("MaxY"))) gMaxValues[1] = i;
						else if (!_tcsicmp(name, TEXT("MaxZ"))) gMaxValues[2] = i;
					}
				}
				else
				{
					//must be an axis configuration
					for (i = BodyPartCount - 1; i >= 0; i--)
					{
						if (!_tcsicmp(SensorBodyPartNames[i], name))
						{
							l = 0;
							while ((c = value[l]) && l++ < 3)
							{
								if (c == 'x' || c == 'X') gSendableValues[i][0] = 1;
								else if (c == 'y' || c == 'Y') gSendableValues[i][1] = 1;
								else if (c == 'z' || c == 'Z') gSendableValues[i][2] = 1;
							}
							break;
						}
					}
				}
			}
		}
		fclose(file);
	}

	//load the translated strings
	GetCfgFilename(name, true);

	file = 0;
	_tfopen_s(&file, name, TEXT("rb"));
	//if the file has not been found, or could not be opened, just go with the default values
	if (file)
	{
		getChar = ((GetCharNormal(file) == 0xFFFE) ? GetCharInverted : GetCharNormal);
		while (ReadValue(file, getChar, name, value))
		{
			if ((*name) && (*value))
			{
				//must be a numeric value specifying a translated string
				if (Text2Int(name, &i, -1) && i >= 0 && i < MsgCount && (*value))
					gMsgs[i] = _tcsdup(value);
			}
		}
		fclose(file);
	}

	for (i = 0; i < AxisCount; i++)
	{
		if (gMaxValues[i] == gMinValues[i])
			gMaxValues[i] = gMinValues[i] + 1;
	}

	//fill in empty messages with the default ones
	for (i = 0; i < MsgCount; i++)
	{
		if (!gMsgs[i])
			gMsgs[i] = _tcsdup(DefaultMsgs[i]);
	}

	gMaximumPacketLen = PacketHeaderLen;

	//create the names for the sendable body parts of all players
	for (i = 0; i < BodyPartCount; i++)
	{
		for (a = 0; a < 3; a++)
		{
			if (gSendableValues[i][a])
			{
				for (p = 0; p < gPlayerCount; p++)
				{
					//transform the name into something like "A NameX1" (in UTF-8)
					//these strings are not null terminated, as they will only be sent through the
					//network and never displayed on the screen... moreover, their length is already
					//stored in a separate place
					l = WideCharToMultiByte(CP_UTF8, 0, gMsgs[i], -1, 0, 0, 0, 0);
					gPlayerBodyParts[p][i][a] = (char*)malloc(l + 16);
					gPlayerBodyParts[p][i][a][0] = ' ';
					gPlayerBodyParts[p][i][a][1] = '\"';
					l = WideCharToMultiByte(CP_UTF8, 0, gMsgs[i], -1, gPlayerBodyParts[p][i][a] + 2, l + 1, 0, 0) - 1 + 2;

					gPlayerBodyParts[p][i][a][l] = Axes[a];
					l += 1 + Int2DecChar(gFirstPlayerIndex + p, gPlayerBodyParts[p][i][a] + l + 1);
					gPlayerBodyParts[p][i][a][l] = '\"';
					gPlayerBodyParts[p][i][a][l + 1] = ' ';
					gPlayerBodyPartsLen[p][i][a] = l + 2;

					//reserve 12 extra bytes per value in the final packet sent to Scratch
					gMaximumPacketLen += l + 2 + 12;
				}
			}
		}
	}

	for (p = 0; p < gPlayerCount; p++)
	{
		//just as above...
		l = WideCharToMultiByte(CP_UTF8, 0, gMsgs[MsgPlayerPresent], -1, 0, 0, 0, 0);
		gPlayerPresentMsg[p] = (char*)malloc(l + 16);
		gPlayerPresentMsg[p][0] = ' ';
		gPlayerPresentMsg[p][1] = '\"';
		l = WideCharToMultiByte(CP_UTF8, 0, gMsgs[MsgPlayerPresent], -1, (char*)gPlayerPresentMsg[p] + 2, l + 1, 0, 0) - 1 + 2;

		l += Int2DecChar(gFirstPlayerIndex + p, gPlayerPresentMsg[p] + l);
		gPlayerPresentMsg[p][l] = '\"';
		gPlayerPresentMsg[p][l + 1] = ' ';
		gPlayerPresentMsgLen[p] = l + 2;

		gMaximumPacketLen += l + 2 + 12;

		//create a broadcast package like this: (4 bytes)broadcast "Present1"
		l = 4 + 9 + l + 1;
		gPlayerPresentBroadcastLen[p] = l;
		gPlayerPresentBroadcast[p] = (char*)malloc(l);
		l -= 4;
		gPlayerPresentBroadcast[p][0] = (char)((l >> 24) & 0xFF);
		gPlayerPresentBroadcast[p][1] = (char)((l >> 16) & 0xFF);
		gPlayerPresentBroadcast[p][2] = (char)((l >> 8 ) & 0xFF);
		gPlayerPresentBroadcast[p][3] = (char)((l      ) & 0xFF);
		memcpy(gPlayerPresentBroadcast[p] + 4, "broadcast", 9);
		memcpy(gPlayerPresentBroadcast[p] + 4 + 9, gPlayerPresentMsg[p], l - 9);
	}

	free(name);
	free(value);
}

BOOL InitInstance(HINSTANCE hInstance, BOOL firstTime)
{
	WSADATA wd;
	WNDCLASSEX wcex;
	MENUITEMINFO mnuii;
	NOTIFYICONDATA nid;
	HDC hdc;
	int sx, sy, dpiX, dpiY;

	if (firstTime)
	{
		gInst = hInstance;
		gWnd = 0;
		gWndAngle = 0;
		gMnu = 0;
		gPopMnu = 0;
		gInMenu = false;
		gEventNextAvailable = 0;
		gNetworkDataAvailable = 0;
		gMainIconSmall = 0;
		gMainIconLarge = 0;
		gMainIconSmallRunning = 0;
		gMainIconSmallStopped = 0;
		gMutex = 0;
	}
	memset(gPlayerData, 0, sizeof(int) * MaxPlayerCount * BodyPartCount * AxisCount);
	memset(gNetworkPlayerData, 0, sizeof(int) * MaxPlayerCount * BodyPartCount * AxisCount);
	memset(gPlayerBodyParts, 0, sizeof(char*) * MaxPlayerCount * BodyPartCount * AxisCount);
	memset(gPlayerBodyPartsLen, 0, sizeof(int) * MaxPlayerCount * BodyPartCount * AxisCount);
	memset(gPlayerPresentMsg, 0, sizeof(char*) * MaxPlayerCount);
	memset(gPlayerPresentMsgLen, 0, sizeof(int) * MaxPlayerCount);
	memset(gPlayerPresentBroadcast, 0, sizeof(char*) * MaxPlayerCount);
	memset(gPlayerPresentBroadcastLen, 0, sizeof(int) * MaxPlayerCount);
	memset(gSendableValues, 0, sizeof(BOOL) * BodyPartCount * AxisCount);
	memset(gPlayerPresent, 0, MaxPlayerCount);
	memset(gNetworkPlayerPresent, 0, MaxPlayerCount);
	memset(gMsgs, 0, sizeof(TCHAR*) * MsgCount);
	gLanguageFileExt = 0;
	for (sx = 0; sx < MaxPlayerCount; sx++)
		gPlayersSkeleton[sx] = -1;
	for (sx = 0; sx < NUI_SKELETON_COUNT; sx++)
		gSkeletonsPlayer[sx] = -1;
	gKinectIndex = -1;
	gPlayerCount = 1;
	gFirstPlayerIndex = 1;
	gMaximumPacketLen = 0;
	gPort = 42001;
	gMinValues[0] = -240;
	gMaxValues[0] = 240;
	gMinValues[1] = -160;
	gMaxValues[1] = 160;
	gMinValues[2] = 0;
	gMaxValues[2] = 400;
	gNetworkAlive = false;
	gNetworkDataReady = false;
	gUseUDP = false;
	gSensor = 0;
	gStream = 0;
	gNetworkSocket = 0;
	gNetworkThread = 0;

	InitConfig();

	gDelta[0] = (float)(gMaxValues[0] - gMinValues[0]);
	gDelta[1] = (float)(gMaxValues[1] - gMinValues[1]);
	gDelta[2] = (float)(gMaxValues[2] - gMinValues[2]);

	if (firstTime)
	{
		//try to get the mutex access ONLY after initializing the configuration,
		//where all translated messages are loaded
		gMutex = CreateMutex(0, true, TEXT("Global\\KinectTranslatorAccess556EC5F158D44576B1594B1DAE2AE48E"));
		if (!gMutex || gMutex == INVALID_HANDLE_VALUE)
		{
			gMutex = 0;
			return ShowError(MsgErrorInternalStructures, GetLastError());
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return ShowSimpleError(MsgErrorAlreadyRunning);

		//load and create the tray icon
		hdc = CreateDC(TEXT("DISPLAY"), 0, 0, 0);
		dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		DeleteDC(hdc);
		sx = MulDiv(16, dpiX, 96);
		sy = MulDiv(16, dpiY, 96);
		//find the largest dimension among all possible dimensions
		dpiX = GetSystemMetrics(SM_CXSMICON);
		dpiY = GetSystemMetrics(SM_CYSMICON);
		if (dpiX > sx) sx = dpiX;
		if (dpiY > sy) sy = dpiY;
		if (sy > sx) sx = sy;
		//try to get an icon size as close as possible to one of the sizes present in the main icon file (256, 48, 32, 24, 16)
		if (sx > 48) sx = 256;
		else if (sx > 32) sx = 48;
		else if (sx > 24) sx = 32;
		else if (sx > 16) sx = 24;
		else sx = 16;

		gMainIconSmall = (HICON)LoadImage(gInst, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, sx, sx, 0);
		gMainIconLarge = (HICON)LoadImage(gInst, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
		//these icons don't have a higher resolution version
		if (sx > 48) sx = 48;
		gMainIconSmallRunning = (HICON)LoadImage(gInst, MAKEINTRESOURCE(IDI_MAINRUN), IMAGE_ICON, sx, sx, 0);
		gMainIconSmallStopped = (HICON)LoadImage(gInst, MAKEINTRESOURCE(IDI_MAINSTOP), IMAGE_ICON, sx, sx, 0);

		memset(&wcex, 0, sizeof(WNDCLASSEX));
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ProcWnd;
		wcex.hInstance = hInstance;
		wcex.hIcon = gMainIconLarge;
		wcex.hIconSm = gMainIconSmall;
		wcex.lpszClassName = KINECTTRANSLATORWND;
		RegisterClassEx(&wcex);

		memset(&wcex, 0, sizeof(WNDCLASSEX));
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ProcWndAngle;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(0, IDC_CROSS);
		wcex.hIcon = gMainIconLarge;
		wcex.hIconSm = gMainIconSmall;
		wcex.lpszClassName = KINECTTRANSLATORANGLEWND;
		RegisterClassEx(&wcex);

		gWnd = CreateWindowEx(0, KINECTTRANSLATORWND, gMsgs[MsgTitle], WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_DESKTOP, 0, gInst, 0);
		if (!gWnd)
			return ShowError(MsgErrorCreatingWindow, GetLastError());

		//try to initialize network structures
		if (WSAStartup(MAKEWORD(2, 2), &wd))
			return ShowError(MsgErrorInitializingNetwork, WSAGetLastError());

		//try to create the events that will be used for communication
		gEventNextAvailable = CreateEvent(0, true, false, 0);
		if (!gEventNextAvailable || gEventNextAvailable == INVALID_HANDLE_VALUE)
		{
			gEventNextAvailable = 0;
			return ShowError(MsgErrorCreatingEvent, GetLastError());
		}

		gNetworkDataAvailable = CreateEvent(0, false, false, 0);
		if (!gNetworkDataAvailable || gNetworkDataAvailable == INVALID_HANDLE_VALUE)
		{
			gNetworkDataAvailable = 0;
			return ShowError(MsgErrorCreatingEvent, GetLastError());
		}

		gMnu = LoadMenu(gInst, MAKEINTRESOURCE(IDR_MNU));
		gPopMnu = GetSubMenu(gMnu, 0);
	}

	//translate the menus
	memset(&mnuii, 0, sizeof(MENUITEMINFO));
	mnuii.cbSize = sizeof(MENUITEMINFO);
	mnuii.fMask = MIIM_STRING | MIIM_FTYPE;
	mnuii.fType = MFT_STRING;
	mnuii.dwTypeData = gMsgs[MsgStart];
	SetMenuItemInfo(gPopMnu, IDM_START, false, &mnuii);
	mnuii.dwTypeData = gMsgs[MsgStop];
	SetMenuItemInfo(gPopMnu, IDM_STOP, false, &mnuii);
	mnuii.dwTypeData = gMsgs[MsgConfigureAngle];
	SetMenuItemInfo(gPopMnu, IDM_ANGLE, false, &mnuii);
	mnuii.dwTypeData = gMsgs[MsgEditCfgFile];
	SetMenuItemInfo(gPopMnu, IDM_EDITCFGFILE, false, &mnuii);
	mnuii.dwTypeData = gMsgs[MsgAbout];
	SetMenuItemInfo(gPopMnu, IDM_ABOUT, false, &mnuii);
	mnuii.dwTypeData = gMsgs[MsgExit];
	SetMenuItemInfo(gPopMnu, IDM_EXIT, false, &mnuii);

	//disable the stop item
	mnuii.fMask = MIIM_STATE;
	mnuii.fState = MFS_GRAYED;
	SetMenuItemInfo(gPopMnu, IDM_STOP, false, &mnuii);

	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = gWnd;
	nid.uID = 100;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYNOTIFICATION;
	nid.hIcon = gMainIconSmallStopped;
	_tcscpy_s(nid.szTip, gMsgs[MsgTitle]);
	Shell_NotifyIcon(NIM_ADD, &nid);

	//try to start already, saving some time for the user
	Start();

	return true;
}

void TerminateInstance(BOOL full)
{
	//clean up everything
	int i, p, a;
	NOTIFYICONDATA nid;

	Stop();

	if (full)
	{
		if (gEventNextAvailable)
		{
			CloseHandle(gEventNextAvailable);
			gEventNextAvailable = 0;
		}

		if (gNetworkDataAvailable)
		{
			CloseHandle(gNetworkDataAvailable);
			gNetworkDataAvailable = 0;
		}

		WSACleanup();

		if (gMnu)
		{
			DestroyMenu(gMnu);
			gMnu = 0;
			gPopMnu = 0;
		}
	}

	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = gWnd;
	nid.uID = 100;
	Shell_NotifyIcon(NIM_DELETE, &nid);

	if (full)
	{
		if (gWndAngle)
		{
			DestroyWindow(gWndAngle);
			gWndAngle = 0;
		}

		UnregisterClass(KINECTTRANSLATORWND, gInst);
		UnregisterClass(KINECTTRANSLATORANGLEWND, gInst);

		if (gMainIconSmall)
		{
			DestroyIcon(gMainIconSmall);
			gMainIconSmall = 0;
		}

		if (gMainIconLarge)
		{
			DestroyIcon(gMainIconLarge);
			gMainIconLarge = 0;
		}

		if (gMainIconSmallRunning)
		{
			DestroyIcon(gMainIconSmallRunning);
			gMainIconSmallRunning = 0;
		}

		if (gMainIconSmallStopped)
		{
			DestroyIcon(gMainIconSmallStopped);
			gMainIconSmallStopped = 0;
		}

		if (gMutex)
		{
			CloseHandle(gMutex);
			gMutex = 0;
		}
	}

	for (i = 0; i < MsgCount; i++)
	{
		if (gMsgs[i])
		{
			free(gMsgs[i]);
			gMsgs[i] = 0;
		}
	}

	if (gLanguageFileExt)
	{
		free(gLanguageFileExt);
		gLanguageFileExt = 0;
	}

	for (p = 0; p < MaxPlayerCount; p++)
	{
		if (gPlayerPresentMsg[p])
		{
			free(gPlayerPresentMsg[p]);
			gPlayerPresentMsg[p] = 0;
		}
		if (gPlayerPresentBroadcast[p])
		{
			free(gPlayerPresentBroadcast[p]);
			gPlayerPresentBroadcast[p] = 0;
		}
		for (i = 0; i < BodyPartCount; i++)
		{
			for (a = 0; a < AxisCount; a++)
			{
				if (gPlayerBodyParts[p][i][a])
				{
					free(gPlayerBodyParts[p][i][a]);
					gPlayerBodyParts[p][i][a] = 0;
				}
			}
		}
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	if (!InitInstance(hInstance, true))
	{
		TerminateInstance(true);
		return false;
	}

	do
	{
		if (MsgWaitForMultipleObjects(1, &gEventNextAvailable, false, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0)
			ProcessData();

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	} while (msg.message != WM_QUIT);

	TerminateInstance(true);

	return (int)msg.wParam;
}
