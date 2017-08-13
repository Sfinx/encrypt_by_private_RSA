
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "app.h"

using namespace std;

RSA *str2pubkey(const char* publicKeyStr)
{
 BIO *bio = BIO_new_mem_buf((void *)publicKeyStr, -1);
 BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
 RSA *rsaPubKey = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
 if (!rsaPubKey)
   printf("*** str2pubkey: PEM_read_bio_RSA_PUBKEY failed: %s\n",
     ERR_error_string(ERR_get_error(), NULL));
 BIO_free(bio);
 return rsaPubKey;
}

u8* rsaDecrypt(RSA *pubKey, const u8* in_data, int in_size, int *out_size)
{
 u8 *dec_buf = (u8 *) malloc(RSA_size(pubKey));
 *out_size = RSA_public_decrypt(in_size, in_data, dec_buf, pubKey, RSA_PKCS1_PADDING);
 if (*out_size == -1) {
   printf("*** RSA_public_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));
   free(dec_buf);
   return 0;
 }
 return dec_buf;
}

// decrypt file by public key
// ./dec in.bin out.bin pub.key

int main(int argc, char **argv)
{
 ERR_load_crypto_strings();  
 if (argc < 4)
   exit(-1);
 ifstream in_file(argv[1], ios::binary | ios::ate);
 if (!in_file) {
   cout << "Can't open " << argv[1] << " !\n";
   exit(-1);
 } 
 ifstream pubkey_file(argv[3]);
 stringstream pub_key;
 pub_key << pubkey_file.rdbuf();
 if (!pubkey_file) {
   cout << "Can't load pubkey !\n";
   exit(-1);
 }
 RSA *pubKey = str2pubkey(pub_key.str().c_str());
 streamsize total_size = in_file.tellg();
 total_size -= 4; // file size data at the beginning
 in_file.seekg(0, ios::beg);
 streamsize in_size;
 in_file.read((char *)&in_size, sizeof(in_size)); 
 u8 in_data[DECRYPT_BLOCK_SIZE];
 ofstream out_file(argv[2], ios::binary);
 if (!out_file) {
   cout << "Can't open " << argv[2] << " !\n";
   exit(-1);
 }
 cout << "decrypting " << total_size << " bytes of " << argv[1] << " to " << in_size << " of " << argv[2] << " .. ";
 int total_written = 0;
 for (streamsize total = 0; total < total_size; total_size += DECRYPT_BLOCK_SIZE) {
   int n_read = ((total + DECRYPT_BLOCK_SIZE) < total_size) ? DECRYPT_BLOCK_SIZE : total_size - total;
   in_file.read((char *)in_data, n_read);
   if (!in_file)
     break;
   int dec_size;
   u8* dec_data = rsaDecrypt(pubKey, in_data, DECRYPT_BLOCK_SIZE, &dec_size);
   if (!dec_data)
     exit(-1);
   if (dec_size != ENCRYPT_BLOCK_SIZE) {
     cout << "wrong dec_size !\n";
     exit(-1);
   }
   if ((total_written + ENCRYPT_BLOCK_SIZE) > in_size)
     dec_size = in_size - total_written;
   out_file.write((char *)dec_data, dec_size);
   free(dec_data);
   total_written += dec_size;
   if (total_written >= in_size)
     break;
 }
 in_file.close();
 out_file.close();
 cout << "done\n";
 RSA_free(pubKey);
 ERR_free_strings();
}
