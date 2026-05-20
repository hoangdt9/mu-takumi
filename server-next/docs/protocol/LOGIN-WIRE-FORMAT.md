# Login wire format (Takumi ↔ `server-next`)

Short reference for **encrypted login** and pointers to the full opcode inventory (**M1**).

## Client → server (real device / PC)

- Takumi **`SendRequestLogIn`** uses `CStreamPacketEngine` with **`spe.Send(TRUE)`** — **SimpleModulus** encryption for the MU frame (Season 6 style), then the usual **Xor32** transport obfuscation where applicable.
- A host that only accepts **plaintext `C1`** will **not** see credentials from a stock Takumi Android build.
- **Key material:** `Data/Dec2.dat` (SimpleModulus modulus/exponent) must match between client and `LegacyLoginHost` (`TAKUMI_DEC2_PATH` on host, `/keys/Dec2.dat` in Docker). See `../README.md` and `../../docs/android/ANDROID-DEV-MAC.md`.

## Server → client

- **`C1 F1 01`** login result (`PHEADER_DEFAULT_SUBCODE`: `HeadCode=0xF1`, `SubCode=0x01`, `Value` = status).
- **`C1 F1 00`** join / connect continuation where used by legacy flow.
- Character roster / join / inventory: **`F3`** subcodes — layouts in `Source/5.Main/source/WSclient.h`; parity table in **`M1-PROTOCOL-PARITY-MAP.md`**.

## Full post-login opcode map

See **`M1-PROTOCOL-PARITY-MAP.md`** (`TranslateProtocol` + `ProtocolCoreEx` + legacy vs `server-next` table).
