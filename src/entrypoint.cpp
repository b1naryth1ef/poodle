#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

#include "oodle2net.h"
#include "oodle2base.h"

const size_t BUFFER_SIZE = 16000;

struct OodleCompressor {
    void *in_buff;
    void *out_buff;
    void *dic;
    OodleNetwork1UDP_State *state;
    OodleNetwork1_Shared *shared;
    
    bool decode(const void *comp, OO_SINTa compLen, void *raw, OO_SINTa rawLen) {
        return OodleNetwork1UDP_Decode(state, shared, comp, compLen, raw, rawLen);
    }
};

extern "C" __declspec(dllexport) OodleCompressor* oodle_new(char* data) {
    OodleCompressor* comp = (OodleCompressor*)malloc(sizeof(OodleCompressor));

    auto magic = *(reinterpret_cast<OO_U32 *>(data));
    if (magic != 0x11235801) {
        return NULL;
    }

    auto ht_bits = *(reinterpret_cast<OO_U32 *>(&data[8]));
    auto dic_size = *(reinterpret_cast<OO_U32 *>(&data[12]));
    auto oodle_major_version = *(reinterpret_cast<OO_U32 *>(&data[16]));
    auto statecompacted_size = *(reinterpret_cast<OO_U32 *>(&data[24]));

    comp->dic = malloc(dic_size);
    auto *on1udpnew_compacted = (OodleNetwork1UDP_StateCompacted *)malloc((size_t)statecompacted_size);
    memcpy(comp->dic, &data[32], dic_size);
    memcpy(on1udpnew_compacted, &data[32 + dic_size], statecompacted_size);

    auto on1udpnew_state_size = OodleNetwork1UDP_State_Size();
    comp->state = (OodleNetwork1UDP_State *)malloc(on1udpnew_state_size);
    if (!OodleNetwork1UDP_State_Uncompact_ForVersion(comp->state, on1udpnew_compacted, oodle_major_version)) {
        return NULL;
    }
    free(on1udpnew_compacted);

    auto shared_size = OodleNetwork1_Shared_Size(ht_bits);
    comp->shared = (OodleNetwork1_Shared *)malloc(shared_size);
    OodleNetwork1_Shared_SetWindow(comp->shared, ht_bits, comp->dic, (OO_S32)dic_size);

    comp->in_buff = malloc(BUFFER_SIZE);
    comp->out_buff = malloc(BUFFER_SIZE);

    return comp;
}

extern "C" __declspec(dllexport) bool oodle_decode(OodleCompressor* comp, void* input, size_t input_len, void*output, size_t output_len) {
    bool use_input_buffer = (input_len <= BUFFER_SIZE);
    bool use_output_buffer = (output_len <= BUFFER_SIZE);
    char* in = (char*)comp->in_buff;
    char* out = (char*)comp->out_buff;

    if (use_input_buffer) {
        memcpy(comp->in_buff, input, input_len);
    } else {
        in = (char*)malloc(input_len);
        memcpy(in, input, input_len);
    }
    
    if (!use_output_buffer) {
        out = (char*)malloc(output_len);
    }

    auto result = comp->decode(in, input_len, out, output_len);

    if (!use_input_buffer) {
        free(in);
    }

    if (!result) {
        if (!use_output_buffer) {
            free(out);
        }
        return false;
    }

    memcpy(output, out, output_len);
    if (!use_output_buffer) {
        free(out);
    }
    return true;
}

extern "C" __declspec(dllexport) void oodle_destroy(OodleCompressor* comp) {
    free(comp->in_buff);
    free(comp->out_buff);
    free(comp->dic);
    free(comp->state);
    free(comp->shared);
    free(comp);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

