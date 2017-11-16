//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015-2017 Raphael Bost
//
// This file is part of libsse_crypto.
//
// libsse_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// libsse_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with libsse_crypto.  If not, see <http://www.gnu.org/licenses/>.
//

#include "../src/mbedtls/bignum.h"
#include "../src/mbedtls/rsa.h"
#include "../src/mbedtls/rsa_io.h"
#include "../src/random.hpp"


#include "gtest/gtest.h"

#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

using namespace std;
int mbedTLS_rng_wrap(void *arg, unsigned char *out, size_t len)
{
    sse::crypto::random_bytes(len, out);
    return 0;
}
TEST(mbedTLS, bignum)
{
    ASSERT_EQ(mbedtls_mpi_self_test(1),0);
}

// This test is derived from the mbedtls_rsa_self_test routine
// of mbedTLS' RSA implementation, and performs basic testing

/*
 * Example RSA-1024 keypair, for test purposes
 */
#define KEY_LEN 128

#define RSA_N   "9292758453063D803DD603D5E777D788" \
"8ED1D5BF35786190FA2F23EBC0848AEA" \
"DDA92CA6C3D80B32C4D109BE0F36D6AE" \
"7130B9CED7ACDF54CFC7555AC14EEBAB" \
"93A89813FBF3C4F8066D2D800F7C38A8" \
"1AE31942917403FF4946B0A83D3D3E05" \
"EE57C6F5F5606FB5D4BC6CD34EE0801A" \
"5E94BB77B07507233A0BC7BAC8F90F79"

#define RSA_E   "10001"

#define RSA_D   "24BF6185468786FDD303083D25E64EFC" \
"66CA472BC44D253102F8B4A9D3BFA750" \
"91386C0077937FE33FA3252D28855837" \
"AE1B484A8A9A45F7EE8C0C634F99E8CD" \
"DF79C5CE07EE72C7F123142198164234" \
"CABB724CF78B8173B9F880FC86322407" \
"AF1FEDFDDE2BEB674CA15F3E81A1521E" \
"071513A1E85B5DFA031F21ECAE91A34D"

#define RSA_P   "C36D0EB7FCD285223CFB5AABA5BDA3D8" \
"2C01CAD19EA484A87EA4377637E75500" \
"FCB2005C5C7DD6EC4AC023CDA285D796" \
"C3D9E75E1EFC42488BB4F1D13AC30A57"

#define RSA_Q   "C000DF51A7C77AE8D7C7370C1FF55B69" \
"E211C2B9E5DB1ED0BF61D0D9899620F4" \
"910E4168387E3C30AA1E00C339A79508" \
"8452DD96A9A5EA5D9DCA68DA636032AF"

#define RSA_DP  "C1ACF567564274FB07A0BBAD5D26E298" \
"3C94D22288ACD763FD8E5600ED4A702D" \
"F84198A5F06C2E72236AE490C93F07F8" \
"3CC559CD27BC2D1CA488811730BB5725"

#define RSA_DQ  "4959CBF6F8FEF750AEE6977C155579C7" \
"D8AAEA56749EA28623272E4F7D0592AF" \
"7C1F1313CAC9471B5C523BFE592F517B" \
"407A1BD76C164B93DA2D32A383E58357"

#define RSA_QP  "9AE7FBC99546432DF71896FC239EADAE" \
"F38D18D2B2F0E2DD275AA977E2BF4411" \
"F5A3B2A5D33605AEBBCCBA7FEB9F2D2F" \
"A74206CEC169D74BF5A8C50D6F48EA08"

#define MBED_RSA_TEST_COUNT 100

#define ASSERT_MPI(f) ASSERT_EQ(f,0)

TEST(mbedTLS, basic_rsa)
{
    int ret = 0;
    mbedtls_rsa_context rsa;
    mbedtls_mpi a,c;
    unsigned char a_buffer[KEY_LEN], b_buffer[KEY_LEN];
    unsigned char c_buffer[KEY_LEN];

    mbedtls_rsa_init( &rsa, 0, 0 );

    rsa.len = KEY_LEN;
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.N , 16, RSA_N  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.E , 16, RSA_E  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.D , 16, RSA_D  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.P , 16, RSA_P  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.Q , 16, RSA_Q  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.DP, 16, RSA_DP ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.DQ, 16, RSA_DQ ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.QP, 16, RSA_QP ) );

    ASSERT_EQ(mbedtls_rsa_check_pubkey(&rsa), 0);
    ASSERT_EQ(mbedtls_rsa_check_privkey(&rsa), 0);

    ASSERT_EQ(rsa.len, KEY_LEN);

    mbedtls_mpi_init(&a);
    mbedtls_mpi_init(&c);

    for (size_t t_count = 0; t_count < MBED_RSA_TEST_COUNT; t_count++) {

        ASSERT_MPI( mbedtls_mpi_fill_random( &a, KEY_LEN, mbedTLS_rng_wrap, NULL ) );
        ASSERT_MPI( mbedtls_mpi_mod_mpi(&a,&a,&rsa.N) );

        ASSERT_MPI( mbedtls_mpi_write_binary(&a, a_buffer, KEY_LEN) );


        ASSERT_MPI( mbedtls_rsa_public( &rsa, a_buffer, b_buffer ) );
        ASSERT_MPI( mbedtls_rsa_private( &rsa, mbedTLS_rng_wrap, NULL, b_buffer, c_buffer ));

        ASSERT_MPI( mbedtls_mpi_read_binary( &c, c_buffer, KEY_LEN ) );

        ASSERT_EQ(mbedtls_mpi_cmp_mpi(&a, &c),0);
        
    }

cleanup:
    mbedtls_mpi_free(&a);
    mbedtls_mpi_free(&c);
    mbedtls_rsa_free( &rsa );
    ASSERT_EQ(ret,0);
}

TEST(mbedTLS, key_serialization)
{
    int ret = 0;
    mbedtls_rsa_context rsa;
    mbedtls_rsa_context rsa_cp;
    mbedtls_rsa_context rsa_pk;
    unsigned char buf[5000];
    
    mbedtls_rsa_init( &rsa, 0, 0 );
    mbedtls_rsa_init( &rsa_cp, 0, 0 );
    mbedtls_rsa_init( &rsa_pk, 0, 0 );

    rsa.len = KEY_LEN;
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.N , 16, RSA_N  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.E , 16, RSA_E  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.D , 16, RSA_D  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.P , 16, RSA_P  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.Q , 16, RSA_Q  ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.DP, 16, RSA_DP ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.DQ, 16, RSA_DQ ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &rsa.QP, 16, RSA_QP ) );
    
    // check that everything is in order to proceed
    ASSERT_EQ(mbedtls_rsa_check_pubkey(&rsa), 0);
    ASSERT_EQ(mbedtls_rsa_check_privkey(&rsa), 0);
    
    ASSERT_EQ(rsa.len, KEY_LEN);

    
    MBEDTLS_MPI_CHK( mbedtls_rsa_write_key_pem( &rsa, buf, sizeof( buf )) );
    // must include the \0 character in the string length
    MBEDTLS_MPI_CHK( mbedtls_rsa_parse_key( &rsa_cp, buf, strlen((char*) buf)+1, 0, 0));

    // check that we parsed everthing correctly
    ASSERT_EQ(mbedtls_rsa_check_pubkey(&rsa_cp), 0);
    ASSERT_EQ(mbedtls_rsa_check_privkey(&rsa_cp), 0);

    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.N, &rsa_cp.N) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.E, &rsa_cp.E) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.D, &rsa_cp.D) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.P, &rsa_cp.P) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.Q, &rsa_cp.Q) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.DP, &rsa_cp.DP) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.DQ, &rsa_cp.DQ) == 0);

    // erase the buffer
    memset(buf, 0x00, sizeof(buf));
    
    // serialize the PK
    MBEDTLS_MPI_CHK( mbedtls_rsa_write_pubkey_pem( &rsa, buf, sizeof( buf )) );
    // must include the \0 character in the string length
    MBEDTLS_MPI_CHK( mbedtls_rsa_parse_public_key( &rsa_pk, buf, strlen((char*)buf)+1 ));

    // check the public key
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.N, &rsa_pk.N) == 0);
    ASSERT_TRUE(mbedtls_mpi_cmp_mpi(&rsa.E, &rsa_pk.E) == 0);

    ASSERT_EQ(mbedtls_rsa_check_pub_priv(&rsa_pk, &rsa), 0);

cleanup:
    mbedtls_rsa_free( &rsa );
    mbedtls_rsa_free( &rsa_cp );
    mbedtls_rsa_free( &rsa_pk );
    ASSERT_EQ(ret,0);
}


#define OPEN_SSL_BN_COMPAT_TEST_COUNT 500

#define OPEN_SSL_COMPAT_BIT_COUNT 2048
#define OPEN_SSL_COMPAT_BYTE_COUNT (OPEN_SSL_COMPAT_BIT_COUNT+7)/8

TEST(mbedTLS, open_ssl_bn_to_mpi)
{
    srandom(static_cast<int>(time(NULL)));
    
    uint8_t buffer[OPEN_SSL_COMPAT_BYTE_COUNT];
    BIGNUM *y = BN_new();
    mbedtls_mpi z;
    mbedtls_mpi_init(&z);
    
    for (size_t t = 0; t < OPEN_SSL_BN_COMPAT_TEST_COUNT; t++) {
        // generate a random number of BIT_COUNT bits
        // the most significant bit can be 0
        BN_pseudo_rand(y, OPEN_SSL_COMPAT_BIT_COUNT, -1, 0);
        // bn2bin returns a BIG endian array, so be careful ...
        ASSERT_GE(OPEN_SSL_COMPAT_BYTE_COUNT, BN_num_bytes(y));
        size_t pos = OPEN_SSL_COMPAT_BYTE_COUNT - BN_num_bytes(y);
        // set the leading bytes to 0
        memset(buffer, 0, OPEN_SSL_COMPAT_BYTE_COUNT);
        BN_bn2bin(y, buffer+pos);
        
        // read the buffer in a mbedTLS bignum
        
        ASSERT_MPI( mbedtls_mpi_read_binary( &z, buffer, OPEN_SSL_COMPAT_BYTE_COUNT ) );
        
        for (int i = 0; i < OPEN_SSL_COMPAT_BIT_COUNT; i++) {
            ASSERT_TRUE(BN_is_bit_set(y,i) == mbedtls_mpi_get_bit(&z,i));
        }
    }
}

TEST(mbedTLS, mpi_to_open_ssl_bn)
{
    srandom(static_cast<int>(time(NULL)));
    
    uint8_t buffer[OPEN_SSL_COMPAT_BYTE_COUNT];
    BIGNUM *y = BN_new();
    mbedtls_mpi z;
    mbedtls_mpi_init(&z);
    
    for (size_t t = 0; t < OPEN_SSL_BN_COMPAT_TEST_COUNT; t++) {
        mbedtls_mpi_fill_random( &z, OPEN_SSL_COMPAT_BYTE_COUNT, mbedTLS_rng_wrap, NULL );
        mbedtls_mpi_write_binary(&z, buffer, OPEN_SSL_COMPAT_BYTE_COUNT);
        
        BN_bin2bn(buffer, OPEN_SSL_COMPAT_BYTE_COUNT, y);
        
        for (int i = 0; i < OPEN_SSL_COMPAT_BIT_COUNT; i++) {
            ASSERT_TRUE(BN_is_bit_set(y,i) == mbedtls_mpi_get_bit(&z,i));
        }
    }
}

static bool cpm_mpi_bn(mbedtls_mpi* x, BIGNUM* y)
{
    bool res = true;
    for (int i = 0; i < std::max<size_t>(BN_num_bits(y), mbedtls_mpi_bitlen(x)); i++) {
        res &= (BN_is_bit_set(y,i) == mbedtls_mpi_get_bit(x,i));
    }
    return res;
}

#define OPEN_SSL_KEY_COMPAT_TEST_COUNT 20
TEST(mbedTLS, key_serialization_compat_mbedtls2openssl)
{
    for (size_t t = 0; t < OPEN_SSL_KEY_COMPAT_TEST_COUNT; t++) {

        mbedtls_rsa_context mbedtls_rsa;
        mbedtls_rsa_init( &mbedtls_rsa, 0, 0 );
        RSA* openssl_sk_rsa;
        RSA* openssl_pk_rsa;
        BIO *mem;
        EVP_PKEY* evpkey;
        unsigned char buf[5000];

        mbedtls_rsa_gen_key(&mbedtls_rsa, mbedTLS_rng_wrap, NULL, KEY_LEN*8, 0x10001L);

        // private key

        // write the mbedTLS key in a buffer
        ASSERT_EQ(mbedtls_rsa_write_key_pem( &mbedtls_rsa, buf, sizeof( buf )), 0);

        // create the OpenSSL key from the buffer
        mem = BIO_new_mem_buf(buf, (int)strlen((char*)buf));
        evpkey = PEM_read_bio_PrivateKey(mem, NULL, NULL, NULL);

        ASSERT_FALSE(evpkey == NULL);
        openssl_sk_rsa = EVP_PKEY_get1_RSA(evpkey);

        // close and destroy the BIO
        BIO_set_close(mem, BIO_NOCLOSE);
        BIO_free(mem);
        mem = NULL;

        EVP_PKEY_free(evpkey);

        // check that the keys are identical
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.N, openssl_sk_rsa->n));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.E, openssl_sk_rsa->e));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.P, openssl_sk_rsa->p));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.Q, openssl_sk_rsa->q));


        // public key
        memset(buf, 0, sizeof(buf));
        ASSERT_EQ( mbedtls_rsa_write_pubkey_pem( &mbedtls_rsa, buf, sizeof( buf )), 0 );


        // create the OpenSSL key from the buffer
        mem = BIO_new_mem_buf(buf, (int) strlen((char*)buf));
        openssl_pk_rsa = PEM_read_bio_RSA_PUBKEY(mem, NULL, NULL, NULL);
        ASSERT_FALSE(openssl_pk_rsa == NULL);


        // close and destroy the BIO
        BIO_set_close(mem, BIO_NOCLOSE);
        BIO_free(mem);

        // check that the public element are identical
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.N, openssl_pk_rsa->n));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa.E, openssl_pk_rsa->e));

   }
}

TEST(mbedTLS, key_serialization_compat_openssl2mbedtls)
{

    for (size_t t = 0; t < OPEN_SSL_KEY_COMPAT_TEST_COUNT; t++) {
        
        mbedtls_rsa_context mbedtls_rsa_sk;
        mbedtls_rsa_context mbedtls_rsa_pk;
        mbedtls_rsa_init( &mbedtls_rsa_sk, 0, 0 );
        mbedtls_rsa_init( &mbedtls_rsa_pk, 0, 0 );
        RSA* openssl_rsa = RSA_new();
        EVP_PKEY* evpkey;
        unsigned char buf[5000];
        
        unsigned long e = 0x10001;
        BIGNUM *bne = NULL;
        bne = BN_new();
        
        ASSERT_EQ(BN_set_word(bne, e), 1);
        ASSERT_EQ(RSA_generate_key_ex(openssl_rsa, KEY_LEN*8, bne, NULL), 1);
        
        // private key
        // create an EVP encapsulation
        evpkey = EVP_PKEY_new();
        ASSERT_EQ(EVP_PKEY_set1_RSA(evpkey, openssl_rsa), 1);
        
        // initialize a buffer
        BIO *bio = BIO_new(BIO_s_mem());
        
        // write the openssl key to the buffer
        ASSERT_EQ(PEM_write_bio_PKCS8PrivateKey(bio, evpkey, NULL, NULL, 0, NULL, NULL),1);
        
        // put the buffer in a C string
        size_t len = BIO_ctrl_pending(bio);
        ASSERT_LE(len+1, sizeof(buf));
        
        ASSERT_NE(BIO_read(bio, buf, (int)len), 0);
        
        EVP_PKEY_free(evpkey);
        BIO_free_all(bio);
        
        // create an mbedTLS key from the buffer
        ASSERT_EQ(mbedtls_rsa_parse_key( &mbedtls_rsa_sk, buf, strlen((char*) buf)+1, 0, 0), 0);
        
        
        // check that the keys are identical
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.N, openssl_rsa->n));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.E, openssl_rsa->e));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.P, openssl_rsa->p));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.Q, openssl_rsa->q));

        // public key
        // create an EVP encapsulation
        
        // initialize a buffer
        bio = BIO_new(BIO_s_mem());
        
        // write the openssl key to the buffer
        ASSERT_EQ(PEM_write_bio_RSA_PUBKEY(bio, openssl_rsa),1);
        
        // put the buffer in a C string
        len = BIO_ctrl_pending(bio);
        ASSERT_LE(len+1, sizeof(buf));
        
        ASSERT_NE(BIO_read(bio, buf, (int)len), 0);
        
        BIO_free_all(bio);
        
        std::string v(reinterpret_cast<const char*>(buf), len);
//        std::cout << v << std::endl;
        
        // create an mbedTLS key from the buffer
        ASSERT_EQ(mbedtls_rsa_parse_public_key( &mbedtls_rsa_pk, buf, strlen((char*) buf)+1), 0);
        
        
        // check that the keys are identical
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.N, openssl_rsa->n));
        ASSERT_TRUE(cpm_mpi_bn(&mbedtls_rsa_sk.E, openssl_rsa->e));

    }
}
