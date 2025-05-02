#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include <span>

#include <botan/auto_rng.h>
#include <botan/rsa.h>
#include <botan/pk_ops.h>
#include <botan/exceptn.h>

#define DUDECT_IMPLEMENTATION
#include "dudect.h"

static constexpr size_t PLAINTEXT_LEN = 32;
static constexpr size_t NUM_MEASUREMENTS = 5000;

static Botan::AutoSeeded_RNG rng;
static std::unique_ptr<Botan::RSA_PrivateKey> key;
static std::unique_ptr<Botan::PK_Ops::Encryption> enc_op;
static std::unique_ptr<Botan::PK_Ops::Decryption> dec_op;
static size_t g_ct_len = 0;

void init_rsa()
{
    key = std::make_unique<Botan::RSA_PrivateKey>(rng, 2048);
    enc_op = key->create_encryption_op(rng, "EME-PKCS1-v1_5", "");
    dec_op = key->create_decryption_op(rng, "EME-PKCS1-v1_5", "");
    g_ct_len = key->get_n().bytes() + 11;
}

uint8_t do_one_computation(uint8_t* data)
{
    try
    {
        uint8_t valid = 0;
        auto span_ct = std::span<const uint8_t>(data, g_ct_len);
        auto pt = dec_op->decrypt(valid, span_ct);
        volatile uint8_t dummy = pt[0];
        (void)dummy;
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

void prepare_inputs(dudect_config_t* c, uint8_t* input_data, uint8_t* classes)
{
    std::vector<uint8_t> pt(PLAINTEXT_LEN);
    for (size_t i = 0; i < pt.size(); ++i)
        pt[i] = static_cast<uint8_t>(i);

    for (size_t i = 0; i < c->number_measurements; ++i)
    {
        classes[i] = rng.next_byte() & 1;
        uint8_t* dst = input_data + i * g_ct_len;

        if (classes[i] == 0)
        {
            auto span_pt = std::span<const uint8_t>(pt.data(), pt.size());
            auto ct = enc_op->encrypt(span_pt, rng);
            std::memcpy(dst, ct.data(), g_ct_len);
        }
        else
        {
            for (size_t j = 0; j < g_ct_len; ++j)
                dst[j] = rng.next_byte();
        }
    }
}

int run_test(void)
{
    init_rsa();

    dudect_config_t config = {
        .chunk_size = static_cast<uint32_t>(g_ct_len),
        .number_measurements = static_cast<uint32_t>(NUM_MEASUREMENTS),
    };

    dudect_ctx_t ctx;
    dudect_init(&ctx, &config);

    dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
    while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET)
    {
        state = dudect_main(&ctx);
    }

    if (state == 1)
        printf("PASSED: no leakage evidence (state=%d)\n", (int)state);
    else if (state == 2)
        printf("FAILED: leakage detected (state=%d)\n", (int)state);
    else
        printf("UNKNOWN state=%d\n", (int)state);

    dudect_free(&ctx);
    return (int)state;
}

int main()
{
    return run_test();
}
