using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>Maps <see cref="CharacterRosterRow"/> ↔ <see cref="GameRosterEntry"/> (M7 full field parity).</summary>
public static class CharacterRosterEntryMapping
{
    public static GameRosterEntry ToGameEntry(CharacterRosterRow row)
    {
        var nm = new byte[10];
        var enc = Encoding.ASCII.GetBytes(row.Name.Trim());
        Buffer.BlockCopy(enc, 0, nm, 0, Math.Min(10, enc.Length));
        var entry = new GameRosterEntry
        {
            Name10 = nm,
            ServerClass = row.ServerClass,
            Level = row.Level,
            Experience = (uint)Math.Clamp(row.Experience, 0, uint.MaxValue),
            MapId = row.MapId,
            PosX = row.PosX,
            PosY = row.PosY,
            Angle = row.Angle,
            CurrentHp = row.CurrentHp,
            MaxHp = row.MaxHp,
            CurrentMp = row.CurrentMp,
            MaxMp = row.MaxMp,
            Zen = row.Zen,
            CurrentShield = row.CurrentShield,
            MaxShield = row.MaxShield,
            Strength = (ushort)Math.Clamp(row.Strength, 0, ushort.MaxValue),
            Dexterity = (ushort)Math.Clamp(row.Dexterity, 0, ushort.MaxValue),
            Vitality = (ushort)Math.Clamp(row.Vitality, 0, ushort.MaxValue),
            Energy = (ushort)Math.Clamp(row.Energy, 0, ushort.MaxValue),
            Leadership = (ushort)Math.Clamp(row.Leadership, 0, ushort.MaxValue),
            LevelUpPoint = (ushort)Math.Clamp(row.LevelUpPoint, 0, ushort.MaxValue),
            CurrentBp = row.CurrentBp,
            MaxBp = row.MaxBp,
            KeyConfiguration = row.KeyConfiguration is { Length: > 0 }
                ? CharacterKeyConfiguration.Normalize(row.KeyConfiguration)
                : CharacterKeyConfiguration.CreateDefault(),
        };
        GameRosterDisk.ApplyLegacySpawnIfUnset(entry);
        return entry;
    }

    public static void ApplyDbOverlay(GameRosterEntry e, CharacterRosterRow d)
    {
        e.MapId = d.MapId;
        e.PosX = d.PosX;
        e.PosY = d.PosY;
        e.Angle = d.Angle;
        e.Level = d.Level;
        e.Experience = (uint)Math.Clamp(d.Experience, 0, uint.MaxValue);
        e.ServerClass = d.ServerClass;
        e.CurrentHp = d.CurrentHp;
        e.MaxHp = d.MaxHp;
        e.CurrentMp = d.CurrentMp;
        e.MaxMp = d.MaxMp;
        e.Zen = d.Zen;
        e.CurrentShield = d.CurrentShield;
        e.MaxShield = d.MaxShield;
        e.Strength = (ushort)Math.Clamp(d.Strength, 0, ushort.MaxValue);
        e.Dexterity = (ushort)Math.Clamp(d.Dexterity, 0, ushort.MaxValue);
        e.Vitality = (ushort)Math.Clamp(d.Vitality, 0, ushort.MaxValue);
        e.Energy = (ushort)Math.Clamp(d.Energy, 0, ushort.MaxValue);
        e.Leadership = (ushort)Math.Clamp(d.Leadership, 0, ushort.MaxValue);
        e.LevelUpPoint = (ushort)Math.Clamp(d.LevelUpPoint, 0, ushort.MaxValue);
        e.CurrentBp = d.CurrentBp;
        e.MaxBp = d.MaxBp;
        if (d.KeyConfiguration is { Length: > 0 })
        {
            e.KeyConfiguration = CharacterKeyConfiguration.Normalize(d.KeyConfiguration);
        }
    }
}
