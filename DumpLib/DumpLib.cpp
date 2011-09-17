#include "stdafx.h"
#include "windows.h"
#include "vcclr.h"
#include "Dbghelp.h"
#include "DumpLib.h"

#pragma comment(lib, "Dbghelp.lib")

using namespace System::Diagnostics;
using namespace System::IO;
using namespace DumpLib;

static DumpErrorCode CreateFile(System::String ^path, HANDLE *hFile)
{
	if (path == nullptr)
	{
		Debug::WriteLine("null path");
		return DumpErrorCode::DirectoryNotFound;
	}
	else
	{
		System::String ^dir = Path::GetDirectoryName(path);
		if (!System::String::IsNullOrEmpty(dir) && !Directory::Exists(dir))
		{
			Debug::WriteLine("not found: " + dir);
			return DumpErrorCode::DirectoryNotFound;
		}
	}
	pin_ptr<const wchar_t> wptr = PtrToStringChars(path);
	*hFile = CreateFile(wptr, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (*hFile == INVALID_HANDLE_VALUE)
	{
		DWORD lastError = ::GetLastError();
		Debug::WriteLine("CreateFile failed " + lastError);
		switch (lastError)
		{
		case ERROR_FILE_EXISTS:
			return DumpErrorCode::FileAlreadyExists;
		case ERROR_ACCESS_DENIED:
			return DumpErrorCode::FileAccessError;
		default:
			return DumpErrorCode::GenericFileError;
		}
	}
	return DumpErrorCode::NoError;
}

DumpErrorCode Dumper::DumpProcess(int pid, System::String ^filename)
{
	DumpErrorCode err;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hProcess = NULL;

	DWORD desiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
	hProcess = OpenProcess(desiredAccess, FALSE, pid);
	if (hProcess == NULL)
	{
		DWORD lastError = ::GetLastError();
		Debug::WriteLine("OpenProcess failed " + lastError);
		switch (lastError)
		{
		case ERROR_ACCESS_DENIED:
			return DumpErrorCode::ProcessAccessError;
		default:
			return DumpErrorCode::GenericProcessError;
		}
	}

	err = CreateFile(filename, &hFile);
	if (err != DumpErrorCode::NoError)
	{
		CloseHandle(hProcess);
		return err;
	}

	// MiniDumpWriteDump function
	// http://msdn.microsoft.com/en-us/library/ms680360.aspx
	// MINIDUMP_TYPE
	// http://msdn.microsoft.com/en-us/library/ms680519.aspx
	int dumpType = MiniDumpWithFullMemory
		| MiniDumpWithHandleData
		| MiniDumpWithUnloadedModules
		| MiniDumpWithProcessThreadData
		| MiniDumpWithFullMemoryInfo
		| MiniDumpWithThreadInfo
		| MiniDumpWithFullAuxiliaryState
		| MiniDumpIgnoreInaccessibleMemory
		| MiniDumpWithTokenInformation;
	if (!MiniDumpWriteDump(hProcess, pid, hFile, static_cast<MINIDUMP_TYPE>(dumpType), NULL, NULL, NULL))
	{
		DWORD lastError = ::GetLastError();
		Debug::WriteLine("MiniDumpWriteDump failed " + lastError);
		CloseHandle(hFile);
		try
		{
			File::Delete(filename);
		}
		catch (Exception ^e)
		{
			Debug::WriteLine("remove " + filename + " failed: " + e->ToString());
		}
		CloseHandle(hProcess);
		switch (lastError)
		{
		case ERROR_ACCESS_DENIED:
			return DumpErrorCode::DumpAccessError;
		default:
			return DumpErrorCode::GenericDumpError;
		}
	}

	Debug::WriteLine("dump created: " + filename);
	CloseHandle(hFile);
	CloseHandle(hProcess);
	return DumpErrorCode::NoError;
}
