// HDTV998Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment(lib, "..\\Board973\\Board973.lib")

typedef enum
{
	STATUS_BASE_8VSB = 0x10001000,
	STATUS_BASE_DLL = 0x10002000,
	STATUS_BASE_MAIN = 0x10003000
}ENUM_STATUS_BASE;

typedef enum
{
	STATUS_FAILURE = -1,
	STATUS_SUCCESS,

	// Initialization process fail
	STATUS_INITIALIZE_ERROR_DLL = STATUS_BASE_DLL,
	// If Start >= End
	STATUS_PLAY_RANGE_INVALID,

	STATUS_BANK0_READY = STATUS_BASE_MAIN,
	STATUS_BANK1_READY, // Reserved
	STATUS_RECORD_COMPLETE, // Reserved
	ERROR_BANK_CONFLICT, // Reserved
	ERROR_FILE_NOT_OPEN, // Reserved
	ERROR_FILE_TOO_SMALL, // Reserved
	ERROR_FILE_READ_NO, // Reserved
	ERROR_FILE_WRITE_NO, // Reserved
	ERROR_INTERRUPT, // Reserved
	ERROR_MEMORY_ALLOCATION, // Reserved
	ERROR_FILE_PLAY_LIMIT, // Reserved

	// STATUS - Indicates a file has starting loading into system memory
	STATUS_PLAY_START = 0x1000300B,

	STATUS_OCX_STOP_VIDEO, // Reserved
	STATUS_PAUSE_PLAY, // Reserved
	ERROR_INVALID_ASPECT_RATIO, // Reserved
	STATUS_VIDEO_PAUSE, // Reserved
	ERROR_PTS_NOT_FOUND, // Reserved
	ERROR_PCR_NOT_FOUND, // Reserved

	// STATUS - Indicates a file had ended loading into system memory
	STATUS_PLAY_COMPLETE = 0x10003012,

	// STATUS - Indicates that 'Video Locked Interrupt' has occurred
	// (New Mpeg Data arrival)
	STATUS_PROGRAM_UPDATE = 0x10003013,

	STATUS_RESTART_QPSK, // Reserved
	STATUS_STREAM_UPDATE, // Reserved
	STATUS_NO_SIGNAL, // Reserved
	STATUS_STARTUP_VIDEO, // Reserved
	STATUS_MONITOR_RESTART, // Reserved
	STATUS_STREAM_START, // Reserved
	STATUS_STREAM_STARTED, // Reserved
	STATUS_STREAM_PSIP, // Reserved
	STATUS_STREAM_STOPPED, // Reserved
	STATUS_VIDEO_STARTED, // Reserved
	STATUS_VIDEO_STOPPED, // Reserved

	// same as STATUS_PROGRAM_UPDATE
	STATUS_VIDEO_LOCKED = 0x1000301F,

	// Non-seamless data monitoring indicating a video or audio change
	STATUS_NEW_PSIP = 0x10003020,

	STATUS_SYNC_READY = 0x10003021, // Reserved
	ERROR_VIDEO_START = 0x10003022, // Reserved
	ERROR_STREAM_CHANNEL = 0x10003023, // Reserved

	// Socket setup and ready to connect
	STATUS_SOCKET_LISTEN = 0x10003024,

	STATUS_SOCKET_WAIT = 0x10003025, // Reserved

	// A data underflow has happened pausing playback on the card,
	// but allowing the system buffers to fill again.
	STATUS_BUFFER_EMPTY_PAUSED = 0x10003026,

	// System buffers are full continuing playback
	STATUS_BUFFER_FULL_PLAYING = 0x10003027,

	// No programs found in "File Program Information Search"
	ERROR_NO_PROGRAMS = 0x10003028,

	// Starting playback error trying to open or add file to queue
	ERROR_SEAMLESS_MODULE = 0x10003029,

	// No PIDs found in "File Program Information Search"
	ERROR_NO_PIDS = 0x1000302A,

	// Not getting proper amount of data from "Seamless" module
	ERROR_TRANSFER_SIZE = 0x1000302B,

	// Couldn't find the program information
	ERROR_PROGRAM_SEARCH = 0x1000302C,

	// Invalid 19392658 bitrate
	ERROR_INVALID_ATSC_BITRATE = 0x1000302D,

	// After switching to the LVDS port Broadcom could not start the decode
	ERROR_NO_LVDS_DECODE = 0x1000302E,

	STATUS_END,
}ENUM_STATUS;

typedef enum
{
	ASPECT_AUTO,
	ASPECT_FULL,
	ASPECT_4X3,
	ASPECT_CROP,
	ASPECT_LETTERBOX,
	ASPECT_RESERVED_1
}ASPECT_ENUM;

typedef enum
{
	BOARD_TYPE_300_A,
	BOARD_TYPE_300_B,
	BOARD_TYPE_400_A
}ENUM_BOARD_TYPE;

typedef void (CALLBACK* PFN_ON_PROGRESS)(long lngDevice, long lPercent);
typedef void (CALLBACK* PFN_ON_STATUS)(long lngDevice, long lStatus);
typedef void (CALLBACK* PFN_ON_GENERIC)(long lngDevice, long lStatus);

extern "C"{
	// Documented APIs

	//Initialize973
	//
	//Function: Initializes the 973 hardware device.
	//
	//Usage: long Initialize973 ( long lngDevice );
	//
	//Parameters: lngDevice – The device to control.
	//
	//Note: The system can have multiple PRN devices; which the first device
	// is 0, the second is 1, and so on. To control each subsequent
	// function just place this device number in the lngDevice parameter
	// for each funtion. This function has to be called to initialize
	// each device.
	//
	//Return Value: STATUS_FAILURE – Already initialized
	// STATUS_SUCCESS
	// STATUS_INITIALIZE_ERROR_DLL – Hardware initialization error
	//
	// Example: Initialize973 ( 0 );
	//
	__declspec(dllimport) long WINAPI Initialize973(long lngDevice);

	//Uninitialize973
	//
	//Function: Uninitializes the 973 hardware device.
	//
	//Usage: long Uninitialize973 ( long lngDevice );
	//
	//Parameters: lngDevice – The device to control
	//
	//Return Value: STATUS_FAILURE – Hardware not initialized
	// STATUS_SUCCESS
	//
	//Example: Uninitialize973 ( 0 );
	//
	__declspec(dllimport) long WINAPI Uninitialize973(long lngDevice);

	//GetRevisions973
	//
	//Function: Retrieve the revision of the hardware device, the DLL, and the device driver.
	//
	//Syntax: long GetRevisions973 ( long lngDevice, long *plngRevDevice, double *pdblRevDLL, double *pdblRevDriver );
	//
	//Parameters: lngDevice – The device to control.
	// plngRevDevice – The pointer to the hardware device variable.
	// pdblRevDLL – The pointer to the DLL variable.
	// pdblRevDriver – The pointer to the driver variable.
	//
	//Return Value: STATUS_FAILURE – Hardware not initialized or DLL failure
	// STATUS_SUCCESS
	//
	//Example: GetRevisions973 ( 0, &lngRevDevice, &dblRevDLL, &dblRevDriver );
	//
	__declspec(dllimport) long WINAPI GetRevisions973(long lngDevice, long FAR* plngRevDevice, double FAR* pdblRevDLL, double FAR* pdblRevDriver);

	//ExecutePlay973
	//                                    
	//Function:	Start streaming a HDTV video or continue streaming video after a ‘pause’.
	//
	//Syntax:	long ExecutePlay973(long lngDevice , LPCSTR strFilename, long lngPercentStart, long lngPercentEnd, long lngProgram); 
	//
	//Parameters:	lngDevice – The device to control.
	//				strFilename – The HDTV video path and name with extension. 
	//				lngPercentStart – The starting point of the video file.
	//				lngPercentEnd – The ending point (percent) of the video file.
	//				lngProgram – The program index to select in the video.
	//
	//Note:	The program index should be set to 0 if the program index is not 
	//		known (see GetProgramCountFile973 to retrieve a video program 
	//		count before or after streaming occurs).  
	//
	//To stream the whole video file set lngPercentStart = 0 and lngPercentEnd = 100.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//		STATUS_SUCCESS
	//		STATUS_PLAY_RANGE_INVALID
	//
	//Example:	ExecutePlay973 (0, “C:\Clip.trp”, 0, 100, 0 );
	//
	__declspec(dllimport) long WINAPI ExecutePlay973(long lngDevice, LPCSTR strFilename, long lngPercentStart, long lngPercentEnd, long lngProgram);

	//ExecuteStop973
	//                                    
	//Function:	Stop streaming a HDTV video.
	//
	//Syntax:	long ExecuteStop973(long lngDevice); 
	//
	//Parameters:	lngDevice – The device to control.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	ExecuteStop973(0);
	//
	__declspec(dllimport) long WINAPI ExecuteStop973(long lngDevice);

	//ExecutePause973
	//                                    
	//Function:	‘Pause’ streaming a HDTV video in the current location or continue streaming video after a ‘pause’.
	//
	//Syntax:	long ExecutePause973(long lngDevice); 
	//
	//Parameters:	lngDevice – The device to control.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	ExecutePause973(0);
	//
	__declspec(dllimport) long WINAPI ExecutePause973(long lngDevice);

	//SetProgram973
	//                                    
	//Function:	Set the program index of the currently streaming video.
	//
	//Syntax:	long SetProgram973(long lngDevice , long lngProgramIndex); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngProgramIndex – The index of the available program list. 
	//
	//Notes:	The indexes are contiguous starting at a base index of 0 until 
	//			the end of the available program list.  The available program 
	//			list can be generated by the functions GetProgramCount973 or 
	//			GetProgramCountFile973.
	//
	//Example:  Programs available = 1,3,4,5; index 0 = program 1, index 1 = program 3, index 2 = program 4, index 3 = program 5.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	SetProgram973 (0, 0);
	//
	__declspec(dllimport) long WINAPI SetProgram973(long lngDevice, long lngProgramIndex);

	//GetProgramCount973
	//                                    
	//Function:	Retrieve the program count and the program numbers of the currently streaming video.
	//
	//Syntax:	long GetProgramCount973(long lngDevice,  long * plngProgramInfo); 
	//
	//Parameters:	lngDevice – The device to control.
	//				plngProgramInfo – A pointer to the variable to store the binary represetation of the files in the video.
	//
	//Notes:	The binary representation is as follows:  if bit N is a 1 then program N+1 is present.
	// 
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS - The number of programs present in the stream.
	//
	//Example:	GetProgramCount973( 0, &lngProgramInfo );
	//
	__declspec(dllimport) long WINAPI GetProgramCount973(long lngDevice, long FAR* plngProgramInfo);

	//GetProgramCountFile973
	//                                    
	//Function:	Retrieve the program count and the program numbers from a file.
	//
	//Syntax:	long GetProgramCountFile973 (long lngDevice,  long * plngProgramInfo); 
	//
	//Parameters:	lngDevice – The device to control.
	//				plngProgramInfo – A pointer to the variable to store the binary represetation of the files in the video (see GetProgramCount973 for description).  
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS - On success the function returns the number of programs present in the stream.
	//
	//Example:	GetProgramCountFile973 ( 0, &lngProgramInfo );
	//
	__declspec(dllimport) long WINAPI GetProgramCountFile973(long lngDevice, LPCSTR strFilename, long FAR* plngProgramInfo);

	//Set8VSBAdjustChannel973
	//                                    
	//Function:	Set the output channel for 8-VSB.
	//
	//Syntax:	long Set8VSBAdjustChannel973(long lngDevice,  long lngChannel); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngChannel – The output channel for 8-VSB.  Range: 14 - 21
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//				STATUS_CHANNEL_RANGE_ERROR
	//
	//Example:	Set8VSBAdjustChannel973 (0, 14);
	//
	__declspec(dllimport) long WINAPI Set8VSBAdjustChannel973(long lngDevice, long lngChannel);

	//SetNTSCAdjustChannel973
	//                                    
	//Function:	Set the output channel for NTSC.
	//
	//Syntax:	long SetNTSCAdjustChannel973 (long lngDevice, long lngChannel); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngChannel – The output channel for NTSC.  Range: VHF 3 – 13
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//				STATUS_CHANNEL_RANGE_ERROR
	//
	//Example:	SetNTSCAdjustChannel973 (0, 4);
	//
	__declspec(dllimport) long WINAPI SetNTSCAdjustChannel973(long lngDevice, long lngChannel);

	//SetNTSCClockExtension973 (Obsolete)
	//                                    
	//Function:	Lengthen the clock signal high or low (duty cycle).  
	//
	//Syntax:	long SetNTSCClockExtention973 (long lngDevice, long lngExtension); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngExtension – The number of times to lengthen the clock signal.  The default value is 20.
	//
	//Note:	This was just put in as a precautionary measure.  In the future 
	//      when computers are n-times as fast increasing this number will 
	//      help programming the NTSC chip.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	SetNTSCClockExtension973 (0, 50);
	//
	__declspec(dllimport) long WINAPI SetNTSCClockExtension973(long lngDevice, long lngExtension);

	//SetAspectRatio973
	//                                    
	//Function:	Set the display output aspect ratio.  
	//
	//Syntax:	long SetAspectRatio973(long lngDevice, long lngAspectRatio); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngAspectRatio – The aspect ratio (see enumeration ENUM_ASPECT)
	//
	//Note:	The valid modes:
	//
	//	Source Clip			Output Size			Valid Modes
	//		4x3					4x3				Full, Auto
	//		4x3					16x9			4x3
	//		16x9				4x3				Letterbox, Crop
	//		16x9				16x9			Full, Auto
	// 
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//				ERROR_INVALID_ASPECT_RATIO – A correct value was not selected from enum 
	//
	//Example:	SetAspectRatio973 (0, ASPECT_LETTERBOX);
	//
	__declspec(dllimport) long WINAPI SetAspectRatio973(long lngDevice, long lngAspectRatio);

	//GetRevisionXilinx973
	//                                    
	//Function:	Retrieve the revision of the Xilinx load (firmware).
	//
	//Syntax:	long GetRevisionsXilinx973(long lngDevice, double pdblRevXilinx , long plngBoardType); 
	//
	//Parameters:	lngDevice – The device to control.
	//				pdblRevXilinx – The pointer to the Xilinx variable.
	//				plngBoardType – The pointer to the Board Type variable. 
	//								Board type is as follows:	 0 = 300A, 1 = 300B,  2 = 400A
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	GetRevisionsXilinx973( 0, &dblRevXilinx, &lngBoardType );
	//
	__declspec(dllimport) long WINAPI GetRevisionXilinx973(long lngDevice, double FAR* pdblRevXilinx, long FAR* plngBoardType );

	//OnProgressSetup973
	//                                    
	//Function:	Setup the OnProgress callback function.
	//
	//Syntax:	long OnProgressSetup973 ( PFN_ON_PROGRESS OnProgress_); 
	//
	//Parameters:	OnProgress_ – The address of the function to callback to.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	OnProgressSetup973 (OnProgress973);
	//
	__declspec(dllimport) long WINAPI OnProgressSetup973(long lngDevice, PFN_ON_PROGRESS OnProgress_ );

	//OnStatusSetup973
	//                                    
	//Function:	Setup the OnStatus callback function.
	//
	//Syntax:	long OnStatusSetup973 (PFN_ON_PROGRESS OnStatus_); 
	//
	//Parameters:	OnStatus_ – The address of the function to callback to.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	OnStatusSetup973 (OnStatus973);
	//
	__declspec(dllimport) long WINAPI OnStatusSetup973(long lngDevice, PFN_ON_STATUS OnStatus_ );

	//ExecutePlaySeamless973
	//                                    
	//Function:	Start streaming a HDTV video or continue streaming video after a ‘pause’.
	//
	//Syntax:	long ExecutePlaySeamless973(long lngDevice , LPCTSTR strFilename, long lngNullPackets); 
	//
	//Parameters:	lngDevice – The device to control.
	//				strFilename – The HDTV video path and name with extension. 
	//				LngNullPackets – Reserved.
	//
	//Note:	1) Send this function to play a file in seamless mode.
	//	2) When the status message returns with STATUS_PLAY_START get the next file in the playlist.
	//	3) Go to step 1.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	ExecutePlaySeamless973 (0, “C:\Clip1.trp”, 0);
	//
	__declspec(dllimport) long WINAPI ExecutePlaySeamless973(long lngDevice, LPCSTR strFilename, long lngReserved);

	//GetCurrentlyPlayingFile973
	//                                    
	//Function:	Start Get the currently playing filename.
	//
	//Syntax:	long GetCurrentPlayingFile973 (long lngDevice , LPCTSTR strFilename); 
	//
	//Parameters:	lngDevice – The device to control.
	//				strFilename – The passed out string for the filename. 
	//
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized or DLL failure
	//				STATUS_SUCCESS
	//
	//Example:	GetCurrentPlayingFile973 (0, strFilenamePlaceholder );
	//
	__declspec(dllimport) long WINAPI GetCurrentPlayingFile973(long lngDevice , LPTSTR strFilename);

	//RestampStream973
	//                                    
	//Function:	Enable/Disable hardware restamping (calculate and overwrite PCR information in the stream).
	//
	//Syntax:	long RestampStream973(long lngDevice , long lMode); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lMode – 	0 – Disable firmware restamping.
	//							1 – Enable firmware restamping 
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or incorrect mode.
	//				STATUS_SUCCESS
	//
	//Example:	RestampStream973 (0, 1);
	//
	__declspec(dllimport) long WINAPI RestampStream973( long lngDevice, long lngMode );

	//SetPids973
	//                                    
	//Function:	Set the PIDs for the seamless playback and restamping.
	//
	//Syntax:	long SetStreamPids973(long lngDevice , long lPmtPid, long lVidPid, long lAudPid, long lPcrPid); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lPmtPid – The PMT PID.
	//				lVidPid – The video PID 
	//				lAudPid – The audio PID
	//				lPcrPid – The PCR PID
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or a PID > 0x1FFF.
	//				STATUS_SUCCESS
	//
	//Example:	SetPids973 (0, 0x0010, 0x0011, 0x0014, 0x0011);
	//
	__declspec(dllimport) long WINAPI SetPids973( long lngDevice,long lngPmtPid,long lngVidPid, long lngAudPid, long lngPcrPid );

	// TODO: Document
	__declspec(dllimport) long WINAPI EnableRestamping973( long lngDevice, long lngEnableRestamping );

	//Board973_SetNtscFrequency (See SetNTSCAdjustChannel973)
	//                                    
	//Function:	Set the NTSC output frequency.
	//
	//Syntax:	long Board973_SetNtscFrequency(long lngDevice , double dblFrequency); 
	//
	//Parameters:	lngDevice – The device to control.
	//				dblFrequency – The frequency of NTSC output.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or out of frequency range.
	//				STATUS_SUCCESS
	//
	//Note1: 		Range: 55.25 MHz – 213 MHz (Channels 2 – 13)
	//Note2: 		Resolution: 250 KHz
	//
	//Example:	Board973_SetNtscFrequency (0, 57000000);
	//
	__declspec(dllimport) long WINAPI Board973_SetNtscFrequency(long lngDevice, double dblFrequency);

	//Board973_Set8VsbFrequency (See Set8VSBAdjustChannel973)
	//                                    
	//Function:	Set the 8VSB output frequency
	//
	//Syntax:	long Board973_Set8VsbFrequency(long lngDevice , double frequency); 
	//
	//Parameters:	lngDevice – The device to control.
	//				lngFrequency – The frequency of 8VSB output.
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or out of frequency range.
	//				STATUS_SUCCESS
	//
	//Note1: 		Range: 470 MHz – 520 MHz (Channels 14 – 21)
	//Note2: 		Resolution: 100 KHz
	//
	//Example:	Board973_Set8VsbFrequency (0, 470000000);
	//
	__declspec(dllimport) long WINAPI Board973_Set8VsbFrequency(long lngDevice, double dblFrequency);

	//Board973_SetTvct
	//                                    
	//Function:	Set TVCT major_channel_number and minor_channel_number.
	//
	//Syntax:	long Board973_SetTvct(long lngDevice , int iMajor, int iMinor); 
	//
	//Parameters:	lngDevice – The device to control.
	//				iMajor – The major channel number (1-999).
	//				iMinor – The minor channel number (0-999).
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or out of frequency range.
	//				STATUS_SUCCESS
	//
	//Example:	Board973_SetTvct (0, 14, 1);
	//
	__declspec(dllimport) long WINAPI Board973_SetTvct(long lngDevice , int iMajor, int iMinor);

	//Board973_Set8VsbVct
	//                                    
	//Function:	Set 8VSB frequency, VCT major_channel_number and minor_channel_number, and VCT select (Cable or Terrestrial).
	//
	//Syntax:	long Board973_Set8VsbVct(long lngDevice , double dblFrequency, int iMajor, int iMinor, int iMode); 
	//
	//Parameters:	lngDevice – The device to control.
	//				dblFrequency – The frequency of 8VSB output.
	//				iMajor – The major channel number (1-999).
	//				iMinor – The minor channel number (0-999).
	//				iMode – Table selection TVCT or CVCT (default: TVCT)
	//						0 – TVCT, 1 - CVCT
	//
	//Note1: 		Range: 470 MHz – 520 MHz (Channels 14 – 21)
	//Note2: 		Resolution: 100 KHz
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or out of frequency range.
	//				STATUS_SUCCESS
	//
	//Example:	Board973_Set8VsbVct (0, 473000000, 66, 1, 0);
	//
	__declspec(dllimport) long WINAPI Board973_Set8VsbVct(long lngDevice, double dblFrequency, int iMajor,int iMinor, int iMode);

	//Board973_SetVct
	//                                    
	//Function:	VCT major_channel_number and minor_channel_number, and VCT select (Cable or Terrestrial).
	//
	//Syntax:	long Board973_SetVct(long lngDevice , int iMajor, int iMinor, int iMode); 
	//
	//Parameters:	lngDevice – The device to control.
	//				iMajor – The major channel number (1-999).
	//				iMinor – The minor channel number (0-999).
	//				iMode – Table selection TVCT or CVCT (default: TVCT)
	//						0 – TVCT, 1 - CVCT
	//
	//Return Value:	STATUS_FAILURE – Hardware not initialized, DLL failure, or out of frequency range.
	//				STATUS_SUCCESS
	//
	//Example:	Board973_SetVct (0, 66, 1, 0);
	//
	__declspec(dllimport) long WINAPI Board973_SetVct(long lngDevice , int iMajor,int iMinor, int iMode);

	// Undocumented but interesting functions...
	__declspec(dllimport) long WINAPI Board973_GetCardCount();
	__declspec(dllimport) long WINAPI IndirectMemoryWrite973(long lngDevice, LPBYTE cbBuffer, long lUnk1, long lUnk2);
	__declspec(dllimport) long WINAPI IndirectMemoryRead973(long lngDevice, LPBYTE cbBuffer, long lngBufferLen);
	__declspec(dllimport) long WINAPI DirectMemoryWrite973(long lngDevice, LPBYTE cbBuffer, long lngBufferLen);
	__declspec(dllimport) long WINAPI DirectMemoryRead973(long lngDevice, long* plngUnk1);
	__declspec(dllimport) long WINAPI Board973_GetId();
	__declspec(dllimport) long WINAPI Board973_GetStatus(long lngDevice);
	__declspec(dllimport) long WINAPI Board973_Revisions(long lngDevice, LPTSTR pszUnk1, LPTSTR pszUnk2);
	__declspec(dllimport) long WINAPI Board973_GetProgramCountFileEx(long lngDevice, long* plUnk1, long* plUnk2);
	__declspec(dllimport) long WINAPI Board973_ConfigurePsiAndPsip(long lngDevice, LPTSTR pszUnk1);
	__declspec(dllimport) long WINAPI Board973_GetProgramInfo(long lngDevice, long* plUnk1);
	__declspec(dllimport) long WINAPI Board973_RegisterOnGeneric(long lngDevice, PFN_ON_GENERIC pfn, long lUnk1);
	__declspec(dllimport) long WINAPI Board973_GetEeprom(long lngDevice, long unk, LPBYTE cbBuffer, long nBufferLen);
	__declspec(dllimport) long WINAPI Board973_SetEeprom(long lngDevice, long unk, LPBYTE cbBuffer, long nBufferLen);

	// Obsolete or duplicate functions
	// Obsolete __declspec(dllimport) long WINAPI SetFastPlayCount973@4 @16
	// Obsolete __declspec(dllimport) long WINAPI SetPMTSearchDelay973@4 @18
	// Obsolete __declspec(dllimport) long WINAPI EnableAutoNull973@4 @24
	// Obsolete __declspec(dllimport) long WINAPI GetPids973@4 @25
	// Obsolete __declspec(dllimport) long WINAPI GetPidsFile973@4 @26
	// Obsolete __declspec(dllimport) long WINAPI SetNullPacketCount973@4 @29
	// Obsolete __declspec(dllimport) long WINAPI SetNTSCClockExtension973(long lngDevice, long lExtension);
	// See Board973_Write __declspec(dllimport) long WINAPI DirectMemoryWrite973@12 @27
	// See Board973_Read __declspec(dllimport) long WINAPI DirectMemoryRead973@8 @28
	// See Initialize973 __declspec(dllimport) long WINAPI Board973_Initialize@4 @103
	// See ExecutePlay973 __declspec(dllimport) long WINAPI Board973_Play@20 @104
	// See ExecuteStop973 __declspec(dllimport) long WINAPI Board973_Stop@4 @105
	// See OnStatusSetup973 __declspec(dllimport) long WINAPI Board973_RegisterOnStatus@4 @106
	// See OnProgressSetup973 __declspec(dllimport) long WINAPI Board973_RegisterOnProgress@4 @107
	// See Uninitialize973 __declspec(dllimport) long WINAPI Board973_Uninitialize@4 @108
	// See ExecutePlaySeamless973 __declspec(dllimport) long WINAPI Board973_PlaySeamless@8 @109
	// See GetRevisionXilinx97 __declspec(dllimport) long WINAPI Board973_RevisionFpga@8 @111
	// See SetNTSCAdjustChannel973 __declspec(dllimport) long WINAPI Board973_SetNtsc@8 @112
	// See Set8VSBAdjustChannel973 __declspec(dllimport) long WINAPI Board973_Set8Vsb@8 @113
	// See SetAspectRaio973 __declspec(dllimport) long WINAPI Board973_SetAspectRatio@12 @114
	// See OnStatusSetup973 __declspec(dllimport) long WINAPI Board973_SetupOnStatus@8 @115
	// See OnProgressSetup973 __declspec(dllimport) long WINAPI Board973_SetupOnProgress@8 @116
	// See GetCurrentPlayingFile973 __declspec(dllimport) long WINAPI	(long lngDevice, LPTSTR pszBuffer);
	// See Initialie973 __declspec(dllimport) long WINAPI Board973_GenericIn@8 @127
}

LONG g_nDevice = -1;

HANDLE g_hExitEvent = NULL;

void log(LPCTSTR format, ...)
{
	va_list argptr;
	time_t ltime = time(NULL);
	const char* timestamp = asctime(localtime(&ltime));
	_tprintf(_T("%.*hs "), strlen(timestamp) - 1, timestamp); 
	va_start(argptr, format);
	_vtprintf(format, argptr);
	va_end(argptr);
}

void CALLBACK OnProgress(long nDevice, long nPercent)
{
	log(_T("%d) PROGRESS: 0x%08X\n"), nDevice, nPercent);
}

void CALLBACK OnStatus(long nDevice, long nStatus)
{
	switch (nStatus)
	{
	case STATUS_SOCKET_LISTEN:
		log(_T("Socket listen...\n"));
		break;
	case STATUS_PLAY_START:
		log(_T("Play starting...\n"));
		break;
	case STATUS_PLAY_COMPLETE:
		log(_T("Play stopping...\n"));
		break;
	case STATUS_BUFFER_EMPTY_PAUSED:
		log(_T("Buffer empty. Pausing...\n"));
		break;
	case STATUS_BUFFER_FULL_PLAYING:
		log(_T("Buffer full. Resuming...\n"));
		break;
	default:
		log(_T("STATUS: 0x%08X\n"), nStatus);
		break;
	}
}

BOOL CtrlHandler(DWORD dwCtrlType) 
{ 
	static BOOL g_bBeenHere = FALSE;

	if (dwCtrlType == CTRL_C_EVENT) 
	{ 
		if (g_bBeenHere == TRUE)
			ExitProcess(255);

		if (g_hExitEvent != NULL)
		{
			_tprintf(_T("Ctrl-C detected!\n"));

			SetEvent(g_hExitEvent);

			//ExecuteStop973(g_nDevice); // Unfortunately, calling ExecuteStop973 didn't sto the API's Play thread.
			Uninitialize973(g_nDevice);  // So instead I'm calling Uninitialize973.

			g_bBeenHere = TRUE;
		}

		return TRUE;
	}

	return FALSE; 
} 

int Usage()
{
	_tprintf(_T("HDTV998Server device_id atsc_ch [ntsc_ch] [listen_port]\n"));
	_tprintf(_T("\n"));
	_tprintf(_T(" device_id    Specifies the card index to use.\n"));
	_tprintf(_T(" atsc_ch      Specifies the ATSC channel number (14 to 21).\n"));
	_tprintf(_T(" ntsc_ch      Specifies the NTSC channel number (3 to 13).\n"));
	_tprintf(_T(" listen_port  Specifies the TCP listening port for TS data.\n"));
	_tprintf(_T("              The default TCP port will be 9980 + device_id.\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("Example: HDTV998Server 0 14\n"));
	_tprintf(_T("Run server for card 0, ATSC ch14, listening on TCP port 9980.\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("Example: HDTV998Server 1 15\n"));
	_tprintf(_T("Run server for card 0, ATSC ch15, listening on TCP port 9981.\n"));
	return 2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	long nNtscChannel = 0;
	long nAtscChannel = 0;
	long nServerPort = 9980;

	_tprintf(_T("HDTV998Server v0.2 by stefvie2k\n\n"));

	if (argc < 3)
		return Usage();

	g_nDevice = _ttoi(argv[1]);
	if (errno == ERANGE)
	{
		_tprintf(_T("Invalid device_id value.\n"));
		return Usage();
	}

	nAtscChannel = _ttoi(argv[2]);
	if (errno == ERANGE)
	{
		_tprintf(_T("Invalid atsc_ch value.\n"));
		return Usage();
	}

	if (argc > 3)
	{
		nNtscChannel = _ttoi(argv[3]);
		if (errno == ERANGE)
		{
			_tprintf(_T("Invalid ntsc_ch value.\n"));
			return Usage();
		}
	}

	if (argc > 4)
	{
		nServerPort = _ttoi(argv[4]);
		if (errno == ERANGE)
		{
			_tprintf(_T("Invalid port value.\n"));
			return Usage();
		}
	}
	else
	{
		nServerPort = 9980 + g_nDevice;
	}

	_tprintf(_T("Server starting for device %d (NTSC: ch%d, ATSC: ch%d (real)).\n"), 
		g_nDevice, 
		nNtscChannel, 
		nAtscChannel);

	g_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_hExitEvent == NULL)
	{
		_tprintf(_T("CreateEvent failed (%d)\n"), GetLastError());
		return -1;
	}

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		_tprintf(_T("SetConsoleCtrlHandler failed (%d)\n"), GetLastError());
		return -1;
	}

	int nResult = Initialize973(g_nDevice);
	if (nResult != STATUS_SUCCESS)
	{
		_tprintf(_T("Initialize973 failed (%d)\n"), nResult);
		return 1;
	}

	OnStatusSetup973(g_nDevice, OnStatus);

	__try
	{
		long nRevDevice = 0;
		double dblRevDLL = 0.0f, dblRevDriver = 0.0f;

		long lngResult = GetRevisions973(g_nDevice, &nRevDevice, &dblRevDLL, &dblRevDriver);
		if (lngResult != STATUS_SUCCESS)
		{
			_tprintf(_T("GetRevisions973 failed (%d)\n"), nResult);
			return 1;
		}

		_tprintf(_T("\n"));

		_tprintf(_T("	Device Revision: %X\n"), nRevDevice); // 7030-C
		_tprintf(_T("	DLL Revision...: %.2f\n"), dblRevDLL);
		_tprintf(_T("	Driver Revision: %.2f\n"), dblRevDriver);

		double dblRevXilinx = 0.0f;
		long nBoardType;

		nResult = GetRevisionXilinx973(g_nDevice, &dblRevXilinx, &nBoardType);
		if (nResult != STATUS_SUCCESS)
		{
			_tprintf(_T("GetRevisionXilinx973 failed (%d)\n"), nResult);
			return 1;
		}

		LPCTSTR pszBoardType = _T("Unknown");
		switch (nBoardType)
		{
		case BOARD_TYPE_300_A:
			pszBoardType = _T("300-A");
			break;
		case BOARD_TYPE_300_B:
			pszBoardType = _T("300-B");
			break;
		case BOARD_TYPE_400_A:
			pszBoardType = _T("400-A");
			break;
		}

		_tprintf(_T("	Xilinx Revision: %.2f\n"), dblRevXilinx);
		_tprintf(_T("	Board Type.....: %s\n"), pszBoardType);
		
		_tprintf(_T("\n"));

		if (nNtscChannel > 0)
		{
			nResult = SetNTSCAdjustChannel973(g_nDevice, nNtscChannel); // disable?
			if (nResult != STATUS_SUCCESS)
			{
				_tprintf(_T("SetNTSCAdjustChannel973 failed (%d)\n"), nResult);
				return 1;
			}
		}

		if (nAtscChannel > 0)
		{
			nResult = Set8VSBAdjustChannel973(g_nDevice, nAtscChannel);
			if (nResult != STATUS_SUCCESS)
			{
				_tprintf(_T("Set8VSBAdjustChannel973 failed (%d)\n"), nResult);
				return 1;
			}
		}

		// The following is key to the whole socket thing for the Board973 API.
		// Using "LOCALHOST:nnn" instructs the API to using the SocketServer.dll (if found) for
		// it's transport stream.

		CHAR szFileName[_MAX_PATH + 1];
		sprintf(szFileName, "LOCALHOST:%d", nServerPort);
		ExecutePlay973(g_nDevice, szFileName, 0, 100, 0);

		WaitForSingleObject(g_hExitEvent, INFINITE);
	}
	__finally
	{
		Uninitialize973(g_nDevice);
	}

	return 0;
}

