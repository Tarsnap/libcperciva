#!/usr/bin/env python3

""" Generate AES-CTR test vectors with an independent implementation. """

import binascii

import Crypto.Util.Counter
import Crypto.Cipher.AES

PLAINTEXTS = [
    " ", "A", "AAAA", "AB", "hello", "hello world",
    "This is 16 chars",
    "Ceci n'est pas 24 chars.",
    "This block is exactly 32 chars!!",
    ]

INITIAL_VECTOR = b'\0' * 16


def generate_for_keylen(keylen):
    """ Print plaintext and ciphertext for the given key length.  """
    # key: 00 01 02 03 04...
    key_arr = bytearray(b'\0' * keylen)
    for i in range(keylen):
        key_arr[i] = i
    key = bytes(key_arr)

    print("--- test cases for %i-bit AES-CTR" % (keylen * 8))

    for plaintext in PLAINTEXTS:
        # Always initialize to the same key and initial value (for these cases).
        ctr = Crypto.Util.Counter.new(128, initial_value=0)
        aesctr = Crypto.Cipher.AES.new(key,
                                       Crypto.Cipher.AES.MODE_CTR,
                                       counter=ctr,
                                       IV=INITIAL_VECTOR)

        ciphertext = aesctr.encrypt(plaintext.encode())

        # Format and print for C99.
        print('\t{"%s",\n\t    "%s",\n\t    "%s"},' % (
            binascii.hexlify(key).decode("ascii"),
            plaintext,
            binascii.hexlify(ciphertext).decode("ascii")))


generate_for_keylen(16)
generate_for_keylen(32)
