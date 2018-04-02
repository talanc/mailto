#pragma once

#include "stdafx.h"

const wchar_t *mapi_error_message_unknown = L"unknown error code";

const wchar_t *mapi_error_messages[] = {
	/*  0 */ L"The call succeeded and the message was sent.",
	/*  1 */ L"The user canceled one of the dialog boxes. No message was sent.",
	/*  2 */ L"One or more unspecified errors occurred. No message was sent.",
	/*  3 */ L"There was no default logon, and the user failed to log on successfully when the logon dialog box was displayed. No message was sent.",
	/*  4 */ L"",
	/*  5 */ L"There was insufficient memory to proceed. No message was sent.",
	/*  6 */ L"",
	/*  7 */ L"",
	/*  8 */ L"",
	/*  9 */ L"There were too many file attachments. No message was sent.",
	/* 10 */ L"There were too many recipients. No message was sent.",
	/* 11 */ L"The specified attachment was not found. No message was sent.",
	/* 12 */ L"The specified attachment could not be opened. No message was sent.",
	/* 13 */ L"",
	/* 14 */ L"A recipient did not appear in the address list. No message was sent.",
	/* 15 */ L"The type of a recipient was not MAPI_TO, MAPI_CC, or MAPI_BCC. No message was sent.",
	/* 16 */ L"",
	/* 17 */ L"",
	/* 18 */ L"The text in the message was too large. No message was sent.",
	/* 19 */ L"",
	/* 20 */ L"",
	/* 21 */ L"A recipient matched more than one of the recipient descriptor structures and MAPI_DIALOG was not set. No message was sent.",
	/* 22 */ L"",
	/* 23 */ L"",
	/* 24 */ L"",
	/* 25 */ L"One or more recipients were invalid or did not resolve to any address.",
	/* 26 */ L"",
	/* 27 */ L"The MAPI_FORCE_UNICODE flag is specified and Unicode is not supported.",
};

const wchar_t* get_mapi_error_message(ULONG ulErrorCode)
{
	const wchar_t * errorMessage;

	if (ulErrorCode >= SUCCESS_SUCCESS && ulErrorCode <= MAPI_E_UNICODE_NOT_SUPPORTED) {
		errorMessage = mapi_error_messages[ulErrorCode];
	}
	else {
		errorMessage = mapi_error_message_unknown;
	}

	return errorMessage;
}
