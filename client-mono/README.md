# Takumi Client MonoGame (skeleton)

MonoGame client line for Takumi — **starts after** `server-next` reaches **Gate S2**. Native C++ (`Source/5.Main`) remains the parity reference until then.

## Status

| Component | State |
|-----------|--------|
| `Takumi.Client.Protocol` | Skeleton — references `server-next` `Takumi.Server.Protocol` |
| `Client.Main` / MonoGame heads | Not started (see roadmap) |

## Build

```bash
cd client-mono
dotnet build Takumi.Client.slnx -c Release
dotnet test src/Takumi.Client.Protocol.Tests/Takumi.Client.Protocol.Tests.csproj -c Release
```

Requires **.NET 10** and sibling folder `../server-next/`.

## Docs

- Roadmap: `../server-next/docs/client/MONOGAME-CLIENT-ROADMAP.md`
- Server checklist: `../server-next/docs/milestones/IMPLEMENTATION-CHECKLIST.md`
- S2 QA (C++ + Docker): `../server-next/docs/qa-gates/S2-GATE-QA.md`
