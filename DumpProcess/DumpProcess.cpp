#include "stdafx.h"

using namespace System;
using namespace System::Diagnostics;
using namespace DumpLib;

static void PrintUsage()
{
	Console::WriteLine("usage:");
	Console::WriteLine("\tDumpProcess /p pid [filename]");
	Console::WriteLine("\tDumpProcess /n name [outdir]");
}

static String ^GenerateDumpFileName(Process ^proc, String ^dir)
{
	String ^filename;
	for (UInt32 i = 0; i < UInt32::MaxValue; i++)
	{
		filename = IO::Path::Combine(dir, proc->ProcessName + "." + proc->Id + "." + i + ".dmp");
		if (!IO::File::Exists(filename))
		{
			break;
		}
	}
	return filename;
}

static int Dump(int pid, String ^filename)
{
	DumpErrorCode err = Dumper::DumpProcess(pid, filename);
	if (err == DumpErrorCode::NoError)
	{
		Console::WriteLine("created " + filename);
		return 0;
	}
	else
	{
		Console::WriteLine("failed for pid " + pid + " " + err.ToString());
		return -1;
	}
}

int main(array<String ^> ^args)
{
	if (args->Length != 2 && args->Length != 3)
	{
		PrintUsage();
		return -1;
	}

	if (args[0] == "/p")
	{
		int pid;
		try
		{
			pid = int::Parse(args[1]);
		}
		catch (Exception ^e)
		{
			Console::WriteLine("invalid argument " + args[1]);
			Console::WriteLine(e->ToString());
			return -1;
		}
		Process ^proc;
		try
		{
			proc = Process::GetProcessById(pid);
		}
		catch (Exception ^)
		{
			Console::WriteLine("no process with pid " + pid);
			return -1;
		}

		String ^filename;
		if (args->Length > 2)
		{
			filename = args[2];
		}
		else
		{
			String ^dir = IO::Directory::GetCurrentDirectory();
			filename = GenerateDumpFileName(proc, dir);
		}
		return Dump(pid, filename);
	}
	else if (args[0] == "/n")
	{
		array<Process ^> ^procs = Process::GetProcessesByName(args[1]);
		if (procs->Length == 0)
		{
			Console::WriteLine("no process with name " + args[1]);
			return -1;
		}
		String ^dir;
		if (args->Length > 2)
		{
			dir = args[2];
			if (!IO::Directory::Exists(dir))
			{
				Console::WriteLine("no outdir " + dir);
				return -1;
			}
		}
		else
		{
			dir = IO::Directory::GetCurrentDirectory();
		}

		int result = 0;
		for each (Process ^proc in procs)
		{
			String ^filename = GenerateDumpFileName(proc, dir);
			result |= Dump(proc->Id, filename);
		}
		return result;
	}
	else
	{
		PrintUsage();
		return -1;
	}
}
