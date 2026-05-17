namespace Takumi.Server.Game.World;

/// <summary>Base stats from client <c>Item_*.bmd</c> for legacy buy-price parity.</summary>
internal static class ClientItemStatCatalog
{
    const int ItemAttributeBytes = 84;
    const int MaxItemCount = 8192;
    const int NameBytes = 30;
    const int TwoHandOffset = NameBytes;
    const int LevelOffset = NameBytes + 2;
    const int ValueOffset = NameBytes + 32;
    const int ZenOffset = NameBytes + 34;
    static readonly byte[] BuxCode = [0xFC, 0xCF, 0xAB];

    static readonly object Gate = new();
    static bool _ready;
    static ushort[] _dropLevel = [];
    static ushort[] _value = [];
    static int[] _zen = [];
    static byte[] _twoHand = [];

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

            _dropLevel = new ushort[MaxItemCount];
            _value = new ushort[MaxItemCount];
            _zen = new int[MaxItemCount];
            _twoHand = new byte[MaxItemCount];
            var path = ClientItemFootprintCatalog.ResolveBmdPath();
            if (path is not null)
            {
                Load(path);
            }

            _ready = true;
        }
    }

    public static bool TryGetDropLevel(int index, out int level)
    {
        EnsureInitialized();
        if ((uint)index >= MaxItemCount)
        {
            level = 0;
            return false;
        }

        level = _dropLevel[index];
        return level > 0;
    }

    public static bool TryGetValue(int index, out int value)
    {
        EnsureInitialized();
        if ((uint)index >= MaxItemCount)
        {
            value = 0;
            return false;
        }

        value = _value[index];
        return value > 0;
    }

    public static bool TryGetZen(int index, out int zen)
    {
        EnsureInitialized();
        if ((uint)index >= MaxItemCount)
        {
            zen = 0;
            return false;
        }

        zen = _zen[index];
        return zen > 0;
    }

    public static bool TryGetTwoHand(int index, out bool twoHand)
    {
        EnsureInitialized();
        if ((uint)index >= MaxItemCount)
        {
            twoHand = false;
            return false;
        }

        twoHand = _twoHand[index] != 0;
        return true;
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
            _twoHand[i] = DecodeUInt16(raw, o + TwoHandOffset) != 0 ? (byte)1 : (byte)0;
            _dropLevel[i] = DecodeUInt16(raw, o + LevelOffset);
            _value[i] = DecodeUInt16(raw, o + ValueOffset);
            _zen[i] = DecodeInt32(raw, o + ZenOffset);
        }
    }

    static byte DecodeByte(byte[] file, int offset)
    {
        var b = file[offset];
        b ^= BuxCode[offset % 3];
        return b;
    }

    static ushort DecodeUInt16(byte[] file, int offset)
    {
        var lo = DecodeByte(file, offset);
        var hi = DecodeByte(file, offset + 1);
        return (ushort)(lo | (hi << 8));
    }

    static int DecodeInt32(byte[] file, int offset)
    {
        var b0 = DecodeByte(file, offset);
        var b1 = DecodeByte(file, offset + 1);
        var b2 = DecodeByte(file, offset + 2);
        var b3 = DecodeByte(file, offset + 3);
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }
}
