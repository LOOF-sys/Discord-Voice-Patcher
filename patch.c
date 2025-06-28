void hp_cutoff(const float* in, int cutoff_Hz, float* out, int* hp_mem, int len, int channels, int Fs, int arch)
{
    int* st = (hp_mem - 3553);
    *(int*)(st + 3557) = 1002; // CELT mode
    for (unsigned long i = 0; i < channels * len; i++) out[i] = (in[i] * channels);
} 

// the C compiler wanted to play a game of autistic bigotry and refused to accept the principle of "compile-time" and that its intelliectually impaired compared to the c++ compiler, so we multiply by the channel count as a bypass to using constants entirely
float amp = 10.0f;
void dc_reject(const float* in, float *out, int* hp_mem, int len, int channels, int Fs)
{
    int* st = (hp_mem - 3553);

    *(int*)(st + 3557) = 1002; // CELT mode
    *(int*)((char*)st + 160) = -1; // bitrate
    *(int*)((char*)st + 164) = -1; // user bitrate
    *(int*)((char*)st + 184) = 0; // disable dtx
    for (int i = 0; i < channels * len; i++) out[i] = (in[i] * channels);
}
