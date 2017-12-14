
#include <inttypes.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "app.h"

using namespace std;

RSA *str2privkey(const char* privateKeyStr)
{
 BIO *bio = BIO_new_mem_buf((void*)privateKeyStr, -1);
 BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
 RSA* rsaPrivKey = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
 if (!rsaPrivKey) {
   printf("*** str2privkey: PEM_read_bio_RSAPrivateKey failed: %s\n",
     ERR_error_string(ERR_get_error(), NULL));
   return 0;
 }
 BIO_free(bio);
 return rsaPrivKey;
}

// TODO: max in_size have to be derived from current key size

u8* rsaEncrypt(RSA *privKey, const u8* in_data, int in_size, int *out_size)
{
 u8* enc_buf = (u8 *)malloc(RSA_size(privKey));
 *out_size = RSA_private_encrypt(in_size, (const unsigned char*)in_data, enc_buf, privKey,
   RSA_PKCS1_PADDING);
 if (*out_size == -1) {
   printf("*** RSA_private_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));
   free(enc_buf);
   return 0;
 }
 return enc_buf;
}

// encrypt file by private key
// ./enc in.bin out.bin priv.key
int main(int argc, char **argv)
{
 ERR_load_crypto_strings();
 OpenSSL_add_all_algorithms();
 if (argc < 4)
   exit(-1); 
 ifstream in_file(argv[1], ios::binary | ios::ate);
 if (!in_file) {
   cout << "Can't open " << argv[1] << " !\n";
   exit(-1);
 }
 ifstream privkey_file(argv[3]);
 stringstream priv_key;
 priv_key << privkey_file.rdbuf();
 if (!privkey_file) {
   cout << "Can't load privkey !\n";
   exit(-1);
 }
 RSA *privKey = str2privkey(priv_key.str().c_str());
 if (!privKey) {
   cout << "Can't load privkey !\n";
   exit(-1);
 } 
 uint64_t in_size = in_file.tellg();
 in_file.seekg(0, ios::beg);
 u8 in_data[ENCRYPT_BLOCK_SIZE];
 ofstream out_file(argv[2], ios::binary);
 if (!out_file) {
   cout << "Can't open " << argv[2] << " !\n";
   exit(-1);
 }
 out_file.write((char *)&in_size, sizeof(in_size));
 cout << "encrypting " << in_size << " bytes of " << argv[1] << " to " << argv[2] << " .. ";
 for (uint64_t total = 0; total < in_size; total += ENCRYPT_BLOCK_SIZE) {
   int n_read = ((total + ENCRYPT_BLOCK_SIZE) < in_size) ? ENCRYPT_BLOCK_SIZE : in_size - total;
   in_file.read((char *)in_data, n_read);
   if (!in_file)
     break;
   int enc_size;
   u8* enc_data = rsaEncrypt(privKey, in_data, ENCRYPT_BLOCK_SIZE, &enc_size);
   if (!enc_data)
     exit(-1);
   out_file.write((char *)enc_data, enc_size);
   free(enc_data);
 }
 in_file.close();
 out_file.close();
 cout << "done\n";
 RSA_free(privKey);
 ERR_free_strings();
}
