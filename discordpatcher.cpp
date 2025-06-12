#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>

/* Only works for the 1.0.9186 version of the voice node, signature scanning will be added later prob */
uint32_t CreateAudioFrameStereoInstruction = 0xAD794; // patch to the bytes 4D 89 C5 90
uint32_t AudioEncoderOpusConfigSetChannelsInstruction = 0x302EA8; // patch to 02
uint32_t MonoDownmixerInstructions = 0x95B23; // patch to 90 90 90 90 90 90 90 90 90 E9
//uint32_t opus_encoder_create_CELT_mode_offset_9186 = 0x8B0A44; // should always be EA 03 (non functional)
uint32_t HighPassFilter_Process = 0x4A5022; // patch to 48 B8 10 9E D8 CF 08 02 00 00 C3
uint32_t OpusBitrate = 0x8B09A1; // patch to 48 C7 C0 FF FF FF FF 48 89 86 A0 00 00 00 90 90 90 90
uint32_t OpusUserBitrate = 0x8B0992; // patch to FF FF
uint32_t VoiceBitrate = 0x302EBA; // patch to 00 E8 03
uint32_t EmulateStereoSuccess = 0x497504; // patch to BD 01 00 00 00 90 90 90 90 90 90 90 90 90 90 90 90
uint32_t EmulateBitrateModified = 0x497762; // patch to 48 C7 C5 00 D0 07 00 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 ; patching to 512000 for cosmetic purposes since kidz don't like 256000 x22 nop
uint32_t HighpassCutoffFilter = 0x8B4370; // the bytes needed for a simple loop should not exceed 0x100 (around 35% the function length)
uint32_t DcReject = 0x8B4550; // the bytes needed for this cannot exceed 0x1B6, so we will write 0x1B6
uint32_t downmix_func = 0x8B0BB0; // patch to C3 to remove this routine

extern "C" void dc_reject(const float* in, float* out, int* hp_mem, int len, int channels, int Fs);
extern "C" void hp_cutoff(const float* in, int cutoff_Hz, float* out, int* hp_mem, int len, int channels, int Fs, int arch);

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

/* known issues for the developers who want to try and fix besides me (I will usually fix these if they exist)
* Bitrate uncap not working yet cause prob need to relook at the assembly
* All other issues with the lag and FEC should be resolved with the removal of the 1/0 returning routine that was getting patched and causing problems
*/
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
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + EmulateStereoSuccess), "\xBD\x01\x00\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", sizeof("\xBD\x01\x00\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + CreateAudioFrameStereoInstruction), "\x4D\x89\xC5\x90", sizeof("\x4D\x89\xC5\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + AudioEncoderOpusConfigSetChannelsInstruction), 2);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + MonoDownmixerInstructions), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9", sizeof("\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + HighPassFilter_Process), "\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3", sizeof("\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + OpusBitrate), "\x48\xC7\xC0\xFF\xFF\xFF\xFF\x48\x89\x86\xA0\x00\x00\x00\x90\x90\x90\x90", sizeof("\x48\xC7\xC0\xFF\xFF\xFF\xFF\x48\x89\x86\xA0\x00\x00\x00\x90\x90\x90\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + OpusUserBitrate), "\xFF\xFF", sizeof("\xFF\xFF") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + VoiceBitrate), "\x00\xE8\x03", sizeof("\x00\xE8\x03") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + HighpassCutoffFilter), (const char*)hp_cutoff, 0x100);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + DcReject), (const char*)dc_reject, 0x1B6);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + downmix_func), "\xC3", 1);
	//ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + EmulateBitrateModified), "\x48\xC7\xC5\x00\xD0\x07\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", sizeof("\x48\xC7\xC5\x00\xD0\x07\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90") - 1);
	std::cout << "Patches applied." << std::endl;
	system("pause");
}
