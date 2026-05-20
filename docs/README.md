# Takumi docs

Tài liệu repo `takumi/` — client native, migration, QA APK. **Dev checklist server:** [`../server-next/docs/README.md`](../server-next/docs/README.md).

## Cấu trúc

| Thư mục | Nội dung |
|---------|----------|
| [`android/`](android/) | Build Mac, input touch, skill combat MG, rollout plan, matrix CSV |
| [`qa/`](qa/) | Checklist QA trên APK (S2, M8, M9…) — index [`QA-MILESTONE.md`](QA-MILESTONE.md) |
| [`journal/`](journal/) | Nhật ký dev theo ngày (`DEVELOPMENT-LOG-*`, worklog) |
| [`protocol/`](protocol/) | Baseline mạng Takumi, compatibility matrix, dispatch index |
| [`migration/`](migration/) | Migration OpenMU, data zip merge, manifest tracker |
| [`game-spec/`](game-spec/) | Spec data/season/SQL backlog, skill hotkey persistence |
| [`manifests/`](manifests/) | Snapshot file list (server source, gamedata, config) |
| [`architecture/`](architecture/) | Luồng persistence client ↔ server |
| [`ops/`](ops/) | Port plan, MU server trên Mac/VMware |

## Điểm vào thường dùng

- **QA milestone (APK):** [`QA-MILESTONE.md`](QA-MILESTONE.md)
- **Android dev Mac:** [`android/ANDROID-DEV-MAC.md`](android/ANDROID-DEV-MAC.md)
- **Skill combat mobile:** [`android/MOBILE-SKILL-COMBAT-GUIDE.md`](android/MOBILE-SKILL-COMBAT-GUIDE.md)
- **Migration OpenMU:** [`migration/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](migration/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md)
