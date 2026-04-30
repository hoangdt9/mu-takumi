# Ma trận tương thích — Client Takumi ↔ OpenMU (fork)

Điền dần khi có **golden pcap**/log từ client thật nối **server cũ** hoặc **staging OpenMU**. Đối chiếu `docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`.

| Phase | Opcode / flow | Client Takumi (expected) | OpenMU vanilla | Fork Takumi note |
|-------|----------------|---------------------------|----------------|------------------|
| Connect | UDP server list request/response | TBD capture | Often 44406 (muonline) — verify | Spike |
| Connect | Versión handshake | Season 6 / EX603 heuristic | Align | See `*_UPDATE=603` in vcxproj |
| Join | Account login MD5/GlobalPassword | Join `MD5Encryption=0`; `GlobalPassword` in ini | TBD | |
| Join | Character list/create | | | |
| Game | TCP port | `55901` / `55920` shards | Configure multi-world | Map 2 shards |
| Game | Encryption/session key | See `Protect.cpp` stack; ENCRYPT macros in stdafx | TBD | |
| Anti-cheat | XShield dependency | Batch starts XShield.exe | Usually none — policy | Decide remove/replace |

## Chỉ mục từ mã C++ (bổ sung pcap sau)

Điều hướng **head/sub** và hằng **`PROTOCOL_CODE*`** đã được tóm trong [**`TAKUMI-PROTOCOL-DISPATCH-INDEX.md`**](TAKUMI-PROTOCOL-DISPATCH-INDEX.md). **Không thay thế** golden capture — chỉ là mỏ neo khi grep `Protocol.cpp`/OpenMU handler.

**Quy trình:** mỗi dòng trên được coi là DONE khi có file hex dump hoặc test TCP integration chứng minh.

**Liên quan:**

- Phase 0: [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../TAKUMI-MIGRATION-OPENMU-CHECKLIST.md)
- Ports: [`TAKUMI-SERVER-NETWORK-BASELINE.md`](TAKUMI-SERVER-NETWORK-BASELINE.md)
