namespace Takumi.Server.Protocol;

/// <summary>Combat fields from client <c>Item_*.bmd</c> (layout parity <c>ITEM_ATTRIBUTE</c> in <c>_struct.h</c>).</summary>
public static class ItemCombatStatCatalog
{
    const int ItemAttributeBytes = 84;
    const int MaxItemCount = 8192;
    const int DamageMinOffset = 40;
    const int DamageMaxOffset = 41;
    const int SuccessfulBlockingOffset = 42;
    const int DefenseOffset = 43;
    const int MagicDefenseOffset = 44;
    const int AttackSpeedOffset = 28;
    const int DropLevelOffset = 32;
    const int MagicPowerOffset = 46;
    static readonly byte[] BuxCode = [0xFC, 0xCF, 0xAB];

    static readonly object Gate = new();
    static bool _ready;
    static byte[] _damageMin = [];
    static byte[] _damageMax = [];
    static byte[] _defense = [];
    static byte[] _magicDefense = [];
    static byte[] _blocking = [];
    static byte[] _attackSpeed = [];
    static byte[] _dropLevel = [];
    static byte[] _magicPower = [];

    public readonly record struct ItemCombatBase(
        byte DamageMin,
        byte DamageMax,
        byte Defense,
        byte MagicDefense,
        byte DefenseSuccessRate,
        byte AttackSpeed,
        byte DropLevel,
        byte MagicPower);

    public static void EnsureInitialized()
    {
        if (_ready)
        {
            return;
        }

        lock (Gate)
        {
            if (_ready)
            {
                return;
            }

            _damageMin = new byte[MaxItemCount];
            _damageMax = new byte[MaxItemCount];
            _defense = new byte[MaxItemCount];
            _magicDefense = new byte[MaxItemCount];
            _blocking = new byte[MaxItemCount];
            _attackSpeed = new byte[MaxItemCount];
            _dropLevel = new byte[MaxItemCount];
            _magicPower = new byte[MaxItemCount];

            var path = ResolveBmdPath();
            if (path is not null)
            {
                Load(path);
            }

            _ready = true;
        }
    }

    public static bool TryGet(int itemIndex, out ItemCombatBase stats)
    {
        EnsureInitialized();
        if ((uint)itemIndex >= MaxItemCount)
        {
            stats = default;
            return false;
        }

        stats = new ItemCombatBase(
            _damageMin[itemIndex],
            _damageMax[itemIndex],
            _defense[itemIndex],
            _magicDefense[itemIndex],
            _blocking[itemIndex],
            _attackSpeed[itemIndex],
            _dropLevel[itemIndex],
            _magicPower[itemIndex]);
        return true;
    }

    public static string? ResolveBmdPath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_ITEM_BMD_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        var locale = Environment.GetEnvironmentVariable("TAKUMI_ITEM_BMD_LOCALE")?.Trim();
        if (string.IsNullOrEmpty(locale))
        {
            locale = "Eng";
        }

        var roots = new[]
        {
            Environment.GetEnvironmentVariable("TAKUMI_CLIENT_DATA_ROOT")?.Trim(),
            "/att-data",
            Path.Combine(Environment.CurrentDirectory, "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "docker", "data-zip", "host", "Data"),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "..", "docker", "data-zip", "host", "Data"),
        };

        foreach (var root in roots)
        {
            if (string.IsNullOrEmpty(root))
            {
                continue;
            }

            var localDir = Path.Combine(root, "Local");
            if (!Directory.Exists(localDir))
            {
                continue;
            }

            foreach (var sub in Directory.EnumerateDirectories(localDir))
            {
                if (!Path.GetFileName(sub).Equals(locale, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                foreach (var file in Directory.EnumerateFiles(sub, "Item*.bmd", SearchOption.TopDirectoryOnly))
                {
                    return Path.GetFullPath(file);
                }
            }
        }

        return null;
    }

    static void Load(string path)
    {
        var raw = File.ReadAllBytes(path);
        if (raw.Length < ItemAttributeBytes * MaxItemCount)
        {
            return;
        }

        for (var i = 0; i < MaxItemCount; i++)
        {
            var o = i * ItemAttributeBytes;
            _damageMin[i] = DecodeByte(raw, o + DamageMinOffset);
            _damageMax[i] = DecodeByte(raw, o + DamageMaxOffset);
            _defense[i] = DecodeByte(raw, o + DefenseOffset);
            _magicDefense[i] = DecodeByte(raw, o + MagicDefenseOffset);
            _blocking[i] = DecodeByte(raw, o + SuccessfulBlockingOffset);
            _attackSpeed[i] = DecodeByte(raw, o + AttackSpeedOffset);
            _dropLevel[i] = DecodeByte(raw, o + DropLevelOffset);
            _magicPower[i] = DecodeByte(raw, o + MagicPowerOffset);
        }
    }

    static byte DecodeByte(byte[] file, int offset)
    {
        var b = file[offset];
        b ^= BuxCode[offset % 3];
        return b;
    }
}
