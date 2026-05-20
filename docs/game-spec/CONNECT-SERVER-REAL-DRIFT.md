# Connect layout — `MuServer/1.ConnectServer` vs `1.ConnectServer_real`

**Ngày:** 2026-04-30  
**Lệnh:** `diff -rq` (không đếm log); so sánh byte `ConnectServer.ini`, `ServerList.ini`.

## Kết quả

| Hạng mục | `1.ConnectServer` | `1.ConnectServer_real` |
|----------|---------------------|-------------------------|
| `ConnectServer.ini` | **Giống hệt** | **Giống hệt** |
| `ServerList.ini` | **Giống hệt** | **Giống hệt** |
| Binary chính | `ConnectServer.exe` (snapshot) | `ConnectServer_real.exe` |
| VC++ runtime | Có `msvcp140.dll`, `vcruntime140.dll` | Không trong thư mục (giả định cài VC++ hoặc path khác) |
| PDB | `ConnectServer.pdb` | Không trong diff mẫu |

**Authority config:** Hai bản dùng **cùng** INI được track trong repo — migration OpenMU chỉ cần một khung **`ServerList` + cổ** (tham chiếu [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](../protocol/TAKUMI-SERVER-NETWORK-BASELINE.md)). Tên **`_real`** phản ánh bản deploy / exe build khác, **không** drift nội dung INI trong snapshot này.

**Liên kết:** [`docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §4.
