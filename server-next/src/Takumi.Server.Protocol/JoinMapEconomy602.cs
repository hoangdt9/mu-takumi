using System.Globalization;

namespace Takumi.Server.Protocol;

/// <summary>
/// Join gold for client move UI (<c>SettingCanMoveMap</c> checks <c>CharacterMachine->Gold</c> vs MoveReq zen).
/// Roster zen 0 leaves all paid maps greyed; legacy GS still charges on warp.
/// </summary>
public static class JoinMapEconomy602
{
    /// <summary>Parity starter wallet when DB/json zen is unset (covers Lorencia 2000 and early Move.txt costs).</summary>
    public const uint DefaultMinZenWhenUnset = 10_000_000;

    public static uint ResolveJoinWireGold(long zen)
    {
        if (zen > 0)
        {
            return (uint)Math.Min(zen, uint.MaxValue);
        }

        var raw = Environment.GetEnvironmentVariable("TAKUMI_JOIN_MIN_ZEN");
        if (uint.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var min) && min > 0)
        {
            return min;
        }

        return DefaultMinZenWhenUnset;
    }
}
