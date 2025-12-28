#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>

#define VERSION 9219
#define FILE_PATCHER_MODE 0

#if (VERSION == 9186)
/* Only works for the 1.0.9186 version of the voice node, signature scanning will be added later prob */
uint32_t CreateAudioFrameStereoInstruction = 0xAD794; // patch to the bytes 4D 89 C5 90
uint32_t AudioEncoderOpusConfigSetChannelsInstruction = 0x302EA8; // patch to 02
uint32_t MonoDownmixerInstructions = 0x95B23; // patch to 90 90 90 90 90 90 90 90 90 E9
uint32_t HighPassFilter_Process = 0x4A5022; // patch to 48 B8 10 9E D8 CF 08 02 00 00 C3
uint32_t EmulateStereoSuccess = 0x497504; // patch to BD 01 00 00 00 90 90 90 90 90 90 90 90 90 90 90 90
uint32_t EmulateBitrateModified = 0x497762; // patch to 48 C7 C5 00 D0 07 00 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 ; patching to 512000 x22 nop
uint32_t Emulate48Khz = 0x49761B; // patch to 90 90 90
uint32_t HighpassCutoffFilter = 0x8B4370; // the bytes needed for a simple loop should not exceed 0x100
uint32_t DcReject = 0x8B4550; // the bytes needed for this cannot exceed 0x1B6, so we will write 0x1B6
uint32_t downmix_func = 0x8B0BB0; // patch to C3 to remove this routine
uint32_t AudioEncoderOpusConfig_IsOk = 0x30310C; // patch to 48 C7 C0 01 00 00 00 C3 (pretty sure discord modified this to prevent high bitrates from passing which is why none of my previous patches were working)
uint32_t AudioEncoderOpusImpl_EncodeImpl_Jump = 0x4998BA; // patch to 90 E9
uint32_t AudioEncoderOpusImpl_EncodeImpl_Shellcode = 0x499A2F; // patch to 48 c7 47 20 00 d0 07 00 E9 84 FE FF FF
#else
/* Version 1.0.9219 */
uint32_t CreateAudioFrameStereoInstruction = 0x116C91; // 49 89 C5 90
uint32_t AudioEncoderOpusConfigSetChannelsInstruction = 0x3A0B64; // 02
uint32_t MonoDownmixerInstructions = 0xD6319; // 90 90 90 90 90 90 90 90 90 90 90 90 E9
uint32_t HighPassFilter_Process = 0x52CF70; // 48 B8 10 9E D8 CF 08 02 00 00 C3
uint32_t EmulateStereoSuccess1 = 0x520CFB; // 02
uint32_t EmulateStereoSuccess2 = 0x520D07; // E9
uint32_t EmulateBitrateModified = 0x52115A; // 00 EE 02
uint32_t Emulate48Khz = 0x520E63; // 90 90 90 
uint32_t HighpassCutoffFilter = 0x8D64B0;
uint32_t DcReject = 0x8D6690;
uint32_t downmix_func = 0x8D2820; // C3
uint32_t AudioEncoderOpusConfig_IsOk = 0x3A0E00; // 48 C7 C0 01 00 00 00 C3
uint32_t SetsBitrate_BitrateValue = 0x522F81; // 00 0D 07
uint32_t SetsBitrate_BitwiseOr = 0x522F89; // 90 90 90
uint32_t WebRtcOpus_SetBitrate_Check = 0x6A8FA5; // 00 00 
uint32_t ThrowError = 0x2B3340;
#endif

uint8_t* SignatureScanFor(uint8_t* Address, uint64_t Range, const char* Signature, uint64_t SignatureLength);
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

int FilePatch(HANDLE Process, HMODULE Module);
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
#if (FILE_PATCHER_MODE == 1)
					return FilePatch(Process, Modules[i]);
#endif
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
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + EmulateStereoSuccess1), 2);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + EmulateStereoSuccess2), 0xEB);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + CreateAudioFrameStereoInstruction), "\x49\x89\xC5\x90", sizeof("\x49\x89\xC5\x90") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + AudioEncoderOpusConfigSetChannelsInstruction), 2);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + MonoDownmixerInstructions), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9", sizeof("\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + EmulateBitrateModified), "\x00\xEE\x02", sizeof("\x00\xEE\x02") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + HighPassFilter_Process), "\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3", sizeof("\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + HighpassCutoffFilter), (const char*)hp_cutoff, 0x100);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + DcReject), (const char*)dc_reject, 0x1B6);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + downmix_func), 0xC3);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + Emulate48Khz), "\x90\x90\x90", sizeof("\x90\x90\x90") - 1);
	//ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + AudioEncoderOpusConfig_IsOk), "\x48\xC7\xC0\x01\x00\x00\x00\xC3", sizeof("\x48\xC7\xC0\x01\x00\x00\x00\xC3") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + SetsBitrate_BitrateValue), "\x30\xC8\x07\x00\x00", sizeof("\x30\xC8\x07\x00\x00") - 1);
	ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + SetsBitrate_BitwiseOr), "\x90\x90\x90", sizeof("\x90\x90\x90") - 1);
	//ExternalWrite(Discord, (void*)((uintptr_t)VoiceEngine + ThrowError), "\xC3", 1);
	std::cout << "Patches applied." << std::endl;
	system("pause");
}

void CloseAllProcesses(const char* process_name)
{
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot == INVALID_HANDLE_VALUE)
	{
		printf("failed to create snapshot\n");
		return;
	}

	PROCESSENTRY32 ProcessEntry = {};
	ProcessEntry.dwSize = sizeof(ProcessEntry);
	while (Process32Next(Snapshot, &ProcessEntry))
	{
		if (!strcmp(ProcessEntry.szExeFile, process_name) || strstr(ProcessEntry.szExeFile, process_name))
		{
			HANDLE Process = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessEntry.th32ProcessID);
			TerminateProcess(Process, 0);
		}
	}
	CloseHandle(Snapshot);
}

bool ProcessOpen(const char* process_name)
{
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot == INVALID_HANDLE_VALUE)
	{
		printf("failed to create snapshot\n");
		return false;
	}

	PROCESSENTRY32 ProcessEntry = {};
	ProcessEntry.dwSize = sizeof(ProcessEntry);
	while (Process32Next(Snapshot, &ProcessEntry))
	{
		if (!strcmp(ProcessEntry.szExeFile, process_name) || strstr(ProcessEntry.szExeFile, process_name))
		{
			CloseHandle(Snapshot);
			return true;
		}
	}

	CloseHandle(Snapshot);
	return false;
}

int FilePatch(HANDLE Process, HMODULE Module)
{
	char ModulePath[MAX_PATH] = {};
	if (!GetModuleFileNameExA(Process, Module, ModulePath, MAX_PATH)) return 0;
	TerminateProcess(Process, 0);
	while (ProcessOpen("Discord.exe")) CloseAllProcesses("Discord.exe");

	OFSTRUCT Useless = {};
	auto VoiceNode = (HANDLE)OpenFile(ModulePath, &Useless, OF_READWRITE);
	if (VoiceNode == INVALID_HANDLE_VALUE)
	{
		printf("failed to open discord_voice.node for readwrite\n");
		system("pause");
		return 0;
	}

	uint64_t FileSize = 0;
	if (!GetFileSizeEx(VoiceNode, (PLARGE_INTEGER)&FileSize))
	{
		printf("failed to query information about discord_voice.node\n");
		system("pause");
		return 0;
	}

	void* file_data = VirtualAlloc(nullptr, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!file_data)
	{
		printf("failed to allocate file data\n");
		system("pause");
		return 0;
	}

	if (!ReadFile(VoiceNode, file_data, FileSize, NULL, NULL))
	{
		printf("failed to read discord_voice.node\n");
		system("pause");
		return 0;
	}

	memcpy((void*)((uintptr_t)file_data + (EmulateStereoSuccess1 - 0xC00)), "\x02", 1);
	memcpy((void*)((uintptr_t)file_data + (EmulateStereoSuccess2 - 0xC00)), "\xEB", 1);
	memcpy((void*)((uintptr_t)file_data + (CreateAudioFrameStereoInstruction - 0xC00)), "\x49\x89\xC5\x90", sizeof("\x49\x89\xC5\x90") - 1);
	memcpy((void*)((uintptr_t)file_data + (AudioEncoderOpusConfigSetChannelsInstruction - 0xC00)), "\x02", 1);
	memcpy((void*)((uintptr_t)file_data + (MonoDownmixerInstructions - 0xC00)), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9", sizeof("\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xE9") - 1);
	memcpy((void*)((uintptr_t)file_data + (EmulateBitrateModified - 0xC00)), "\x00\xEE\x02", sizeof("\x00\xEE\x02") - 1);
	memcpy((void*)((uintptr_t)file_data + (HighPassFilter_Process - 0xC00)), "\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3", sizeof("\x48\xB8\x10\x9E\xD8\xCF\x08\x02\x00\x00\xC3") - 1);
	memcpy((void*)((uintptr_t)file_data + (HighpassCutoffFilter - 0xC00)), (const char*)hp_cutoff, 0x100);
	memcpy((void*)((uintptr_t)file_data + (DcReject - 0xC00)), (const char*)dc_reject, 0x1B6);
	memcpy((void*)((uintptr_t)file_data + (downmix_func - 0xC00)), "\xC3", 1);
	memcpy((void*)((uintptr_t)file_data + (Emulate48Khz - 0xC00)), "\x90\x90\x90", sizeof("\x90\x90\x90") - 1);
	memcpy((void*)((uintptr_t)file_data + (AudioEncoderOpusConfig_IsOk - 0xC00)), "\x48\xC7\xC0\x01\x00\x00\x00\xC3", sizeof("\x48\xC7\xC0\x01\x00\x00\x00\xC3") - 1);
	memcpy((void*)((uintptr_t)file_data + (SetsBitrate_BitrateValue - 0xC00)), "\x30\xC8\x07\x00\x00", sizeof("\x30\xC8\x07\x00\x00") - 1);
	memcpy((void*)((uintptr_t)file_data + (SetsBitrate_BitwiseOr - 0xC00)), "\x90\x90\x90", sizeof("\x90\x90\x90") - 1);
	memcpy((void*)((uintptr_t)file_data + (ThrowError - 0xC00)), "\xC3", 1);

	OVERLAPPED Overlap = {};
	RtlZeroMemory(&Overlap, sizeof(Overlap));
	if (!WriteFile(VoiceNode, file_data, FileSize, NULL, &Overlap))
	{
		printf("failed to perform write on discord_voice.node\n");
		system("pause");
		return 0;
	}

	CloseHandle(VoiceNode);
	printf("successfully patched discord_voice.node\n");
	system("pause");
	return 0;

	printf("discord_voice.node was not loaded\n");
	return 0;
}
