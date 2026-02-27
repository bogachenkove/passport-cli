Passport CLI

Passport CLI is a secure, cross-platform command-line database written in modern C++. It stores your sensitive data (passwords, notes, bank cards, etc.) in an encrypted database using state-of-the-art cryptography.

Features

- Strong encryption – Argon2id key derivation + XChaCha20-Poly1305 AEAD (provided by libsodium).
- Multiple record types:
  - Password records (login, password, URL, note)
  - Note records (title, note)
  - Bank card records (card number, expiry, CVV, cardholder, note)
  - Discount card records (card number, barcode, CVV, store name, note)
  - Transport card records (card number, barcode, expiry, holder, CVV, note)
  - Mnemonic phrase records (BIP‑39 seed phrase, passphrase, iteration count, language, note)
- Type‑safe record identification – each record type is authenticated via a BLAKE2b hash derived from a master seed and context salt.
- Interactive CLI – menu‑driven interface with masked password input.
- Portable database format – single encrypted file with magic header and length‑prefixed fields.
- Automatic password generation – create strong random master passwords.
- Cross‑platform – Windows (UTF‑8 console, _getch) and Unix‑like (termios).

Technology Stack

- Language: C++20
- Cryptography: libsodium (Argon2id, XChaCha20‑Poly1305, BLAKE2b, randombytes)
- Unicode normalisation: ICU (NFKD for BIP‑39 word matching)
- Build system: CMake and Make
- Dependencies: libsodium, ICU (development packages)

Building

Requirements
- CMake
- Make
- C++20 compiler (GCC, Clang)
- libsodium (static or shared)
- ICU (libraries and headers)

First launch
- Choose [C] to create a new database.
- Enter a file path (or just a directory – a unique filename will be generated).
- Select password mode: [M]anual or [A]uto‑generated.
- Follow the prompts to set a strong master password.

Main menu
- [L] – list records of a selected type.
- [A] – add a new record (password, note, card, or mnemonic phrase).
- [R] – remove one or more records by number.
- [S] – save the database immediately.
- [E] – save and exit.
- [Q] – quit without saving.

All sensitive input (passwords) is masked.

Database Format

The file consists of:
- 4‑byte magic PSRT
- 1‑byte null
- Salt (16 bytes)
- Nonce (24 bytes)
- Creation timestamp (uint64, big‑endian)
- Modification timestamp (uint64, big‑endian)
- Record count placeholder (uint32, big‑endian)
- Payload length (uint32, big‑endian)
- Encrypted payload (XChaCha20‑Poly1305)

The plaintext payload contains:
- Master seed (32 bytes) + context salt (16 bytes) – used to derive type identifiers.
- A sequence of records. Each record is stored as:
  * Length (4 bytes, uint32 big‑endian) – total size of this record including the length field itself, the type identifier, and the record's data.
  * Type (32 bytes) – BLAKE2b hash of master_seed || context_salt || context_string (e.g., "PASSPORT::PASSWORD//DATABASE").
  * Data – variable‑length fields specific to the record type (e.g., login, password, URL, note).

Metadata Flags

Flag         | Description
--------------|---------------------------------
--version    | Show version information
--homepage   | Show project homepage URL
--license    | Display the MIT license
--help       | Show this help message

License

This project is licensed under the MIT License – see the LICENSE file for details.

Author

Bogachenko Vyacheslav  
Copyright (c) 2026 Bogachenko Vyacheslav (bogachenkove@outlook.com). All rights reserved.