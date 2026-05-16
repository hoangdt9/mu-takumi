using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

public static class GameRosterSheetExtensions
{
    public static CharacterSheetStats ResolveSheet(this GameRosterEntry e) =>
        CharacterSheetCalculator.ResolveSheet(
            e.ServerClass,
            e.Level,
            CharacterSheetStats.FromInts(
                e.Strength,
                e.Dexterity,
                e.Vitality,
                e.Energy,
                e.Leadership,
                e.LevelUpPoint));

    public static void ApplySheet(this GameRosterEntry e, CharacterSheetStats sheet)
    {
        e.Strength = sheet.Strength;
        e.Dexterity = sheet.Dexterity;
        e.Vitality = sheet.Vitality;
        e.Energy = sheet.Energy;
        e.Leadership = sheet.Leadership;
        e.LevelUpPoint = sheet.LevelUpPoint;
    }

    public static CharacterRosterWire ToWireWithSheet(this GameRosterEntry e)
    {
        var sheet = e.ResolveSheet();
        var keepPartial = string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_JOIN_KEEP_VITALS")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase);
        var vitals = CharacterSheetCalculator.MergeVitalsForJoin(
            CharacterRosterVitals.FromInts(
                e.CurrentHp,
                e.MaxHp,
                e.CurrentMp,
                e.MaxMp,
                e.Zen,
                e.CurrentShield,
                e.MaxShield),
            CharacterSheetCalculator.ComputeMaxVitals(e.ServerClass, e.Level, sheet),
            keepPartial);
        return new CharacterRosterWire(e.Name10, e.ServerClass, e.Level, vitals, sheet, e.Experience);
    }

    public static (int CurrentBp, int MaxBp) ResolveBpForSync(this GameRosterEntry e)
    {
        if (e.MaxBp > 0)
        {
            return (e.CurrentBp > 0 ? e.CurrentBp : e.MaxBp, e.MaxBp);
        }

        var computed = CharacterSheetCalculator.ComputeMaxVitals(e.ServerClass, e.Level, e.ResolveSheet());
        return (computed.SkillMana, computed.SkillManaMax);
    }
}
