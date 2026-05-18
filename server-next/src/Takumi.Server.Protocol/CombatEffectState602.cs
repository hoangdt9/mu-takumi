namespace Takumi.Server.Protocol;

/// <summary>Active skill buff contributions for HUD preview (parity <c>EFFECT_OPTION</c> subset).</summary>
public sealed class CombatEffectState602
{
    public int AddPhysiDamage;
    public int AddMagicDamage;
    public int AddCurseDamage;
    public int AddDefense;
    public int AddSwordPowerDamageRate;
    public int AddSwordPowerDefenseRate;
    public uint MulPhysiDamage;
    public uint MulMagicDamage;
    public uint MulCurseDamage;

    public static CombatEffectState602 Empty { get; } = new();
}
