# Manifests (Takumi ↔ OpenMU migration)

Các file `.txt` trong thư mục này là **danh sách đầy đủ** artifact server/data/config tại thời điểm tạo.

**Header chuẩn (checklist §1):** ba dòng đầu mỗi file nên có dạng `# Snapshot …`, `# commit: <sha>` (`git -C "$TAK" rev-parse HEAD`), và `# ---` trước nội dung list. Sau khi chạy script dưới đây, chèn các dòng đó tay hoặc bọc script để không mất lineage.

## Tái sinh nhanh (macOS / Linux)

Đặt `TAK` trỏ vào root repo `takumi`.

```bash
TAK="/Users/hoangmac/Project/MU/takumi"
OUT="$TAK/docs/manifests"
mkdir -p "$OUT"

# SERVER-SOURCE-MANIFEST (.cpp)
{
  echo "# TAKUMI-SERVER-SOURCE-MANIFEST — regenerate"
  echo "# Date: $(date -u +%Y-%m-%d)"
  for label in \
    "1.ConnectServer/ConnectServer##$TAK/Source/1.ConnectServer/ConnectServer" \
    "2.DataServer/DataServer##$TAK/Source/2.DataServer/DataServer" \
    "3.JoinServer/JoinServer##$TAK/Source/3.JoinServer/JoinServer" \
    "4.GameServer/GameServer##$TAK/Source/4.GameServer/GameServer" \
    "6.GetMainInfo/GetMainInfo##$TAK/Source/6.GetMainInfo/GetMainInfo"; do
    title="${label%%##}"
    dir="${label##*##}"
    echo ""; echo "## $title *.cpp"
    find "$dir" -name '*.cpp' | sort
  done
} > "$OUT/TAKUMI-SERVER-SOURCE-MANIFEST.txt"

# SERVER-HEADERS-MANIFEST (.h / .hpp)
{
  echo "# TAKUMI-SERVER-HEADERS-MANIFEST — regenerate"
  echo "# Date: $(date -u +%Y-%m-%d)"
  echo ""
  find \
    "$TAK/Source/1.ConnectServer/ConnectServer" \
    "$TAK/Source/2.DataServer/DataServer" \
    "$TAK/Source/3.JoinServer/JoinServer" \
    "$TAK/Source/4.GameServer/GameServer" \
    "$TAK/Source/6.GetMainInfo/GetMainInfo" \
    \( -name '*.h' -o -name '*.hpp' \) | sort
} > "$OUT/TAKUMI-SERVER-HEADERS-MANIFEST.txt"

# MuServer game data (paths relative to MuServer/4.GameServer/Data)
{
  echo "# Relative to MuServer/4.GameServer/Data — regenerate"
  echo "# Date: $(date -u +%Y-%m-%d)"
  find "$TAK/MuServer/4.GameServer/Data" -type f | sed "s|.*/MuServer/4.GameServer/Data/||" | sort
} > "$OUT/TAKUMI-MUSERVER-GAMEDATA-FILES.txt"

# Config / bat / sql (prune bulk Data + logs + DatEditor)
{
  echo "# TAKUMI-MUSERVER-CONFIG-MANIFEST — regenerate"
  echo "# Date: $(date -u +%Y-%m-%d)"
  find "$TAK/MuServer" \
    \( -path '*/4.GameServer/Data/*' -prune \) -o \
    \( -path '*/6.DatEditor/*' -prune \) -o \
    \( -path '*/7.Log/*' -prune \) -o \
    \( -path '*/5.Antihack/LOG/*' -prune \) -o \
    -type f \( -iname '*.ini' -o -iname '*.txt' -o -iname '*.dat' -o -iname '*.sql' -o -iname '*.bat' \) -print | sort
} > "$OUT/TAKUMI-MUSERVER-CONFIG-MANIFEST.txt"
```

Đọc [`../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) để biết cách dùng từng manifest.
