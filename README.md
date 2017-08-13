
**Private key encryption and public key decryption openssl example**

Such approach allows to safely distribute the signed and encrypted software wich can be verified and decrypted only by public key. Keeping secret key in secure place guarantees that nobody can re-encrypt this file.

The usual RSA approach (encrypt with public key and decrypt with private one) do not work here as the public key (used for encryption) can be easily derived from private one.
