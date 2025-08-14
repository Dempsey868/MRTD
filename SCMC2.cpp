//**************************************************************************************
//Version		Date		Author		Description			
//V2.6			2001/10/15	Pauline Ma	Modify source from SCM' demo code.
//V3.0			2003/09/04	Pauline Ma	Add a thread to deal with card-insert and card-remove events.
//V3.2			2003/09/05	Pauline Ma	Add Sleep(0 ms) in thread to make it have a break.

#include "Stdafx.h"
#include "SCMC2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT	CardEventThreadFunc(LPVOID);

//-- Functions for internal use -------------------------------------------
void ByteToStr(LPBYTE pb, LPTSTR sz)
{
	if (*pb==0x25 || *pb==0x3B) // ignore first signal byte '%' or ';'
		pb++;
	while ((*pb>=0x20) && (*pb<=0x5F) && (*pb!=0x3F)) // right ASCII format
		*sz++ = (TCHAR)*pb++;
	
	*sz = _T('\0');
}

//-- System Functions -----------------------------------------------------------
BOOL PCSC_EstablishContext(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &pscmc->SC_Context);
	if (pscmc->ret==SCARD_S_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

BOOL PCSC_ReleaseContext(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardReleaseContext(pscmc->SC_Context);
	if (pscmc->ret==SCARD_S_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

BOOL PCSC_GetReaderNames(LPPCSC_SCMC pscmc)
{
	LPTSTR			pmszReaders = NULL; // the multiple reader names
	LPTSTR			pReader;
	DWORD           cch = SCARD_AUTOALLOCATE;	

	pscmc->ret = SCardListReaders(pscmc->SC_Context, NULL, (LPTSTR)&pmszReaders, &cch);

	if (pscmc->ret != SCARD_S_SUCCESS) 
		return FALSE;
	
	// Do something with the multi string of readers.
    // Here, we'll merely output the values.
    // A double-null terminates the list of values.
	pscmc->dwReaderCount = 0;
    pReader = pmszReaders;
    while ( _T('\0') != *pReader )
    {
		_tcscpy(pscmc->mszReaderName[pscmc->dwReaderCount], pReader );
		
		pscmc->ret = SCardConnect(pscmc->SC_Context, pscmc->mszReaderName[pscmc->dwReaderCount], SCARD_SHARE_SHARED, 
							SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &pscmc->SC_Handle, &pscmc->dwActProtocol);
		if (pscmc->ret != SCARD_E_UNKNOWN_READER) // -2146435063 == 0X80100009 == SCARD_E_NO_READERS_AVAILABLE
			pscmc->dwReaderCount++;
		if (pscmc->ret == SCARD_S_SUCCESS)
			SCardDisconnect(pscmc->SC_Handle, SCARD_LEAVE_CARD);

        // Advance to the next value.
        pReader = pReader + _tcslen(pReader) + 1;
		
	}
    // Free the memory.
    pscmc->ret = SCardFreeMemory(pscmc->SC_Context, pmszReaders );
    if ( pscmc->ret != SCARD_S_SUCCESS )
		return FALSE;
             
	if (pscmc->dwReaderCount == 0) 
	{
		pscmc->ret = SCARD_E_NO_READERS_AVAILABLE;
		return FALSE;
	} 

	return TRUE;
}

void PCSC_ByteToOutFormat(const BYTE *brSource, DWORD brLen, TCHAR *szTarget)
{
	LPTSTR	pTemp = szTarget;

	for (DWORD i=0; i<brLen; i++) 
	{
		_stprintf( pTemp, _T("%.2x "), brSource[i] );
		pTemp += 3;		
	}
	*pTemp = _T('\0');

}

BOOL PCSC_AToSendBuf(const TCHAR *szSource, LPPCSC_SCMC pscmc)
{
	DWORD dwLen = _tcslen(szSource);
	if ( dwLen < 2 ) 
		return FALSE;

	BOOL  bOdd = TRUE;
	DWORD j = 0;
	for (DWORD i=0; i<dwLen; i++) 
	{
		if ( ((szSource[i] < _T('0')) || (szSource[i] > _T('9'))) &&			
			 ((szSource[i] != _T(' ')) && (szSource[i] != _T(','))) &&
			 ((szSource[i] < _T('a')) || (szSource[i] > _T('f'))) && 
			 ((szSource[i] < _T('A')) || (szSource[i] > _T('F'))) ) 
		{ 
			return FALSE;
		}
		else if (_istxdigit(szSource[i]))			
		{
			pscmc->brSendBuf[j] <<= 4;                 
			if (_istalpha(szSource[i]))              
				pscmc->brSendBuf[j] |= (_totlower(szSource[i]) - 0x57); 
			else
				pscmc->brSendBuf[j] |= (szSource[i] - 0x30);

			if (bOdd)	// odd							
				bOdd = FALSE;
			else		// even
			{
				bOdd = TRUE;
				j++;
			}
		}
	}

	pscmc->dwSendLen = j;

	return TRUE;
}
/*
BOOL PCSC_GetStatusChange(LPPCSC_SCMC pscmc)
{
	pscmc->SC_RS.szReader = pscmc->mszReaderName[pscmc->dwActReader];
	pscmc->SC_RS.dwCurrentState = SCARD_STATE_UNAWARE;
	pscmc->ret = SCardGetStatusChange(pscmc->SC_Context, INFINITE, &pscmc->SC_RS, 1);
	if (pscmc->ret == SCARD_S_SUCCESS)
	{
		switch (pscmc->SC_RS.dwEventState)
		{
		case 0x00000012:
			_tcscpy( pscmc->szMsg, _T("There is no card in the reader.") );
			break;
		case 0x00000022:
			_tcscpy( pscmc->szMsg, _T("There is a smart card in the reader.") );
			break;
		case 0x00000222:
			_tcscpy( pscmc->szMsg, _T("There is a non-smart card in the reader.") );
			break;
		default:
			_tcscpy( pscmc->szMsg, _T("Unknown status.") );
			break;
		}

		return TRUE;
	}
	else
		return FALSE;
}
*/
BOOL PCSC_GetCardStatus(LPPCSC_SCMC pscmc)
{
	TCHAR	szReader[128];
	DWORD	dwReaderLen = 128;
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);
	pscmc->dwRecvLen = 32; // cannot be 0, it is a in-out parameter not just in parameter	

	pscmc->ret = SCardStatus(pscmc->SC_Handle, szReader, &dwReaderLen, &pscmc->dwCardStatus, &pscmc->dwActProtocol, 
					pscmc->brRecvBuf, &pscmc->dwRecvLen);	
	if (pscmc->ret == SCARD_S_SUCCESS)
	{
		memset(pscmc->szMsg, 0, sizeof(pscmc->szMsg));

		switch (pscmc->dwCardStatus) 
		{
			case SCARD_UNKNOWN:
				_tcscpy( pscmc->szMsg, _T("the driver is unaware of the current state of the reader.") );
				pscmc->szMsg; break;

			case SCARD_ABSENT:
				_tcscpy( pscmc->szMsg, _T("There is no card in the reader.") );
				pscmc->szMsg; break;

			case SCARD_PRESENT:
				_tcscpy( pscmc->szMsg, _T("There is a card in the reader, but it cannot be reset or communicated with.") );
				pscmc->szMsg; break;

			case SCARD_SWALLOWED:
				_tcscpy( pscmc->szMsg, _T("There is a smart card in the reader in position for use. The card is not powered.") );
				pscmc->szMsg; break;

			case SCARD_POWERED:
				_tcscpy( pscmc->szMsg, _T("Power is being provided to the smart card, but the reader driver is unware of the mode of the card.") );
				pscmc->szMsg; break;

			case SCARD_NEGOTIABLE:
				_tcscpy( pscmc->szMsg, _T("The smart card has been reset and is awaiting PTS negotiation.") );
				pscmc->szMsg; break;

			case SCARD_SPECIFIC:
				_tcscpy( pscmc->szMsg, _T("There is a smart card in the reader. It has been reset and specific communication protocols have been established.") );
				pscmc->szMsg; break;
			
			default:
				_stprintf(pscmc->szMsg, _T("Unknown dwState value: 0x%X "), pscmc->dwCardStatus);
				pscmc->szMsg; break;
		}

		return TRUE;
	}
	else	
		return FALSE;
}

LPTSTR PCSC_GetLastError(LPPCSC_SCMC pscmc)
{
	memset(pscmc->szMsg, 0, sizeof(pscmc->szMsg));

	switch (pscmc->ret) {
		case SCARD_E_CANCELLED:
			_tcscpy( pscmc->szMsg, _T("The action was cancelled by an SCardCancel request.") );
			return pscmc->szMsg; //break;
			
		case SCARD_E_CANT_DISPOSE:
			_tcscpy( pscmc->szMsg, _T("The system could not dispose of the media in the requested manner.") );
			return pscmc->szMsg;
			
		case SCARD_E_CARD_UNSUPPORTED:
			_tcscpy( pscmc->szMsg, _T("The smart card does not meet minimal requirements for support.") );
			return pscmc->szMsg;
			
		case SCARD_E_DUPLICATE_READER:
			_tcscpy( pscmc->szMsg, _T("The reader driver didn't produce a unique reader name.") );
			return pscmc->szMsg;
			
		case SCARD_E_INSUFFICIENT_BUFFER:
			_tcscpy( pscmc->szMsg, _T("The data buffer to receive returned data is too small for the returned data.") );
			return pscmc->szMsg;

		case SCARD_E_INVALID_ATR:
			_tcscpy( pscmc->szMsg, _T("An ATR obtained from the registry is not a valid ATR string.") );
			return pscmc->szMsg;

		case SCARD_E_INVALID_HANDLE:
			_tcscpy( pscmc->szMsg, _T("The supplied handle was invalid.") );
			return pscmc->szMsg;

		case SCARD_E_INVALID_PARAMETER:
			_tcscpy( pscmc->szMsg, _T("One or more of the supplied parameters could not be properly interpreted.") );
			return pscmc->szMsg;

		case SCARD_E_INVALID_TARGET:
			_tcscpy( pscmc->szMsg, _T("Registry startup information is missing or invalid.") );
			return pscmc->szMsg;

		case SCARD_E_INVALID_VALUE:
			_tcscpy( pscmc->szMsg, _T("One or more of the supplied parameters values could not be properly interpreted.") );
			return pscmc->szMsg;

		case SCARD_E_NOT_READY:
			_tcscpy( pscmc->szMsg, _T("The reader or card is not ready to accept commands.") );
			return pscmc->szMsg;

		case SCARD_E_NOT_TRANSACTED:
			_tcscpy( pscmc->szMsg, _T("An attempt was made to end a non-existent transaction.") );
			return pscmc->szMsg;

		case SCARD_E_NO_MEMORY:
			_tcscpy( pscmc->szMsg, _T("Not enough memory available to complete this command.") );
			return pscmc->szMsg;

		case SCARD_E_NO_READERS_AVAILABLE:
			_tcscpy( pscmc->szMsg, _T("None of the specified readers are currently available for use.") );
			return pscmc->szMsg;

		case SCARD_E_NO_SERVICE:
			_tcscpy( pscmc->szMsg, _T("The Smart card resource manager is not running.") );
			return pscmc->szMsg;

		case SCARD_E_NO_SMARTCARD:
			_tcscpy( pscmc->szMsg, _T("The operation requires a smart card but no smart card is currently in the device.") );
			return pscmc->szMsg;

		case SCARD_E_PCI_TOO_SMALL:
			_tcscpy( pscmc->szMsg, _T("The PCI Receive buffer was too small.") );
			return pscmc->szMsg;

		case SCARD_E_PROTO_MISMATCH:
			_tcscpy( pscmc->szMsg, _T("The requested protocols are incompatible with the protocol currently in use with the card.") );
			return pscmc->szMsg;

		case SCARD_E_READER_UNAVAILABLE:
			_tcscpy( pscmc->szMsg, _T("The specified reader is not currently available for use.") );
			return pscmc->szMsg;

		case SCARD_E_READER_UNSUPPORTED:
			_tcscpy( pscmc->szMsg, _T("The reader driver does not meet minimal requirements for support.") );
			return pscmc->szMsg;

		case SCARD_E_SERVICE_STOPPED:
			_tcscpy( pscmc->szMsg, _T("The Smart card resource manager has shut down.") );
			return pscmc->szMsg;

		case SCARD_E_SHARING_VIOLATION:
			_tcscpy( pscmc->szMsg, _T("The card cannot be accessed because of other connections outstanding.") );
			return pscmc->szMsg;

		case SCARD_E_SYSTEM_CANCELLED:
			_tcscpy( pscmc->szMsg, _T("The action was cancelled by the system presumably to log off or shut down.") );
			return pscmc->szMsg;

		case SCARD_E_TIMEOUT:
			_tcscpy( pscmc->szMsg, _T("The user-specified timeout value has expired.") );
			return pscmc->szMsg;

		case SCARD_E_UNKNOWN_CARD:
			_tcscpy( pscmc->szMsg, _T("The specified card name is not recognized.") );
			return pscmc->szMsg;

		case SCARD_E_UNKNOWN_READER:
			_tcscpy( pscmc->szMsg, _T("The specified reader name is not recognized.") );
			return pscmc->szMsg;

		case SCARD_F_COMM_ERROR:
			_tcscpy( pscmc->szMsg, _T("An internal communications error has been detected.") );
			return pscmc->szMsg;

		case SCARD_F_INTERNAL_ERROR:
			_tcscpy( pscmc->szMsg, _T("An internal consistency check failed.") );
			return pscmc->szMsg;

		case SCARD_F_UNKNOWN_ERROR:
			_tcscpy( pscmc->szMsg, _T("An internal error has been detected but the source is unknown.") );
			return pscmc->szMsg;

		case SCARD_F_WAITED_TOO_LONG:
			_tcscpy( pscmc->szMsg, _T("An internal consistency timer has expired.") );
			return pscmc->szMsg;

		case SCARD_S_SUCCESS:
			_tcscpy( pscmc->szMsg, _T("OK!") );
			return pscmc->szMsg;

		case SCARD_W_REMOVED_CARD:
			_tcscpy( pscmc->szMsg, _T("The card has been removed so that further communication is not possible.") );
			return pscmc->szMsg;

		case SCARD_W_RESET_CARD:
			_tcscpy( pscmc->szMsg, _T("The card has been reset so any shared state information is invalid.") );
			return pscmc->szMsg;

		case SCARD_W_UNPOWERED_CARD:
			_tcscpy( pscmc->szMsg, _T("Power has been removed from the card so that further communication is not possible.") );
			return pscmc->szMsg;

		case SCARD_W_UNRESPONSIVE_CARD:
			_tcscpy( pscmc->szMsg, _T("The card is not responding to a reset.") );
			return pscmc->szMsg;

		case SCARD_W_UNSUPPORTED_CARD:
			_tcscpy( pscmc->szMsg, _T("The reader cannot communicate with the card due to ATR configuration conflicts.") );
			return pscmc->szMsg;

		default:
			_stprintf(pscmc->szMsg, _T("Function returned 0x%X error code."), pscmc->ret);			
			return pscmc->szMsg;			
	}
}

//-- Smart Card Functions -------------------------------------------------------
BOOL PCSC_SCARD_Connect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardConnect(pscmc->SC_Context, pscmc->mszReaderName[pscmc->dwActReader], SCARD_SHARE_SHARED, 
				SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &pscmc->SC_Handle, &pscmc->dwActProtocol);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_SCARD;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_SCARD_Disconnect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardDisconnect(pscmc->SC_Handle, SCARD_UNPOWER_CARD);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_NO;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_SCARD_Transmit(LPPCSC_SCMC pscmc)
{
	pscmc->IO_Request.dwProtocol = pscmc->dwActProtocol;
	pscmc->IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);	
	pscmc->dwRecvLen = 260; // IN OUT Parameter, you need to tell the buff size

	pscmc->ret = SCardTransmit(pscmc->SC_Handle, &pscmc->IO_Request, pscmc->brSendBuf, 
			pscmc->dwSendLen, 0, pscmc->brRecvBuf, &pscmc->dwRecvLen);
	if (pscmc->ret==SCARD_S_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/*-- Reader Functions ----------------------------------------------------
BOOL PCSC_READER_Connect(LPPCSC_SCMC pscmc)
{
	DWORD	dwThreadID;

	pscmc->ret = SCardConnect(pscmc->SC_Context, pscmc->mszReaderName[pscmc->dwActReader], 
		SCARD_SHARE_DIRECT,	0, &pscmc->SC_Handle, &pscmc->dwActProtocol);
	if (pscmc->ret!=SCARD_S_SUCCESS)
		return FALSE;

	pscmc->iConnectionStatus = CONNECTION_READER;
	pscmc->hThread = NULL;
	pscmc->bIsRunning = TRUE;
    if ( pscmc->hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)CardEventThreadFunc,pscmc, 0, &dwThreadID) )
	{
		CloseHandle (pscmc->hThread); // ReadThreadFunc will keep running while psr->hComm is still valid.
		return TRUE;
	}		
	else
	{
		//MessageBox(_T("ERROR: Failed to Create a ReadThread!"), _T("SRCEDemo"), MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
		return FALSE;
	}
}

BOOL PCSC_READER_Disconnect(LPPCSC_SCMC pscmc)
{
	pscmc->bIsRunning = FALSE;
	pscmc->ret = SCardDisconnect(pscmc->SC_Handle, SCARD_LEAVE_CARD);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_NO;
		return TRUE;
	}
	else
		return FALSE;
}

UINT CardEventThreadFunc(LPVOID pParam)
{
	LPPCSC_SCMC	pscmc = (LPPCSC_SCMC)pParam;
	// AfxMessageBox("Thread OK!", MB_OK| MB_ICONINFORMATION   , 0);

	while (pscmc->bIsRunning)
	{
		if (pscmc->iCardInsert == CARD_INSERTED) // a card in the reader
		{
			//Sleep (0);
			PCSC_GetCardStatus(pscmc);

			if (pscmc->dwCardStatus ==SCARD_ABSENT) // the card removed
			{
				pscmc->iCardInsert = CARD_REMOVED;
				::SendMessage(pscmc->hTargetWnd, WM_CARDREMOVED, 0, 0);
				//Sleep (500);

			}
		}
		else if (pscmc->iCardInsert == CARD_NOCARD) // no card in the reader
		{
			//Sleep (0);
			PCSC_GetCardStatus(pscmc);

			if ( (pscmc->dwCardStatus==SCARD_PRESENT) || (pscmc->dwCardStatus==SCARD_SPECIFIC) ) // the card inserted
			{
				pscmc->iCardInsert = CARD_INSERTED;	// user insert a card
				::SendMessage(pscmc->hTargetWnd, WM_CARDINSERTED, 0, 0);
				//Sleep (500);

			}
		}

		if (pscmc->iCardInsert == CARD_REMOVED) // user insert and remove the card
			pscmc->iCardInsert = CARD_NOCARD;	// wait for another card inserted ...

		Sleep (0);
	}
	return 0;
}

BOOL PCSC_READER_Transmit(LPPCSC_SCMC pscmc)
{
	pscmc->IO_Request.dwProtocol = pscmc->dwActProtocol;
	pscmc->IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);	
	DWORD nBytesReturned = 0; // OUT Parameter

	BYTE brSetASCII[6] = {26, 21, 0, 0, 1, 0}; // 1A1500000100
	BYTE brSetBitmap[6] = {26, 21, 0, 0, 1, 1};// 1A1500000101

	pscmc->ret = SCardControl(pscmc->SC_Handle, IOCTL_MAGNETICCCARD_COMMAND, (void *)pscmc->brSendBuf, 
		pscmc->dwSendLen, (void *)pscmc->brRecvBuf, MAX_RECEIVE, &nBytesReturned);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		if ( memcmp(pscmc->brSendBuf, brSetASCII, 6)==0 )
			pscmc->dwMCardMode = 0;

		if ( memcmp(pscmc->brSendBuf, brSetBitmap, 6)==0 )
			pscmc->dwMCardMode = 1;

		pscmc->dwRecvLen = nBytesReturned;
		return TRUE;
	}
	else
		return FALSE;
}


BOOL PCSC_READER_DetectCard(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1C31000000");	

	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen==3 && pscmc->brRecvBuf[1]==0x90 && pscmc->brRecvBuf[2]==0x00)
			pscmc->lMCardError = READER_S_SUCCESS;
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_READER_GetGrnLedState(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1C30000100");	

	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen==3 && pscmc->brRecvBuf[1]==0x90 && pscmc->brRecvBuf[2]==0x00)
			pscmc->lMCardError = READER_S_SUCCESS;
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_READER_GrnLedOn(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1B3000010100");	

	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen==2 && pscmc->brRecvBuf[0]==0x90 && pscmc->brRecvBuf[1]==0x00 )
			pscmc->lMCardError = READER_S_SUCCESS;
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		return TRUE;
	}
	else
		return FALSE;
}


BOOL PCSC_READER_GrnLedOff(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1B3000010101");	

	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen==2 && pscmc->brRecvBuf[0]==0x90 && pscmc->brRecvBuf[1]==0x00 )
			pscmc->lMCardError = READER_S_SUCCESS;
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		return TRUE;
	}
	else
		return FALSE;
}


// set response data's mode, ASCII (0) or Bitmap (1) mode
BOOL PCSC_READER_SetMCDataMode(DWORD dwMode, LPPCSC_SCMC pscmc)
{	
	TCHAR	szCmd[16];
	if (dwMode==1)
	{
		pscmc->dwMCardMode = 1;
		_tcscpy(szCmd, _T("1A1500000101"));
	}
	else
	{	
		pscmc->dwMCardMode = 0;
		_tcscpy(szCmd, _T("1A1500000100"));	
	}	

	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf	
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen==2 && pscmc->brRecvBuf[0]==0x90 && pscmc->brRecvBuf[1]==0x00 )
			pscmc->lMCardError = READER_S_SUCCESS;
		else
			pscmc->lMCardError = READER_E_UNKNOWN;
		return TRUE;
	}
	else	
		return FALSE;
}


BOOL PCSC_READER_GetT1Data(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1A14000100");
	
	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // szCmd to pscmc->brSendBuf
	pscmc->brSendBuf[5] = 0xFF;
	pscmc->dwSendLen++;
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen>2 )
		{
			if (pscmc->brRecvBuf[pscmc->dwRecvLen-2]==0x90 && pscmc->brRecvBuf[pscmc->dwRecvLen-1]==0x00 )
			{
				if (pscmc->dwMCardMode==0)	// ASCII mode
					ByteToStr( pscmc->brRecvBuf, pscmc->TrackData[0]);
				else						// Bitmap mode
					memcpy(pscmc->TrackBitmap[0], pscmc->brRecvBuf, pscmc->dwRecvLen); 
											// deduct 0x90, 0x00

				pscmc->lMCardError = READER_S_SUCCESS;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;
		}
		else if ( pscmc->dwRecvLen==2 )
		{
			if ( pscmc->brRecvBuf[0]==0x6F )
			{
				if ( pscmc->brRecvBuf[1]==0x20 )
					pscmc->lMCardError = READER_E_DATADECODE;
				else if ( pscmc->brRecvBuf[1]==0x50 )
					pscmc->lMCardError = READER_E_DATAEMPTY;
				else if ( pscmc->brRecvBuf[1]==0x70 )
					pscmc->lMCardError = READER_E_TRACKUNKNOWN;
				else
					pscmc->lMCardError = READER_E_UNKNOWN;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;
		}
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		if ( pscmc->lMCardError!=READER_S_SUCCESS)
		{
			_tcscpy( pscmc->TrackData[0], _T("{Data Empty or Error!}") );
			memset(pscmc->TrackBitmap[0], 0, MAX_TRACK_Bitmap);
		}

		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_READER_GetT2Data(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1A14000200");
	
	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // to pscmc->brSendBuf
	pscmc->brSendBuf[5] = 0xFF;
	pscmc->dwSendLen++;
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen>2 )
		{
			if (pscmc->brRecvBuf[pscmc->dwRecvLen-2]==0x90 && pscmc->brRecvBuf[pscmc->dwRecvLen-1]==0x00 )
			{
				if (pscmc->dwMCardMode==0)	// ASCII mode
					ByteToStr( pscmc->brRecvBuf, pscmc->TrackData[1]);
				else						// Bitmap mode
					memcpy(pscmc->TrackBitmap[1], pscmc->brRecvBuf, pscmc->dwRecvLen);
											// deduct 0x90, 0x00

				pscmc->lMCardError = READER_S_SUCCESS;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;
		}
		else if ( pscmc->dwRecvLen==2 )
		{
			if ( pscmc->brRecvBuf[0]==0x6F )
			{
				if ( pscmc->brRecvBuf[1]==0x20 )
					pscmc->lMCardError = READER_E_DATADECODE;
				else if ( pscmc->brRecvBuf[1]==0x50 )
					pscmc->lMCardError = READER_E_DATAEMPTY;
				else if ( pscmc->brRecvBuf[1]==0x70 )
					pscmc->lMCardError = READER_E_TRACKUNKNOWN;
				else
					pscmc->lMCardError = READER_E_UNKNOWN;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;			

		}
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		if ( pscmc->lMCardError!=READER_S_SUCCESS)
		{
			_tcscpy( pscmc->TrackData[1], _T("{Data Empty or Error!}") );
			memset(pscmc->TrackBitmap[1], 0, MAX_TRACK_Bitmap);
		}

		return TRUE;
	}
	else
		return FALSE;
}

BOOL PCSC_READER_GetT3Data(LPPCSC_SCMC pscmc)
{
	TCHAR	szCmd[] = _T("1A14000300");
	
	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	PCSC_AToSendBuf(szCmd, pscmc);  // to pscmc->brSendBuf
	pscmc->brSendBuf[5] = 0xFF;
	pscmc->dwSendLen++;
	
	if ( PCSC_READER_Transmit(pscmc) )
	{
		if ( pscmc->dwRecvLen>2 )
		{
			if (pscmc->brRecvBuf[pscmc->dwRecvLen-2]==0x90 && pscmc->brRecvBuf[pscmc->dwRecvLen-1]==0x00 )
			{
				if (pscmc->dwMCardMode==0)	// ASCII mode
					ByteToStr( pscmc->brRecvBuf, pscmc->TrackData[2]);
				else						// Bitmap mode
					memcpy(pscmc->TrackBitmap[2], pscmc->brRecvBuf, pscmc->dwRecvLen);
											// deduct 0x90, 0x00

				pscmc->lMCardError = READER_S_SUCCESS;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;
		}
		else if ( pscmc->dwRecvLen==2 )
		{
			if ( pscmc->brRecvBuf[0]==0x6F )
			{
				if ( pscmc->brRecvBuf[1]==0x20 )
					pscmc->lMCardError = READER_E_DATADECODE;
				else if ( pscmc->brRecvBuf[1]==0x50 )
					pscmc->lMCardError = READER_E_DATAEMPTY;
				else if ( pscmc->brRecvBuf[1]==0x70 )
					pscmc->lMCardError = READER_E_TRACKUNKNOWN;
				else
					pscmc->lMCardError = READER_E_UNKNOWN;
			}
			else
				pscmc->lMCardError = READER_E_UNKNOWN;			

		}
		else
			pscmc->lMCardError = READER_E_UNKNOWN;

		if ( pscmc->lMCardError!=READER_S_SUCCESS)
		{
			_tcscpy( pscmc->TrackData[2], _T("{Data Empty or Error!}") );
			memset(pscmc->TrackBitmap[2], 0, MAX_TRACK_Bitmap);
		}

		return TRUE;
	}
	else
		return FALSE;
}
*/
BOOL PCSC_RFID_Connect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &pscmc->RFID_Context);
	pscmc->ret = SCardConnect(pscmc->RFID_Context, pscmc->mszReaderName[pscmc->dwActReader], SCARD_SHARE_SHARED, 
				SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &pscmc->RFID_Handle, &pscmc->dwActProtocol);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_RFID;

		TCHAR	szReader[128];
		DWORD	dwReaderLen = 128;
		memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);
		pscmc->dwRecvLen = 32; // cannot be 0, it is a in-out parameter not just in parameter	

		pscmc->ret = SCardStatus(pscmc->RFID_Handle, szReader, &dwReaderLen, &pscmc->dwCardStatus, &pscmc->dwActProtocol, 
						pscmc->brRecvBuf, &pscmc->dwRecvLen);	
		if (pscmc->ret == SCARD_S_SUCCESS)
		{
			memset(pscmc->szMsg, 0, sizeof(pscmc->szMsg));

			switch (pscmc->dwCardStatus) 
			{
				case SCARD_UNKNOWN:
					_tcscpy( pscmc->szMsg, _T("the driver is unaware of the current state of the reader.") );
					pscmc->szMsg; break;

				case SCARD_ABSENT:
					_tcscpy( pscmc->szMsg, _T("There is no card in the reader.") );
					pscmc->szMsg; break;

				case SCARD_PRESENT:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader, but it cannot be reset or communicated with.") );
					pscmc->szMsg; break;

				case SCARD_SWALLOWED:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader in position for use. The card is not powered.") );
					pscmc->szMsg; break;

				case SCARD_POWERED:
					_tcscpy( pscmc->szMsg, _T("Power is being provided to the card, but the reader driver is unware of the mode of the card.") );
					pscmc->szMsg; break;

				case SCARD_NEGOTIABLE:
					_tcscpy( pscmc->szMsg, _T("The card has been reset and is awaiting PTS negotiation.") );
					pscmc->szMsg; break;

				case SCARD_SPECIFIC:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader. It has been reset and specific communication protocols have been established.") );
					pscmc->szMsg; break;
				
				default:
					_stprintf(pscmc->szMsg, _T("Unknown dwState value: 0x%X "), pscmc->dwCardStatus);
					pscmc->szMsg; break;
			}

			return TRUE;
		}
		else	
			return FALSE;
	//	return TRUE;
	}
	else
		return FALSE;
}
BOOL PCSC_RFID_Disconnect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardDisconnect(pscmc->RFID_Handle, SCARD_UNPOWER_CARD);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_NO;
		return TRUE;
	}
	else
		return FALSE;
}
BOOL PCSC_RFID_Transmit(LPPCSC_SCMC pscmc)
{
	//Escape
/*	pscmc->IO_Request.dwProtocol = pscmc->dwActProtocol;
	pscmc->IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);	
	DWORD nBytesReturned = 0; // OUT Parameter
	
	pscmc->ret = SCardControl(pscmc->RFID_Handle, IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE, (void *)pscmc->brSendBuf, 
		pscmc->dwSendLen, (void *)pscmc->brRecvBuf, MAX_RECEIVE, &nBytesReturned);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->dwRecvLen = nBytesReturned;
		return TRUE;
	}
	else
		return FALSE;*/
	//APDU(XfrBlock)
	pscmc->IO_Request.dwProtocol = pscmc->dwActProtocol;
	pscmc->IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);	
	pscmc->dwRecvLen = 260; // IN OUT Parameter, you need to tell the buff size

	pscmc->ret = SCardTransmit(pscmc->RFID_Handle, &pscmc->IO_Request, pscmc->brSendBuf, 
			pscmc->dwSendLen, 0, pscmc->brRecvBuf, &pscmc->dwRecvLen);
	if (pscmc->ret==SCARD_S_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

BOOL PCSC_MagCard_Connect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &pscmc->MAGCard_Context);
	pscmc->ret = SCardConnect(pscmc->MAGCard_Context, pscmc->mszReaderName[pscmc->dwActReader], SCARD_SHARE_SHARED, 
				SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &pscmc->MAGCard_Handle, &pscmc->dwActProtocol);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_MAGCARD;

		TCHAR	szReader[128];
		DWORD	dwReaderLen = 128;
		memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);
		pscmc->dwRecvLen = 32; // cannot be 0, it is a in-out parameter not just in parameter	

		pscmc->ret = SCardStatus(pscmc->MAGCard_Handle, szReader, &dwReaderLen, &pscmc->dwCardStatus, &pscmc->dwActProtocol, 
						pscmc->brRecvBuf, &pscmc->dwRecvLen);
		if (pscmc->ret == SCARD_S_SUCCESS)
		{
			memset(pscmc->szMsg, 0, sizeof(pscmc->szMsg));

			switch (pscmc->dwCardStatus) 
			{
				case SCARD_UNKNOWN:
					_tcscpy( pscmc->szMsg, _T("the driver is unaware of the current state of the reader.") );
					pscmc->szMsg; break;

				case SCARD_ABSENT:
					_tcscpy( pscmc->szMsg, _T("There is no card in the reader.") );
					pscmc->szMsg; break;

				case SCARD_PRESENT:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader, but it cannot be reset or communicated with.") );
					pscmc->szMsg; break;

				case SCARD_SWALLOWED:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader in position for use. The card is not powered.") );
					pscmc->szMsg; break;

				case SCARD_POWERED:
					_tcscpy( pscmc->szMsg, _T("Power is being provided to the card, but the reader driver is unware of the mode of the card.") );
					pscmc->szMsg; break;

				case SCARD_NEGOTIABLE:
					_tcscpy( pscmc->szMsg, _T("The card has been reset and is awaiting PTS negotiation.") );
					pscmc->szMsg; break;

				case SCARD_SPECIFIC:
					_tcscpy( pscmc->szMsg, _T("There is a card in the reader. It has been reset and specific communication protocols have been established.") );
					pscmc->szMsg; break;
				
				default:
					_stprintf(pscmc->szMsg, _T("Unknown dwState value: 0x%X "), pscmc->dwCardStatus);
					pscmc->szMsg; break;
			}

			return TRUE;
		}
		else	
			return FALSE;
	//	return TRUE;
	}
	else
		return FALSE;
}
BOOL PCSC_MagCard_Disconnect(LPPCSC_SCMC pscmc)
{
	pscmc->ret = SCardDisconnect(pscmc->MAGCard_Handle, SCARD_LEAVE_CARD);
	if (pscmc->ret==SCARD_S_SUCCESS)
	{
		pscmc->iConnectionStatus = CONNECTION_NO;
		return TRUE;
	}
	else
		return FALSE;
}
BOOL PCSC_MagCard_GetTrackData(LPPCSC_SCMC pscmc)
{
	pscmc->IO_Request.dwProtocol = pscmc->dwActProtocol;
	pscmc->IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);	
	pscmc->dwRecvLen = 260; // IN OUT Parameter, you need to tell the buff size
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);
	pscmc->ret = SCardTransmit(pscmc->MAGCard_Handle, &pscmc->IO_Request, pscmc->brSendBuf, 
			pscmc->dwSendLen, 0, pscmc->brRecvBuf, &pscmc->dwRecvLen);

	if (pscmc->ret==SCARD_S_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

