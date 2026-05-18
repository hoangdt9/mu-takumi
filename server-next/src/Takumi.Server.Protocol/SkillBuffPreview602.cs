namespace Takumi.Server.Protocol;

/// <summary>Registers self/party buff magnitudes for <c>F3 E1</c> preview (parity common buff skills).</summary>
public static class SkillBuffPreview602
{
    public const ushort SkillGreaterDefense = 27;
    public const ushort SkillGreaterDamage = 28;
    public const ushort SkillGreaterLife = 48;
    public const ushort SkillGreaterMana = 69;
    public const ushort SkillGreaterCriticalDamage = 64;
    public const ushort SkillSwordPower = 218;
    public const ushort SkillMagicCircle = 233;

    public static bool TryApplyBuff(
        ushort skillId,
        byte serverClass,
        CharacterSheetStats sheet,
        CharacterCombatAccumulator acc,
        CombatEffectState602 effects)
    {
        var cls = CharacterSheetCalculator.ClassIndex(serverClass);
        var energy = sheet.Energy + (ushort)Math.Min(ushort.MaxValue - sheet.Energy, acc.AddEnergy);

        switch (skillId)
        {
            case SkillGreaterDamage:
            {
                var value = GreaterDamageValue(energy, cls);
                effects.AddPhysiDamage += value;
                effects.AddMagicDamage += value;
                effects.AddCurseDamage += value;
                return true;
            }

            case SkillGreaterDefense:
            {
                effects.AddDefense += GreaterDefenseValue(energy, cls);
                return true;
            }

            case SkillGreaterCriticalDamage:
                effects.AddPhysiDamage += 30;
                return true;

            case SkillGreaterLife:
                return true;

            case SkillGreaterMana:
                return true;

            case SkillSwordPower:
            {
                var rate = (int)Math.Min(SwordPowerMaxRate, (energy + acc.AddEnergy) / SwordPowerConstA);
                effects.AddSwordPowerDamageRate += rate;
                effects.AddSwordPowerDefenseRate += rate;
                return true;
            }

            case SkillMagicCircle:
                effects.MulMagicDamage += 10;
                return true;

            default:
                return false;
        }

        static int GreaterDamageValue(int ene, int cls) =>
            ScaleByClass(3 + (ene / 7), cls, defaultRate: 100);

        static int GreaterDefenseValue(int ene, int cls) =>
            ScaleByClass((2 + (ene / 8)) * 2, cls, defaultRate: 100);

        static int ScaleByClass(int value, int cls, int defaultRate) =>
            (value * ClassRate(cls, defaultRate)) / 100;
    }

    public static void ApplyToPreview(
        ref CharacterCombatPreview602.Preview preview,
        CombatEffectState602 effects,
        CharacterSheetStats sheet,
        CharacterCombatAccumulator acc)
    {
        var physMin = (int)preview.PhysiDamageMin + effects.AddPhysiDamage;
        var physMax = (int)preview.PhysiDamageMax + effects.AddPhysiDamage;
        var magMin = (int)preview.MagicDamageMin + effects.AddMagicDamage;
        var magMax = (int)preview.MagicDamageMax + effects.AddMagicDamage;
        var defense = (int)preview.Defense + effects.AddDefense;

        if (effects.AddSwordPowerDamageRate > 0)
        {
            var str = sheet.Strength + acc.AddStrength;
            var dex = sheet.Dexterity + acc.AddDexterity;
            var rate = Math.Min(SwordPowerMaxRate, effects.AddSwordPowerDamageRate);
            var addMin = (int)((str + dex) / SupPhysiDamageMinConstA * rate / 100);
            var addMax = (int)((str + dex) / SupPhysiDamageMaxConstA * rate / 100);
            physMin += addMin;
            physMax += addMax;
            magMin += addMin;
            magMax += addMax;
        }

        if (effects.AddSwordPowerDefenseRate > 0)
        {
            var dex = sheet.Dexterity + acc.AddDexterity;
            var sub = (int)(dex / SuDefenseConstA * effects.AddSwordPowerDefenseRate / 100);
            defense = Math.Max(0, defense - sub);
        }

        preview = preview with
        {
            PhysiDamageMin = (uint)Math.Max(0, physMin),
            PhysiDamageMax = (uint)Math.Max(0, physMax),
            MagicDamageMin = (uint)Math.Max(0, magMin),
            MagicDamageMax = (uint)Math.Max(0, magMax),
            Defense = (uint)Math.Max(0, defense),
        };
    }

    static int ClassRate(int cls, int defaultRate) => cls switch
    {
        2 => 100,
        _ => 80,
    };

    const int SwordPowerConstA = 30;
    const int SwordPowerMaxRate = 40;
    const int SupPhysiDamageMinConstA = 8;
    const int SupPhysiDamageMaxConstA = 4;
    const int SuDefenseConstA = 3;
}
