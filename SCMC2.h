
#ifndef _SCMC2_H_
#define _SCMC2_H_

//	include Resource Manager definition
#include "winscard.h"

// Define your own Window Message to deal with the Com Port events.
// Note: If you have defined another Window Message with the same ID "WM_USER + 5", you need to change the plus value.								
#define WM_CARDINSERTED WM_USER + 7 
#define WM_CARDREMOVED WM_USER + 8 

#define IOCTL_MAGNETICCCARD_COMMAND				SCARD_CTL_CODE(0X900) // IO control commands of stcdrv.vxd
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE		SCARD_CTL_CODE(3500)	


//-- Reader error definitions --------------------------------------------
#define READER_S_SUCCESS		100L // magnetic card's commands succeed
#define READER_E_DATADECODE		301L // data error in decode ( do have physical magnetic response but not right format )
#define READER_E_DATAEMPTY		302L // data empty (usually the wrong side)
#define READER_E_TRACKUNKNOWN	303L // track unknown

#define READER_E_UNKNOWN		999L // unknown response

//-- Connection Status ------------------------------------------------------
#define CONNECTION_NO		0
#define CONNECTION_SCARD	1
#define CONNECTION_READER	2
#define CONNECTION_RFID		3
#define CONNECTION_MAGCARD	4

//-- Magnetic Card insert states --------------------------------------------
#define CARD_NOCARD			-1
#define CARD_REMOVED		0
#define CARD_INSERTED		1

#define MAX_MESSAGE			256	
#define NUMBER_OF_READERS	4
#define READERNAME_LENGTH	128
#define MAX_SEND			260
#define MAX_RECEIVE			260 // every record has no more than 255 byte + 0x90 + 0x00
#define MAX_OUTFORMAT		1024 // 260*3
#define MAX_TRACK_DATA		128 // for ASCII mode
#define MAX_TRACK_Bitmap	128 // for Bitmap mode

//-- Magnetic Card Command --------------------------------------------------
#define SIN_ReadMagneticCardData	"00860000FC"

// PCSC_SCMC: a structure for PCSC and the SB520 reader
typedef struct 
{
//-- PCSC variables------------------------------------------------------------------
	SCARDCONTEXT	SC_Context; // SCard Context handle structure
	SCARDHANDLE		SC_Handle;  // SCard handle structure
											
	long	ret;			// the result of SCard API
	TCHAR	szMsg[MAX_MESSAGE];	// for error msg or card status

	DWORD	dwActReader;	// which reader is active
	DWORD	dwReaderCount;	// the total count of readers
	TCHAR	mszReaderName[NUMBER_OF_READERS][READERNAME_LENGTH];

	SCARD_IO_REQUEST	IO_Request;	// for PCSC_SCard_Transmit to use
	DWORD				dwCardStatus;	// card status
	DWORD				dwActProtocol;	// what kind of protocol is using
	int					iConnectionStatus;	// CONNECTION_NO: no connection
											// CONNECTION_SCARD: smart card connected after PCSC_SCARD_Connect() succeeds.
											// CONNECTION_READER: reader connected after PCSC_READER_Connect() succeeds.

	BYTE	brSendBuf[MAX_SEND];// buffer for sending data to the reader or smart card
	DWORD	dwSendLen;
	BYTE	brRecvBuf[MAX_RECEIVE];// buffer for received data from the reader or smart card
	DWORD	dwRecvLen;
	TCHAR	szOutFormat[MAX_OUTFORMAT];// turn byte data into readable string	

//-- Reader variables-------------------------------------------------------------
	DWORD	dwMCardMode;	// response data's mode, ASCII (0) or Bitmap (1) mode
	long	lMCardError;	// Magnetic card error code
	int		iCardInsert;	// MCARD_NOCARD: no magnetic card in the reader
							// MCARD_INSERTED: a magnetic card in the reader
							// MCARD_REMOVED: This card has been removed. The reader has got 3 tracks' data. iMCardInsert=-1;

	TCHAR	TrackData[3][MAX_TRACK_DATA];
	BYTE	TrackBitmap[3][MAX_TRACK_Bitmap];
	
//-- Card event threat -----------------------------
	HANDLE	hThread; // the handle of the created read-thread
	BOOL	bIsRunning;
	HWND	hTargetWnd; // The target-window handle which WM_DATACOMING is sent to

//-- RFID -------------------------------------------
	SCARDCONTEXT	RFID_Context;
	SCARDHANDLE		RFID_Handle;

//-- Magnetic Card -------------------------------------------
	SCARDCONTEXT	MAGCard_Context;
	SCARDHANDLE		MAGCard_Handle;


} PCSC_SCMC, *PPCSC_SCMC, *LPPCSC_SCMC;


//-- System Functions -----------------------------------------------------------
BOOL PCSC_EstablishContext(LPPCSC_SCMC pscmc);
BOOL PCSC_ReleaseContext(LPPCSC_SCMC pscmc);
BOOL PCSC_GetReaderNames(LPPCSC_SCMC pscmc);

LPTSTR PCSC_GetLastError(LPPCSC_SCMC pscmc); // seee scarderr.h
void PCSC_ByteToOutFormat(const BYTE *brSource, DWORD brLen, TCHAR *szTarget);
BOOL PCSC_AToSendBuf(const TCHAR *szSource, LPPCSC_SCMC pscmc);

//-- Smart Card Functions -------------------------------------------------------
BOOL PCSC_SCARD_Connect(LPPCSC_SCMC pscmc);
BOOL PCSC_SCARD_Disconnect(LPPCSC_SCMC pscmc);
BOOL PCSC_SCARD_Transmit(LPPCSC_SCMC pscmc);
BOOL PCSC_GetCardStatus(LPPCSC_SCMC pscmc);  

//-- Reader Functions ----------------------------------------------------
//BOOL PCSC_READER_Connect(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_Disconnect(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_Transmit(LPPCSC_SCMC pscmc);

//BOOL PCSC_READER_DetectCard(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_GetGrnLedState(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_GrnLedOn(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_GrnLedOff(LPPCSC_SCMC pscmc);

/* set response data's mode, ASCII (0) or Bitmap (1) mode */
//BOOL PCSC_READER_SetMCDataMode(DWORD dwMode, LPPCSC_SCMC pscmc); 
//BOOL PCSC_READER_GetT1Data(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_GetT2Data(LPPCSC_SCMC pscmc);
//BOOL PCSC_READER_GetT3Data(LPPCSC_SCMC pscmc);

//-- RFID Mifare Card Function ---------------------------------------------
BOOL PCSC_RFID_Connect(LPPCSC_SCMC pscmc);
BOOL PCSC_RFID_Disconnect(LPPCSC_SCMC pscmc);
BOOL PCSC_RFID_Transmit(LPPCSC_SCMC pscmc);

//-- Magnetic Card Function ------------------------------------------------
BOOL PCSC_MagCard_Connect(LPPCSC_SCMC pscmc);
BOOL PCSC_MagCard_Disconnect(LPPCSC_SCMC pscmc);
BOOL PCSC_MagCard_GetTrackData(LPPCSC_SCMC pscmc);

#endif // #ifndef _SCMC2_H_