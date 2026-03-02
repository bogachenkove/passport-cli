Passport CLI

Passport CLI is a secure, cross-platform command-line database written in modern C++. It stores your sensitive data (passwords, notes, bank cards, Wi‑Fi credentials, crypto keys, etc.) in an encrypted, fixed‑size file using state‑of‑the‑art cryptography. All secrets are kept in protected memory and erased on destruction.

Features

- Strong encryption – Argon2id key derivation + XChaCha20‑Poly1305 AEAD (provided by libsodium).
- Multiple record types:
  - Password records (login, password, URL, note)
  - Note records (title, note)
  - Bank card records (card number, expiry, CVV, cardholder, note)
  - Discount card records (card number, barcode, CVV, store name, note)
  - Transport card records (card number, barcode, expiry, holder, CVV, note)
  - Mnemonic records (BIP‑39 seed phrase, passphrase, iteration count, language, note)
  - Wi‑Fi network records (SSID, password, security type, note)
  - Key records (blockchain/application keys: chain, symbol, public key, private key, note)
- Secure memory handling – all secrets are stored in SecureString (page‑locked, automatically zeroed).
- Type‑safe record identification – each record type is authenticated via a BLAKE2b hash derived from a master seed and context salt.
- Forward‑compatible format – records are prefixed with a length and type.
- Fixed‑size database file – every file is exactly 100 MiB, with random padding to hide the true size of the data.
- Interactive CLI – menu‑driven interface with masked input and UTF‑8 support.
- Automatic password generation – create strong random master passwords.
- Cross‑platform – Windows (UTF‑8 console, _getwch) and Unix‑like (termios).

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
- Choose [C] to create a new database (or select [O] to open an existing database).
- Enter a file path (or just a directory – a unique filename will be generated).
- Select password mode: [M]anual or [A]uto‑generated.
- Follow the prompts to set a strong master password (min. 12 characters with at least one lowercase, uppercase, digit, and special character).

Main menu
- [L] – list records of a selected type.
- [A] – add a new record (password, note, card, Wi‑Fi, or hash – which includes mnemonic and key records).
- [R] – remove one or more records by number.
- [S] – save the database immediately.
- [E] – save and exit.
- [Q] – quit without saving.

All sensitive input (passwords, keys) is masked and stored securely.

Metadata Flags

Flag        | Description
------------|---------------------------------
--about     | Show program information
--version   | Show version information
--license   | Display the MIT license
--support   | Show support / donation info
--help      | Show this help message

Database Format

The file is exactly 100 MiB (see core::constants::kMaxFileSize) and consists of:

- Header (fixed size):
  - 4‑byte magic PSRT
  - 1‑byte null
  - Salt (16 bytes)
  - Nonce (24 bytes)
  - Creation timestamp (uint64, big‑endian)
  - Modification timestamp (uint64, big‑endian)
  - Record count placeholder (uint32, big‑endian)
- Payload length (uint32, big‑endian) – length of the encrypted payload
- Encrypted payload (XChaCha20‑Poly1305)

The decrypted payload contains:
- Master seed (32 bytes) + context salt (16 bytes) – used to derive type identifiers.
- A sequence of records. Each record is stored as:
  - Length (4 bytes, uint32 big‑endian) – total size of this record including the length field, type identifier, and data.
  - Type (32 bytes) – BLAKE2b hash of master_seed || context_salt || context_string (e.g., "PASSPORT::PASSWORD//DATABASE").
  - Data – variable‑length fields specific to the record type (e.g., login, password, URL, note).

The remaining space in the fixed‑size file is filled with random bytes (padding) before encryption to conceal the true data size.

License

This project is licensed under the MIT License – see the LICENSE file for details.

Author

Bogachenko Vyacheslav
Copyright (c) 2026 Bogachenko Vyacheslav (bogachenkove@outlook.com)
https://github.com/bogachenkove/passport-cli