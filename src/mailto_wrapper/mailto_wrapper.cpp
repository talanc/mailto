// mailto_wrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mailto_helper.h"

const wchar_t *usage = L"usage: mailto.exe [--body TEXT] [--subject TEXT] [--to ADDR...] [--cc ADDR...] [--bcc ADDR...] [--attachment PATH...] \n\
\n\
    --body: email body, may also come from stdin.\n\
    --subject: email subject.\n\
    --to: primary recipient email address, may specify multiple addresses.\n\
    --cc: CC addresses.\n\
    --bcc: BCC addresses.\n\
    --attachment: files to attach.";

const int MAX_RECIP = 64;
const int MAX_FILES = 64;

void SetRecip(MapiRecipDescW *recip, ULONG cls, wchar_t *arg)
{
	size_t argLen = wcslen(arg);
	size_t addressLen = argLen + 1 + 5; // +1: \0, +5: SMTP: prefix
	wchar_t *address = new wchar_t[addressLen];
	wcsncpy_s(address, addressLen, L"SMTP:", 5);
	wcsncat_s(address, addressLen, arg, argLen);
	
	ZeroMemory(recip, sizeof(*recip));
	recip->ulRecipClass = cls;
	recip->lpszName = arg;
	recip->lpszAddress = address;
}

void FreeRecip(MapiRecipDescW *recip)
{
	delete[] recip->lpszAddress;
	ZeroMemory(recip, sizeof(*recip));
}

int AddFiles(ULONG *nFileCount, MapiFileDescW *files, wchar_t *arg)
{
	int nAdded = 0;

	HANDLE find;
	WIN32_FIND_DATA data;

	DWORD fullPathMax = 1024;
	wchar_t *fullPath = new wchar_t[fullPathMax];
	DWORD fullPathLen;

	MapiFileDescW *file;

	find = FindFirstFile(arg, &data);

	if (find == INVALID_HANDLE_VALUE) {
		ewprintf_s(L"error adding attachment '%ls' (%d)", arg, GetLastError());
		nAdded = -1;
		goto cleanup;
	}

	do {
		// dont add directory
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// Get full path;
		fullPathLen = GetFullPathName(data.cFileName, fullPathMax, fullPath, NULL);

		// increase buffer capacity
		if (fullPathLen >= fullPathMax) {
			wprintf_s(L"increase buffer capacity, from %d to %d", fullPathMax, fullPathLen + 1);
			fullPathMax = fullPathLen + 1;
			delete[] fullPath;
			fullPathLen = GetFullPathName(data.cFileName, fullPathMax, fullPath, NULL);
		}

		if (fullPathLen == 0) {
			ewprintf_s(L"error adding attachment '%ls', could not get full path (%d)", data.cFileName, GetLastError());
			nAdded = -1;
			goto cleanup;
		}

		// make a copy of the full path (free in FreeFile function)
		wchar_t * filePath = new wchar_t[fullPathLen + 1];
		wcsncpy_s(filePath, fullPathLen + 1, fullPath, fullPathLen + 1);

		// add file with full path
		file = &files[*nFileCount];
		ZeroMemory(file, sizeof(*file));
		file->nPosition = -1;
		file->lpszPathName = filePath;
		(*nFileCount)++;
		nAdded++;
	} while (FindNextFile(find, &data) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		ewprintf_s(L"error adding attachment '%ls' (%d)", arg, GetLastError());
		nAdded = -1;
		goto cleanup;
	}

	if (nAdded == 0) {
		ewprintf_s(L"could not add attachment: %ls (does it exist?)", arg);
	}

cleanup:
	if (find != INVALID_HANDLE_VALUE) {
		FindClose(find); // close find handle
	}
	delete[] fullPath; // delete full path buffer

	return nAdded;
}

void FreeFile(MapiFileDescW *file)
{
	delete[] file->lpszPathName;
	ZeroMemory(file, sizeof(*file));
}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	ULONG recipClass = -1;
	int attach = FALSE;

	ULONG nRecipCount = 0;
	MapiRecipDescW recips[MAX_RECIP];

	ULONG nFileCount = 0;
	MapiFileDescW files[MAX_FILES];

	MapiMessageW msg;
	ZeroMemory(&msg, sizeof(msg));

	int exitCode = -1;
	ULONG msgResult = 0;

	// print usage
	if (argc <= 1) {
		wprintf_s(L"%ls\n", usage);
		goto cleanup;
	}

	// parse args
	for (int i = 1; i < argc; i++) {
		wchar_t *arg = argv[i];
		int is_last = (i == argc - 1);

		if (_wcsicmp(L"--body", arg) == 0) {
			recipClass = -1;
			attach = FALSE;

			if (is_last) {
				ewprintf_s(L"expecting value for --body\n");
				goto cleanup;
			}
			if (msg.lpszNoteText) {
				ewprintf_s(L"body has already been set\n");
				goto cleanup;
			}

			i++;
			msg.lpszNoteText = argv[i];
			continue;
		}

		if (_wcsicmp(L"--subject", arg) == 0) {
			recipClass = -1;
			attach = FALSE;

			if (is_last) {
				ewprintf_s(L"expecting value for --subject\n");
				goto cleanup;
			}
			if (msg.lpszSubject) {
				ewprintf_s(L"subject has already been set\n");
				goto cleanup;
			}

			i++;
			msg.lpszSubject = argv[i];
			continue;
		}

		if (_wcsicmp(L"--to", arg) == 0) {
			recipClass = MAPI_TO;
			attach = FALSE;
			continue;
		}

		if (_wcsicmp(L"--cc", arg) == 0) {
			recipClass = MAPI_CC;
			attach = FALSE;
			continue;
		}

		if (_wcsicmp(L"--bcc", arg) == 0) {
			recipClass = MAPI_BCC;
			attach = FALSE;
			continue;
		}

		if (_wcsicmp(L"--attachment", arg) == 0) {
			recipClass = -1;
			attach = TRUE;
			continue;
		}

		if (recipClass != -1) {
			if (nRecipCount >= MAX_RECIP) {
				ewprintf_s(L"maximum number of recipients reached\n");
				goto cleanup;
			}

			SetRecip(&recips[nRecipCount], recipClass, arg);
			nRecipCount++;
			continue;
		}

		if (attach) {
			if (nFileCount >= MAX_FILES) {
				ewprintf_s(L"maximum number of attachments reached\n");
				goto cleanup;
			}

			int nAdded = AddFiles(&nFileCount, files, arg);
			if (nAdded <= 0) {
				goto cleanup;
			}

			continue;
		}

		ewprintf_s(L"invalid argument %ls\n", arg);
		goto cleanup;
	}

	msg.nRecipCount = nRecipCount;
	msg.lpRecips = recips;
	msg.nFileCount = nFileCount;
	msg.lpFiles = files;

	// try send message
	msgResult = MAPISendMailHelper(0, 0, &msg, MAPI_DIALOG, 0);
	exitCode = (int)msgResult;

	if (msgResult == SUCCESS_SUCCESS || msgResult == MAPI_USER_ABORT) {
		goto cleanup;
	}

	// send mail failed
	ewprintf_s(L"%ls\n", get_mapi_error_message(msgResult));

cleanup:

	// cleanup
	for (ULONG i = 0; i < nRecipCount; i++) {
		FreeRecip(&recips[i]);
	}
	for (ULONG i = 0; i < nFileCount; i++) {
		FreeFile(&files[i]);
	}

	return exitCode;
}
