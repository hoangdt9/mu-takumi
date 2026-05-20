namespace Takumi.Server.Protocol;

/// <summary>Class defaults + max HP/MP/SD/BP (parity <c>DefaultClassInfo</c> + <c>ObjectManager::CharacterCalc*</c>).</summary>
public static class CharacterSheetCalculator
{
    sealed record ClassDef(
        int Str,
        int Dex,
        int Vit,
        int Ene,
        int Lead,
        float MaxLife,
        float MaxMana,
        float LevelLife,
        float LevelMana,
        float VitToLife,
        float EneToMana,
        int LevelUpPerLevel);

    static readonly ClassDef[] Classes =
    [
        new(18, 18, 15, 30, 0, 60, 60, 1, 2, 1, 2, 5),   // DW
        new(28, 20, 25, 10, 0, 110, 20, 2, 0.5f, 3, 1, 5), // DK
        new(22, 25, 20, 15, 0, 80, 80, 1, 1.5f, 2, 2, 5),  // FE
        new(26, 26, 26, 26, 0, 110, 60, 1, 1, 2, 2, 7),    // MG
        new(26, 20, 20, 15, 25, 90, 40, 1.5f, 1, 2, 2, 7), // DL
        new(21, 21, 18, 23, 0, 70, 70, 1, 1, 2, 2, 5),     // SU
        new(32, 27, 25, 20, 0, 120, 30, 2, 1, 3, 1, 7),    // RF
    ];

    public static int ClassIndex(byte serverClass) => Math.Clamp(serverClass / 0x20, 0, Classes.Length - 1);

    public static int LevelUpPointsPerLevel(byte serverClass) => Classes[ClassIndex(serverClass)].LevelUpPerLevel;

    public static CharacterSheetStats DefaultSheet(byte serverClass, ushort level)
    {
        var c = Classes[ClassIndex(serverClass)];
        var lv = Math.Max((ushort)1, level);
        var points = Math.Max(0, (lv - 1) * c.LevelUpPerLevel);
        return CharacterSheetStats.FromInts(c.Str, c.Dex, c.Vit, c.Ene, c.Lead, points);
    }

    public static CharacterSheetStats ResolveSheet(byte serverClass, ushort level, CharacterSheetStats stored) =>
        stored.HasBaseStats ? stored : DefaultSheet(serverClass, level);

    public static CharacterComputedVitals ComputeMaxVitals(byte serverClass, ushort level, CharacterSheetStats sheet)
    {
        var s = ResolveSheet(serverClass, level, sheet);
        var c = Classes[ClassIndex(serverClass)];
        var lv = Math.Max((ushort)1, level);

        var maxLife = c.MaxLife + c.LevelLife * (lv - 1);
        maxLife += (s.Vitality - c.Vit) * c.VitToLife;
        var maxMana = c.MaxMana + c.LevelMana * (lv - 1);
        maxMana += (s.Energy - c.Ene) * c.EneToMana;

        var lifeMax = (ushort)Math.Clamp((int)maxLife, 1, ushort.MaxValue);
        var manaMax = (ushort)Math.Clamp((int)maxMana, 1, ushort.MaxValue);
        var maxBp = (ushort)Math.Clamp(ComputeMaxBp(serverClass, s), 0, ushort.MaxValue);
        var maxSd = (ushort)Math.Clamp(ComputeMaxShield(s, lv), 0, ushort.MaxValue);

        return new CharacterComputedVitals
        {
            LifeMax = lifeMax,
            Life = lifeMax,
            ManaMax = manaMax,
            Mana = manaMax,
            ShieldMax = maxSd,
            Shield = maxSd,
            SkillManaMax = maxBp,
            SkillMana = maxBp,
        };
    }

    /// <summary>Merge DB vitals; heal to full on join unless <paramref name="keepPartial"/>.</summary>
    public static CharacterRosterVitals MergeVitalsForJoin(
        CharacterRosterVitals persisted,
        CharacterComputedVitals computed,
        bool keepPartial = false)
    {
        // Always recompute max vitals from class + stats (parity ObjectManager::CharacterCalcAttribute).
        var maxHp = computed.LifeMax;
        var maxMp = computed.ManaMax;
        var maxSd = computed.ShieldMax;

        // Heal when current is unset/0 (fixes corrupt 0 HP UI); keep partial when DB has current > 0.
        var curHp = persisted.HasHp && persisted.CurrentHp > 0
            ? Math.Min(persisted.CurrentHp, maxHp)
            : maxHp;
        var curMp = persisted.HasMp && persisted.CurrentMp > 0
            ? Math.Min(persisted.CurrentMp, maxMp)
            : maxMp;
        var curSd = persisted.HasShield && persisted.CurrentShield > 0
            ? Math.Min(persisted.CurrentShield, maxSd)
            : maxSd;
        if (keepPartial)
        {
            // Explicit env: never bump to full when any current value was stored (even if below max).
            if (persisted.HasHp && persisted.CurrentHp > 0)
            {
                curHp = persisted.CurrentHp;
            }

            if (persisted.HasMp && persisted.CurrentMp > 0)
            {
                curMp = persisted.CurrentMp;
            }

            if (persisted.HasShield && persisted.CurrentShield > 0)
            {
                curSd = persisted.CurrentShield;
            }
        }

        if (curHp > maxHp)
        {
            curHp = maxHp;
        }

        if (curMp > maxMp)
        {
            curMp = maxMp;
        }

        if (curSd > maxSd)
        {
            curSd = maxSd;
        }

        return CharacterRosterVitals.FromInts(curHp, maxHp, curMp, maxMp, persisted.Zen, curSd, maxSd);
    }

    public static bool TryAddStatPoint(ref CharacterSheetStats sheet, byte statType, out ushort maxLifeOrMana)
    {
        maxLifeOrMana = 0;
        if (sheet.LevelUpPoint == 0)
        {
            return false;
        }

        switch (statType)
        {
            case 0:
                sheet = sheet with
                {
                    Strength = (ushort)(sheet.Strength + 1),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - 1),
                };
                return true;
            case 1:
                sheet = sheet with
                {
                    Dexterity = (ushort)(sheet.Dexterity + 1),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - 1),
                };
                return true;
            case 2:
                sheet = sheet with
                {
                    Vitality = (ushort)(sheet.Vitality + 1),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - 1),
                };
                return true;
            case 3:
                sheet = sheet with
                {
                    Energy = (ushort)(sheet.Energy + 1),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - 1),
                };
                return true;
            case 4:
                sheet = sheet with
                {
                    Leadership = (ushort)(sheet.Leadership + 1),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - 1),
                };
                return true;
            default:
                return false;
        }
    }

    /// <summary>Apply up to <paramref name="count"/> points in one update (capped by remaining level-up points and ushort stat max).</summary>
    /// <param name="serverClass">Wire class from roster; Leadership (stat 4) is only meaningful for DL (index 4).</param>
    public static int TryAddStatPoints(ref CharacterSheetStats sheet, byte statType, int count, byte serverClass, out ushort maxLifeOrMana)
    {
        maxLifeOrMana = 0;
        if (statType == 4 && ClassIndex(serverClass) != 4)
        {
            return 0;
        }

        if (count <= 0 || sheet.LevelUpPoint == 0)
        {
            return 0;
        }

        var toApply = Math.Min(count, (int)sheet.LevelUpPoint);
        var statRoom = StatRoomForAdd(sheet, statType);
        if (statRoom <= 0)
        {
            return 0;
        }

        if (toApply > statRoom)
        {
            toApply = statRoom;
        }

        switch (statType)
        {
            case 0:
                sheet = sheet with
                {
                    Strength = (ushort)(sheet.Strength + toApply),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - toApply),
                };
                return toApply;
            case 1:
                sheet = sheet with
                {
                    Dexterity = (ushort)(sheet.Dexterity + toApply),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - toApply),
                };
                return toApply;
            case 2:
                sheet = sheet with
                {
                    Vitality = (ushort)(sheet.Vitality + toApply),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - toApply),
                };
                return toApply;
            case 3:
                sheet = sheet with
                {
                    Energy = (ushort)(sheet.Energy + toApply),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - toApply),
                };
                return toApply;
            case 4:
                sheet = sheet with
                {
                    Leadership = (ushort)(sheet.Leadership + toApply),
                    LevelUpPoint = (ushort)(sheet.LevelUpPoint - toApply),
                };
                return toApply;
            default:
                return 0;
        }
    }

    static int StatRoomForAdd(CharacterSheetStats sheet, byte statType) =>
        statType switch
        {
            0 => ushort.MaxValue - sheet.Strength,
            1 => ushort.MaxValue - sheet.Dexterity,
            2 => ushort.MaxValue - sheet.Vitality,
            3 => ushort.MaxValue - sheet.Energy,
            4 => ushort.MaxValue - sheet.Leadership,
            _ => 0,
        };

    public static ushort MaxAfterStatAdd(byte serverClass, ushort level, CharacterSheetStats sheet, byte statType)
    {
        var computed = ComputeMaxVitals(serverClass, level, sheet);
        return statType switch
        {
            2 => computed.LifeMax,
            3 => computed.ManaMax,
            _ => 0,
        };
    }

    static int ComputeMaxBp(byte serverClass, CharacterSheetStats s)
    {
        var str = s.Strength;
        var dex = s.Dexterity;
        var vit = s.Vitality;
        var ene = s.Energy;
        var lead = s.Leadership;
        return ClassIndex(serverClass) switch
        {
            0 => (int)(str * 0.20 + dex * 0.40 + vit * 0.30 + ene * 0.20),
            1 => (int)(str * 0.15 + dex * 0.20 + vit * 0.30 + ene * 1.00),
            2 => (int)(str * 0.30 + dex * 0.20 + vit * 0.30 + ene * 0.20),
            3 => (int)(str * 0.20 + dex * 0.25 + vit * 0.30 + ene * 0.15),
            4 => (int)(str * 0.30 + dex * 0.20 + vit * 0.10 + ene * 0.15 + lead * 0.30),
            5 => (int)(str * 0.20 + dex * 0.25 + vit * 0.30 + ene * 0.15),
            _ => (int)(str * 0.15 + dex * 0.20 + vit * 0.30 + ene * 1.00),
        };
    }

    static int ComputeMaxShield(CharacterSheetStats s, ushort level)
    {
        var value = s.Strength + s.Dexterity + s.Vitality + s.Energy + s.Leadership;
        const int shieldGaugeConstA = 10;
        const int shieldGaugeConstB = 20;
        return (value * shieldGaugeConstA / 10) + (level * level / shieldGaugeConstB);
    }
}
