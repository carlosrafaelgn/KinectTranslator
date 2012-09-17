//
// KinectTranslator.h is distributed under the FreeBSD License
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
// https://github.com/carlosrafaelgn/KinectTranslator/blob/master/KinectTranslator/KinectTranslator.h
//
#pragma once

#define WM_TRAYNOTIFICATION (WM_USER + 0x0100)
#define WM_CONNECTIONLOST (WM_USER + 0x0101)

#define HipCenter 0
#define Spine 1
#define ShoulderCenter 2
#define Head 3
#define ShoulderLeft 4
#define ElbowLeft 5
#define WristLeft 6
#define HandLeft 7
#define ShoulderRight 8
#define ElbowRight 9
#define WristRight 10
#define HandRight 11
#define HipLeft 12
#define KneeLeft 13
#define AnkleLeft 14
#define FootLeft 15
#define HipRight 16
#define KneeRight 17
#define AnkleRight 18
#define FootRight 19

#define MaxPlayerCount 4
#define BodyPartCount 20
#define AxisCount 3

#define MsgPlayerPresent BodyPartCount
#define MsgTitle (MsgPlayerPresent + 1)
#define MsgStart (MsgTitle + 1) 
#define MsgStop (MsgStart + 1) 
#define MsgEditCfgFile (MsgStop + 1) 
#define MsgAbout (MsgEditCfgFile + 1) 
#define MsgExit (MsgAbout + 1)
#define MsgTitleAbout (MsgExit + 1)
#define MsgConfirmReset (MsgTitleAbout + 1)
#define MsgResetError (MsgConfirmReset + 1)
#define MsgTitleError (MsgResetError + 1)
#define MsgErrorCreatingWindow (MsgTitleError + 1)
#define MsgErrorInitializingNetwork (MsgErrorCreatingWindow + 1)
#define MsgErrorCreatingEvent (MsgErrorInitializingNetwork + 1)
#define MsgErrorCreatingThread (MsgErrorCreatingEvent + 1)
#define MsgErrorConnectionLost (MsgErrorCreatingThread + 1)
#define MsgErrorGettingSensorCount (MsgErrorConnectionLost + 1)
#define MsgErrorZeroSensorCount (MsgErrorGettingSensorCount + 1)
#define MsgErrorSelectedSensorUnavailable (MsgErrorZeroSensorCount + 1)
#define MsgErrorNoSensorsAvailable (MsgErrorSelectedSensorUnavailable + 1)
#define MsgErrorInitializingSensor (MsgErrorNoSensorsAvailable + 1)
#define MsgErrorEnablingSensorTracking (MsgErrorInitializingSensor + 1)
#define MsgErrorCreatingSocket (MsgErrorEnablingSensorTracking + 1)
#define MsgErrorConnecting (MsgErrorCreatingSocket + 1)
#define MsgConfigureAngle (MsgErrorConnecting + 1)
#define MsgTitleConfigureAngle (MsgConfigureAngle + 1)
#define MsgWaitBeforeNextAngle (MsgTitleConfigureAngle + 1)
#define MsgAngle (MsgWaitBeforeNextAngle + 1)
#define MsgErrorInternalStructures (MsgAngle + 1)
#define MsgErrorAlreadyRunning (MsgErrorInternalStructures + 1)

#define MsgCount (MsgErrorAlreadyRunning + 1)
