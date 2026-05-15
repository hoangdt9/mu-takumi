using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>PRECEIVE_JOIN_MAP_SERVER</c> (Takumi <c>WSclient.h</c>, <c>#pragma pack(1)</c>) — plain <c>C1</c>, 131 bytes.</summary>
public static class JoinMapServerWire602
{
    public const int PacketLength = 131;

    /// <summary>Override map id only (keeps default Lorencia XY/angle — use <see cref="Build(CharacterRosterWire, JoinMapSpawnWire)"/> for full control).</summary>
    public static byte[] Build(CharacterRosterWire r, byte mapIdOverride) =>
        Build(r, JoinMapSpawnWire.LorenciaDefault with { Map = mapIdOverride });

    /// <summary>Build join packet; stats are a minimal stub from <paramref name="r"/> (HP/MP &gt; 0 so the client can enter the world).</summary>
    public static byte[] Build(CharacterRosterWire r, JoinMapSpawnWire? spawn = null)
    {
        var s = spawn ?? JoinMapSpawnWire.LorenciaDefault;
        return Build(r, s);
    }

    /// <summary>Explicit spawn (M4 — no hard-coded Lorencia inside this overload).</summary>
    public static byte[] Build(CharacterRosterWire r, JoinMapSpawnWire spawn)
    {
        var stats = JoinMapStatWire.FromRoster(r);
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = 0xF3;
        p[3] = 0x03;
        p[4] = spawn.PositionX;
        p[5] = spawn.PositionY;
        p[6] = spawn.Map;
        p[7] = spawn.Angle;

        // Experience / next exp (8 + 8) — zero is acceptable for minimal QA; client shows 0%.
        p.AsSpan(8, 16).Clear();

        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(24), stats.LevelUpPoint);

        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(26), stats.Strength);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(28), stats.Dexterity);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(30), stats.Vitality);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(32), stats.Energy);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(34), stats.Life);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(36), stats.LifeMax);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(38), stats.Mana);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(40), stats.ManaMax);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(42), stats.Shield);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(44), stats.ShieldMax);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(46), stats.SkillMana);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(48), stats.SkillManaMax);

        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(50), stats.Gold);
        p[54] = stats.Pk;
        p[55] = stats.CtlCode;
        BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(56), stats.AddPoint);
        BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(58), stats.MaxAddPoint);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(60), stats.Charisma);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(62), stats.MinusPoint);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(64), stats.MaxMinusPoint);
        p[66] = stats.ExtInventory;

        // View* block (16 DWORDs) — mirror common server practice: echo combat stats for UI bars.
        var v = stats.ViewBlock;
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(67), v.ViewReset);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(71), v.ViewMasterReset);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(75), v.ViewPoint);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(79), v.ViewCurHp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(83), v.ViewMaxHp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(87), v.ViewCurMp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(91), v.ViewMaxMp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(95), v.ViewCurBp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(99), v.ViewMaxBp);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(103), v.ViewCurSd);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(107), v.ViewMaxSd);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(111), v.ViewStrength);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(115), v.ViewDexterity);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(119), v.ViewVitality);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(123), v.ViewEnergy);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(127), v.ViewLeadership);

        _ = r;
        return p;
    }
}

/// <summary>Minimal stat sheet for join (expand in M7 with DB-backed values).</summary>
file sealed class JoinMapStatWire
{
    internal readonly struct ViewDwordBlock
    {
        internal uint ViewReset { get; init; }
        internal uint ViewMasterReset { get; init; }
        internal uint ViewPoint { get; init; }
        internal uint ViewCurHp { get; init; }
        internal uint ViewMaxHp { get; init; }
        internal uint ViewCurMp { get; init; }
        internal uint ViewMaxMp { get; init; }
        internal uint ViewCurBp { get; init; }
        internal uint ViewMaxBp { get; init; }
        internal uint ViewCurSd { get; init; }
        internal uint ViewMaxSd { get; init; }
        internal uint ViewStrength { get; init; }
        internal uint ViewDexterity { get; init; }
        internal uint ViewVitality { get; init; }
        internal uint ViewEnergy { get; init; }
        internal uint ViewLeadership { get; init; }
    }

    internal ushort Strength { get; init; }
    internal ushort Dexterity { get; init; }
    internal ushort Vitality { get; init; }
    internal ushort Energy { get; init; }
    internal ushort Life { get; init; }
    internal ushort LifeMax { get; init; }
    internal ushort Mana { get; init; }
    internal ushort ManaMax { get; init; }
    internal ushort Shield { get; init; }
    internal ushort ShieldMax { get; init; }
    internal ushort SkillMana { get; init; }
    internal ushort SkillManaMax { get; init; }
    internal ushort LevelUpPoint { get; init; }
    internal uint Gold { get; init; }
    internal byte Pk { get; init; }
    internal byte CtlCode { get; init; }
    internal short AddPoint { get; init; }
    internal short MaxAddPoint { get; init; }
    internal ushort Charisma { get; init; }
    internal ushort MinusPoint { get; init; }
    internal ushort MaxMinusPoint { get; init; }
    internal byte ExtInventory { get; init; }
    internal ViewDwordBlock ViewBlock { get; init; }

    internal static JoinMapStatWire FromRoster(CharacterRosterWire r)
    {
        var lv = Math.Max((ushort)1, r.Level);
        var tier = (byte)Math.Min(7, r.ServerClass / 0x20);
        ushort str;
        ushort dex;
        ushort vit;
        ushort ene;
        switch (tier)
        {
            case 0: // DW + unknown low
                str = (ushort)(18 + lv);
                dex = (ushort)(18 + lv / 2);
                vit = (ushort)(15 + lv);
                ene = (ushort)(30 + lv);
                break;
            case 1: // DK
                str = (ushort)(28 + lv);
                dex = (ushort)(20 + lv / 2);
                vit = (ushort)(25 + lv);
                ene = (ushort)(10 + lv / 3);
                break;
            case 2: // ELF
                str = (ushort)(22 + lv / 2);
                dex = (ushort)(25 + lv);
                vit = (ushort)(20 + lv / 2);
                ene = (ushort)(15 + lv);
                break;
            case 3: // MG
                str = (ushort)(26 + lv);
                dex = (ushort)(26 + lv);
                vit = (ushort)(26 + lv);
                ene = (ushort)(26 + lv);
                break;
            default:
                str = (ushort)(20 + lv);
                dex = (ushort)(20 + lv);
                vit = (ushort)(20 + lv);
                ene = (ushort)(20 + lv);
                break;
        }

        var life = (ushort)Math.Min(ushort.MaxValue, 55 + vit * 2 + lv * 3);
        var v = r.Vitals;
        if (v.HasHp)
        {
            life = v.ClampU16(v.MaxHp);
            var cur = v.ClampU16(v.CurrentHp > 0 ? v.CurrentHp : v.MaxHp);
            if (cur > life)
            {
                cur = life;
            }

            life = cur;
        }

        ushort lifeMax;
        if (v.HasHp)
        {
            lifeMax = v.ClampU16(v.MaxHp);
        }
        else
        {
            lifeMax = (ushort)Math.Min(ushort.MaxValue, 55 + vit * 2 + lv * 3);
        }

        ushort manaCur;
        ushort manaMax;
        if (v.HasMp)
        {
            manaMax = v.ClampU16(v.MaxMp);
            manaCur = v.ClampU16(v.CurrentMp > 0 ? v.CurrentMp : v.MaxMp);
            if (manaCur > manaMax)
            {
                manaCur = manaMax;
            }
        }
        else
        {
            manaMax = (ushort)Math.Min(ushort.MaxValue, 40 + ene * 2 + lv * 2);
            manaCur = manaMax;
        }

        var gold = v.Zen > 0 ? v.ClampGold() : 0u;
        var view = new ViewDwordBlock
        {
            ViewCurHp = life,
            ViewMaxHp = lifeMax,
            ViewCurMp = manaCur,
            ViewMaxMp = manaMax,
            ViewCurSd = 0,
            ViewMaxSd = 0,
            ViewStrength = str,
            ViewDexterity = dex,
            ViewVitality = vit,
            ViewEnergy = ene,
            ViewLeadership = 0,
        };

        return new JoinMapStatWire
        {
            Strength = str,
            Dexterity = dex,
            Vitality = vit,
            Energy = ene,
            Life = life,
            LifeMax = lifeMax,
            Mana = manaCur,
            ManaMax = manaMax,
            Shield = 0,
            ShieldMax = 0,
            SkillMana = manaCur,
            SkillManaMax = manaMax,
            LevelUpPoint = 0,
            Gold = gold,
            Pk = 0,
            CtlCode = 0,
            AddPoint = 0,
            MaxAddPoint = 0,
            Charisma = 0,
            MinusPoint = 0,
            MaxMinusPoint = 0,
            ExtInventory = 0,
            ViewBlock = view,
        };
    }
}
