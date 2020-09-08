#!/usr/bin/env python3

""" Generate AES-CTR test vectors with an independent implementation. """

import binascii
import struct

import Crypto.Util.Counter
import Crypto.Cipher.AES

PLAINTEXTS = [
    " ", "A", "AAAA", "AB", "hello", "hello world",
    "This is 16 chars",
    "Ceci n'est pas 24 chars.",
    "This block is exactly 32 chars!!",
    ]


def c_replace_var(c_var_name, c_var_value):
    """ Replace a variable in 'main.c'. """
    # Read file.
    with open("main.c") as fp:
        lines = fp.readlines()

    # Keep lines until we find the variable name.
    newfile = ""
    lookfor = "static const struct testcase %s[] = {\n" % (c_var_name)
    found = False
    for i, line in enumerate(lines):
        newfile += line
        if line == lookfor:
            found = True
            break
    lines = lines[i + 1:]
    assert(found)

    # Add new lines to the output.
    newfile += c_var_value
    found = False
    for i, line in enumerate(lines):
        if line == "};\n":
            found = True
            break
    lines = lines[i:]
    assert(found)

    # Add the remaining lines
    newfile += "".join(lines)

    # Write file.
    with open("main.c", "w") as fp:
        fp.write(newfile)


def generate_for_keylen(keylen, nonce, c_var_name):
    """ Print plaintext and ciphertext for the given key length.  """
    # key: 00 01 02 03 04...
    key_arr = bytearray(b'\0' * keylen)
    for i in range(keylen):
        key_arr[i] = i
    key = bytes(key_arr)

    # Format nonce bytes.
    nonce_bytes = struct.pack(">Q", nonce)

    formatted = []
    for plaintext in PLAINTEXTS:
        # Always initialize to the same key and initial value (for these cases).
        ctr = Crypto.Util.Counter.new(64, initial_value=0, prefix=nonce_bytes)
        aesctr = Crypto.Cipher.AES.new(key,
                                       Crypto.Cipher.AES.MODE_CTR,
                                       counter=ctr)

        ciphertext = aesctr.encrypt(plaintext.encode())

        # Format and print for C99.
        formatted.append('\t{"%s",\n\t    "%s",\n\t    "%s"}' % (
            binascii.hexlify(key).decode("ascii"),
            plaintext,
            binascii.hexlify(ciphertext).decode("ascii")))

    # Replace variable in main.c.
    c_var_value = ",\n".join(formatted) + "\n"
    c_replace_var(c_var_name, c_var_value)

generate_for_keylen(16, 0, "tests_128")
generate_for_keylen(32, 0, "tests_256")

# The nonce is a uint64_t in our C code.
nonce = 0xfedcba9876543210
generate_for_keylen(16, nonce, "tests_128_nonce")
generate_for_keylen(32, nonce, "tests_256_nonce")
