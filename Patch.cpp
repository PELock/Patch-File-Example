////////////////////////////////////////////////////////////////////////////////
//
// Patch File Example in C++ for Windows (Complex)
//
// Language : C/C++
// Author   : Bartosz Wójcik
// Website  : http://www.pelock.com
//
////////////////////////////////////////////////////////////////////////////////

#define UNICODE

#include <windows.h>
#include <Sfc.h>

// include SFC.lib (used by the Windows file protection mechanism WFP / WRP)
#pragma comment(lib, "sfc")

int main()
{
	// full path to the file you want to patch
	const wchar_t * wszFilePath = L"C:\\file.exe";

	// patch bytes (those will be written to the selected file)
	const BYTE cPatch[] = { 0xAA, 0xBB, 0xCC };

	// patch raw file offset
	const DWORD dwPatchRawOffset = 0x1234;

	// file size
	DWORD dwFileSize = 0;

	// number of written patch bytes (param for WriteFile function)
	DWORD dwWritten = 0;

	// WriteFile error code
	BOOL bWriteFileResult = FALSE;

	// check the file attributes, especially read only attributes for removable media like DVD, CD etc.
	DWORD dwFileAttributes = GetFileAttributes(wszFilePath);

	// can we read file attributes?
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(NULL, L"Cannot read the file attributes (does the file exists?)!", L"Error", MB_ICONERROR);
		return 1;
	}

	// is it read only file?
	if ( (dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
	{
		MessageBox(NULL, L"Input file is read only (it might be stored on a read only media like DVD drive)!", L"Error", MB_ICONERROR);
		return 2;
	}

	// is it protected by the Windows file protection system?
	if (SfcIsFileProtected(NULL, wszFilePath) == TRUE)
	{
		MessageBox(NULL, L"Input file is protected by the Windows file protection system and it will be automatically restored to the original even after patching.", L"Error", MB_ICONERROR);
		return 3;
	}

	// open input file (use GENERIC_WRITE flag)
	HANDLE hFile = CreateFile(wszFilePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// verify file handle
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"Couldn't open the input file!", L"Error", MB_ICONERROR);
		return 4;
	}

	// get file size
	dwFileSize = GetFileSize(hFile, NULL);

	// is it empty file?
	if (dwFileSize == 0)
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"Input file is empty (0 bytes)!", L"Error", MB_ICONERROR);
		return 5;
	}

	// does the patch offset is within the file size?
	if ( (dwPatchRawOffset + sizeof(cPatch)) > dwFileSize)
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"Patch offset is invalid (it's beyond the file size boundaries)!", L"Error", MB_ICONERROR);
		return 6;
	}

	// set proper patch offset (raw offset) where the patch bytes will be written
	if (SetFilePointer(hFile, dwPatchRawOffset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"Couldn't set file pointer to the patch raw address!", L"Error", MB_ICONERROR);
		return 7;
	}

	// write patch bytes from cPatch array at the appropriate file offset
	bWriteFileResult = WriteFile(hFile, cPatch, sizeof(cPatch), &dwWritten, NULL);

	// did we succeed?
	if (bWriteFileResult == FALSE)
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"An error occured while writing patch bytes to the file!", L"Error", MB_ICONERROR);
		return 8;
	}

	// did all the patch bytes were written correctly?
	if (dwWritten != sizeof(cPatch))
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"Couldn't write all of the patch bytes to the file!", L"Error", MB_ICONERROR);
		return 9;
	}

	// flush the results from the file cache to the media (use it for USB drives etc.)
	if (FlushFileBuffers(hFile) == FALSE)
	{
		CloseHandle(hFile);

		MessageBox(NULL, L"An error occured while flushing the results to the disk!", L"Error", MB_ICONERROR);
		return 10;
	}

	// close the file handle
	CloseHandle(hFile);

	MessageBox(NULL, L"File sucessfully patched", L"Information", MB_ICONINFORMATION);

	return 0;
}
