#pragma once

using namespace System;

namespace DumpLib {
	public enum class DumpErrorCode
	{
		NoError = 0,
		GenericProcessError = 100,
		ProcessAccessError,
		GenericFileError = 200,
		DirectoryNotFound,
		FileAlreadyExists,
		FileAccessError,
		GenericDumpError = 300,
		DumpAccessError,
	};

	public ref class Dumper
	{
	public:
		static DumpErrorCode DumpProcess(int pid, System::String ^filename);
	};
}
