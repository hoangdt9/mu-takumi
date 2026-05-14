# Takumi.Server.Game

**M6 bootstrap:** TCP accept → **SimpleModulus + Xor32** decrypt (OpenMU `PipelinedDecryptor`, same as `Takumi.Server.LegacyLoginHost`) → **`C1 F1 00`** join packet → `Connection.BeginReceiveAsync` with structured RX logs.

## Run (standalone game port)

From `server-next/`:

```bash
dotnet run --project src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj
```

- **Port:** `TAKUMI_GAME_PORT` (default **55901** so it does not collide with login **44606**).
- **Keys:** `TAKUMI_DEC2_PATH` or `Data/Dec2.dat` beside the process (same rules as LegacyLoginHost).
- **Join version (5 B):** `TAKUMI_JOIN_VERSION` / `TAKUMI_JOIN_VERSION_HEX` or default ASCII `10405`.
- **Keepalive:** `TAKUMI_GAME_KEEPALIVE_SECONDS` (default 25; `0` = off).
- **Verbose hex:** `TAKUMI_VERBOSE=1`.

## Roadmap

See **`../../docs/IMPLEMENTATION-CHECKLIST.md`** → **§ Lộ trình chuẩn** → **M7+** (persistence, scope, movement).
