#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>

// 9185
uint32_t CELT_SIG_SCALE_offset_9185 = 0xEA0858; // offset to the global (should always be 47000000 or 32768.0f)
uint32_t opus_encoder_create_CELT_mode_offset_9185 = 0x8A9F94; // should always be E9 03

/* THIS ONLY WORKS FOR DISCORD VERSION 9186, ANY COMPETENT REVERSE ENGINEER CAN FIND THESE OFFSETS FOR ANY VERSION OF DISCORD */
// version 9186
uint32_t CreateAudioFrameStereoInstruction = 0xAD794; // patch to the bytes 4D 89 C5 90
uint32_t RegulatesStereoPropertyInstruction = 0x497AA6; // patch to EB
uint32_t AudioEncoderOpusConfigSetChannelsInstruction = 0x302EA8; // patch to 02
uint32_t MonoDownmixerInstructions = 0x095BFB; // patch to 90 90 90 90 90 90 90 90 90 E9
uint32_t CELT_SIG_SCALE_offset_9186 = 0xEA58A8; // patch to FF FF FF 47
uint32_t opus_encoder_create_CELT_mode_offset_9186 = 0x8B0A44; // should always be E9 03

// copied these 2 good old routines from backtracer src lol
void ExternalWrite(HANDLE Process, void* Address, void* source, uint32_t size)
{
	if (!WriteProcessMemory(Process, Address, source, size, NULL))
	{
		std::cout << "Write failed!" << std::endl;
	}
}

void ExternalWrite(HANDLE Process, void* Address, uint8_t byte)
{
	if (!WriteProcessMemory(Process, Address, &byte, 1, NULL))
	{
		std::cout << "Write failed!" << std::endl;
	}
}


// known issue: FEC enabled (can be disabled in opus not doing allat)
// override those 2 offsets and you are uncapped without using a hook, bitrate is maxed default on celt
int main()
{
	HANDLE Discord = {};
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!Snapshot || Snapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create snapshot" << std::endl;
		return 0;
	}

	// for beginners this is for finding the process
	PROCESSENTRY32 ProcessEntry = {};
	ProcessEntry.dwSize = sizeof(ProcessEntry);
	while (Process32Next(Snapshot, &ProcessEntry))
	{
		if (!strcmp(ProcessEntry.szExeFile, "Discord.exe"))
		{
			HANDLE Process = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessEntry.th32ProcessID);
			if (!Process || Process == INVALID_HANDLE_VALUE)
			{
				// indicative of once again some retard spoofing dont spoof before running this
				std::cout << "Failed to obtain Discord.exe process access, disable any Permission Blockers or run this as admin" << std::endl;
				continue;
			}

			DWORD BytesUsed = 0;
			HMODULE Modules[1024] = {};
			if (!EnumProcessModules(Process, Modules, sizeof(Modules), &BytesUsed))
			{
				// indicates ObProcessCallbacks as the highest probability of causing this problem or an kernel/user hook is in place unintentionally
				std::cout << "Failed to obtain query access to the target process" << std::endl;
				continue;
			}

			for (uint32_t i = 0; i < BytesUsed / sizeof(Modules[0]); i++)
			{
				/*
				MODULEINFO ModuleInfo = {};
				if (!GetModuleInformation(Process, Modules[i], &ModuleInfo, sizeof(ModuleInfo)))
				{
					std::cout << "Could not get information on this specified module" << std::endl;
					continue;
				}
				*/
				char ModuleName[MAX_PATH] = {};
				if (!GetModuleBaseNameA(Process, Modules[i], ModuleName, sizeof(ModuleName)))
				{
					std::cout << "Could not get the module name" << std::endl;
					continue;
				}

				// obviously were not sub string checking for "discord_voice" because this does not support any other version besides 9186
				if (!strcmp(ModuleName, "discord_voice.node"))
				{
					Discord = Process;
					goto exit_from_loop;
				}
			}
		}
	}

	std::cout << "Could not find any running discord processes" << std::endl;
	system("pause");
	return 0;

exit_from_loop:
	// start patches
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + RegulatesStereoPropertyInstruction), 0xEB);
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + CreateAudioFrameStereoInstruction), "\x4D\x89\xC5\x90", sizeof("\x4D\x89\xC5\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + AudioEncoderOpusConfigSetChannelsInstruction), 2);
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + MonoDownmixerInstructions), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9", sizeof("\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + CELT_SIG_SCALE_offset_9186), "\xFF\xFF\xFF\x47", sizeof("\xFF\xFF\xFF\x47") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)Discord + opus_encoder_create_CELT_mode_offset_9186), "\xE9\x03", sizeof("\xE9\x03") - 1);
}
