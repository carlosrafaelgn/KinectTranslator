//
// KinectTranslator is distributed under the FreeBSD License
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
// https://github.com/carlosrafaelgn/KinectTranslator
//
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WindowsX.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <NuiApi.h>
#include <Winsock2.h>

#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cstdio>

//extra libraries
#pragma comment(lib, "Kinect10.lib")
#pragma comment(lib, "Ws2_32.lib")

//intrinsic functions
#include <intrin.h>

#pragma intrinsic (_mm_cvtt_ss2si)
#pragma intrinsic (_mm_cvt_ss2si)

//THESE FUNCTIONS REQUIRE AT LEAST SSE SUPPORT
//there could be a runtime check for SSE support at startup......... but I don't think it's 100% necessary...
//http://msdn.microsoft.com/en-us/library/hskdteyh.aspx

//math_ftol and math_ftolr could fail when compiled in
//debug mode, if it were not for the #ifdef...
//because when in debug mode, the code
//_mm_cvtt_ss2si( *((__m128*)&x) )
//
//is NOT translated by the VS compiler as:
//cvttss2si eax, dword ptr [ebp-18h]
//
//instead, it is translated as:
//movaps xmm0, xmmword ptr [ebp-18h]  
//cvttss2si eax, xmm0
//
//which breaks everything when the variable is not
//aligned at a 16-byte boundary
__inline int math_ftol(float x) //truncates
{
#ifdef NDEBUG
	return _mm_cvtt_ss2si( *((__m128*)&x) );
#else
	return _mm_cvtt_ss2si(_mm_load1_ps(&x));
#endif
}

__inline int math_ftolr(float x) //rounds
{
#ifdef NDEBUG
	return _mm_cvt_ss2si( *((__m128*)&x) );
#else
	return _mm_cvt_ss2si(_mm_load1_ps(&x));
#endif
}
