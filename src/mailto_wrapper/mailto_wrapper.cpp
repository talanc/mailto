// mailto_wrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mailto_helper.h"

const wchar_t *usage = L"usage: mailto.exe [--body TEXT] [--subject TEXT] [--to ADDR...] [--cc ADDR...] [--bcc ADDR...] [--attachment PATH...] \r\n\
\r\n\
    --body: email body, may also come from stdin.\r\n\
    --subject: email subject.\r\n\
    --to: primary recipient email address, may specify multiple addresses.\r\n\
    --cc: CC addresses.\r\n\
    --bcc: BCC addresses.\r\n\
    --attachment: files to attach.";

enum flagmode {
	NONE,
	ADDR,
	ATTA
};

const int MAX_RECIP = 64;
const int MAX_FILES = 64;

void SetRecip(MapiRecipDescW *recip, ULONG cls, wchar_t *arg)
{
	ZeroMemory(recip, sizeof(*recip));

	recip->ulRecipClass = cls;
	recip->lpszAddress = arg;
}

void FreeRecip(MapiRecipDescW *recip)
{
	
}

void SetFile(MapiFileDescW *file, wchar_t *arg)
{
	ZeroMemory(file, sizeof(*file));

	// ...
}

void FreeFile(MapiFileDescW *file)
{

}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	flagmode flag = NONE;
	ULONG recipClass = -1;
	int attach = FALSE;

	ULONG nRecipCount = 0;
	MapiRecipDescW recips[MAX_RECIP];

	ULONG nFileCount = 0;
	MapiFileDescW files[MAX_FILES];

	MapiMessageW msg;
	ZeroMemory(&msg, sizeof(msg));

	int exitCode = 1;
	ULONG msgResult = 0;
	const wchar_t *mapiErrorMessage = NULL;

	// print usage
	if (argc <= 1) {
		wprintf_s(L"%ls\n", usage);
		goto finish;
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

			SetFile(&files[nFileCount], arg);
			nFileCount++;
			continue;
		}

		ewprintf_s(L"invalid argument %ls\n", arg);
		goto cleanup;
	}

	/*msg.nRecipCount = nRecipCount;
	msg.lpRecips = recips;
	msg.nFileCount = nFileCount;
	msg.lpFiles = files;*/

	// try send message
	msgResult = MAPISendMailHelper(0, 0, &msg, MAPI_DIALOG, 0);

	if (msgResult == SUCCESS_SUCCESS || msgResult == MAPI_USER_ABORT) {
		exitCode = 0;
		goto cleanup;
	}

	// send mail failed
	exitCode = 2;
	mapiErrorMessage = get_mapi_error_message(msgResult);
	ewprintf_s(L"%lu %ls\n", msgResult, mapiErrorMessage);

cleanup:

	// cleanup
	for (ULONG i = 0; i < nRecipCount; i++) {
		FreeRecip(&recips[i]);
	}
	for (ULONG i = 0; i < nFileCount; i++) {
		FreeFile(&files[i]);
	}

finish:

	wprintf_s(L"argc = %d\n", argc);

	wprintf_s(usage);
	wprintf_s(L"\r\n");

	getc(stdin);

	return exitCode;
}
