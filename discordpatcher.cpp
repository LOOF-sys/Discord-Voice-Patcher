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
uint32_t MonoDownmixerInstructions = 0x95B23; // patch to 90 90 90 90 90 90 90 90 90 E9
uint32_t CELT_SIG_SCALE_offset_9186 = 0xEA58A8; // patch to FF FF FF 47
uint32_t opus_encoder_create_CELT_mode_offset_9186 = 0x8B0A44; // should always be E9 03
uint32_t HighPassFilter_Process = 0x4A5022; // patch to 48 B8 10 9E D8 CF 08 02 00 00 C3

void ExternalWrite(HANDLE Process, void* Address, const char* source, uint32_t size)
{
	DWORD Old = 0;
	DWORD Junk = 0;
	VirtualProtectEx(Process, Address, 0x1000, PAGE_EXECUTE_READWRITE, &Old);
	if (!WriteProcessMemory(Process, Address, source, size, NULL))
	{
		std::cout << "Write failed! length: " << size << ", address: " << Address << ", code: " << GetLastError() << std::endl;
	}
	VirtualProtectEx(Process, Address, 0x1000, Old, &Junk);
}

void ExternalWrite(HANDLE Process, void* Address, uint8_t byte)
{
	DWORD Old = 0;
	DWORD Junk = 0;
	VirtualProtectEx(Process, Address, 0x1000, PAGE_EXECUTE_READWRITE, &Old);
	if (!WriteProcessMemory(Process, Address, &byte, 1, NULL))
	{
		std::cout << "Write failed!" << std::endl;
	}
	VirtualProtectEx(Process, Address, 0x1000, Old, &Junk);
}

// known issue: i think discord's bitrate is not uncapped however CELT might automatically have it uncapped not fully sure
// override those 2 offsets and you are uncapped without using a hook, bitrate is maxed default on celt
int main()
{
	HMODULE VoiceEngine = {};
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
					VoiceEngine = Modules[i];
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
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + RegulatesStereoPropertyInstruction), 0xEB);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + CreateAudioFrameStereoInstruction), "\x4D\x89\xC5\x90", sizeof("\x4D\x89\xC5\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + AudioEncoderOpusConfigSetChannelsInstruction), 2);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + MonoDownmixerInstructions), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9", sizeof("\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + CELT_SIG_SCALE_offset_9186), "\xFF\xFF\xFF\x47", sizeof("\xFF\xFF\xFF\x47") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + opus_encoder_create_CELT_mode_offset_9186), "\xE9\x03", sizeof("\xE9\x03") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + HighPassFilter_Process), "\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3", sizeof("\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3") - 1);
	std::cout << "Patches applied." << std::endl;
	system("pause");
}
