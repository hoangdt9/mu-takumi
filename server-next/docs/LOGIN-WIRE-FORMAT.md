# Login packet pipeline (Season 6 / Takumi client ↔ OpenMU-compatible host)

Reference implementation: **OpenMU** `PipelinedDecryptor` + `LogInHandlerPlugIn` + `LoginLongPassword`.

The Takumi C++ client builds the login request in `SendRequestLogIn` (`wsclientinline.h`):

- Inner logical packet: **C1**, head **0xF1**, sub **0x01**.
- Fields (after head bytes `[C1][Len][F1]`): sub **0x01** (first XOR’d payload byte at index 3), then:
  - Username **10** bytes (BuxConvert / “Xor3” on plaintext),
  - Password **20** bytes (same),
  - **DWORD** `GetTickCount()` (little-endian on Windows/Android),
  - Client version **5** bytes: `(Version[i] - (i+1))`,
  - Serial **16** bytes: `Serial[i]`.

`CStreamPacketEngine` applies the **32-byte XOR chain** (same key as OpenMU `DefaultKeys.Xor32Key`) from index **3** upward (indices **0–2** are raw `C1`, length, `F1`).

Then `spe.Send(TRUE)` wraps that inner packet with **SimpleModulus** client→server encryption and sends **C3** (or **C4** if large), matching PC MuMain.

## Server-side decrypt order (must match OpenMU)

OpenMU `PipelinedDecryptor` (**client → server**):

1. **SimpleModulus** decrypt (keys paired with client `Enc1.dat` / server decrypt table — same family as OpenMU `PipelinedSimpleModulusDecryptor.DefaultServerKey` when using default data files).
2. **Xor32** decrypt backward (`PipelinedXor32Decryptor`: for `C1`/`C3`, header size **2**, loop `i` from `Length-1` down to **3**,  
   `buf[i] ^= buf[i-1] ^ Xor32Key[i % 32]`).

After step 2 you have a normal **C1** frame; parse length from `[1]`, head `[2]` = `0xF1`, `[3]` = `0x01`.

## Credential decoding

Username/password spans still have **BuxConvert** (3-byte XOR `0xFC, 0xCF, 0xAB`) applied — OpenMU `Xor3Decryptor` / `LoginLongPassword` layout:

- Username: bytes `[4..13]` (10),
- Password: bytes `[14..33]` (20),
- Tick: bytes `[34..37]` — OpenMU struct uses **big-endian** `uint` in the packet definition; the Takumi client sends **little-endian** DWORD. Implementations should compare credentials using the decoded username/password only; tick endian rarely affects auth if XOR+BuxConvert parsing is aligned.

## Response

Success is **`C1`**, `F1`, sub **`0x01`**, result byte **`0x01`** or **`0x20`** (see `WSclient.cpp` `TranslateProtocol` case `0xF1`).  
Then the client sets `RECEIVE_LOG_IN_SUCCESS` and sends **`F3 00`** character list (`spe.Send()` without SimpleModulus on classic Takumi — plain **C1**).

If the host only parses **plain C1** login and skips SimpleModulus, the client must use `spe.Send(FALSE)` for login — that diverges from PC/OpenMU and was **not** compatible with a decrypt-first pipeline; use **encrypted login** (`spe.Send(TRUE)`) when the server implements the stack above.

## QA checklist

- Hex trace after login click must show **outgoing `C3`** (first byte `0xC3`), not `C1`.
- Incoming login result must be parsed after client `ProtocolCompiler` (`SimpleModulusSC` + serial) — usually **`C3`** from server as well for encrypted responses.

## Native client session notes (IME / character UI)

IME ordering, modal focus, and character-delete UX are **not** part of this login wire layout but affect whether QA can complete login → list → delete on a real device. See **`../../docs/DEVELOPMENT-LOG-2026-05-14.md`** and **`IMPLEMENTATION-CHECKLIST.md`** (exit criteria for **`F3 02`**).
