using System.Text;
using Takumi.Server.Game;
using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game.World;

/// <summary>Resolves kill EXP recipient (top damage vs last hitter).</summary>
public static class MonsterKillExperienceGrant
{
    public static bool TopDamageGrantEnabled() =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_COMBAT_TOP_DAMAGE_GRANT_EXP")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static void Grant(
        MapMonsterInstance monster,
        ushort expGain,
        GameRosterEntry? attackerPlayer,
        string? attackerAccountId,
        Guid? presenceSessionId,
        Action? onAttackerRosterDirty)
    {
        if (expGain == 0 || attackerPlayer is null)
        {
            return;
        }

        var recipient = attackerPlayer;
        var accountId = attackerAccountId;
        var dirty = onAttackerRosterDirty;
        Guid? progressSessionId = presenceSessionId;

        List<GameRosterEntry>? topRoster = null;
        if (TopDamageGrantEnabled()
            && monster.TryGetTopDamagePlayerKey(out var topKey, out _)
            && topKey > 0
            && presenceSessionId is { } sid
            && GameMapPresenceRegistry.TryGetObjectKey(sid, out var atkKey)
            && topKey != atkKey
            && MonsterViewerRegistry.TryGetByPlayerKey(topKey, out var topViewer)
            && TryLoadRosterCharacter(topViewer, out var topEntry, out var topAccount, out topRoster))
        {
            recipient = topEntry;
            accountId = topAccount;
            progressSessionId = topViewer.SessionId;
            dirty = () => GameRosterDisk.SaveEntries(topAccount, topRoster!);
        }

        RosterExperienceCombat.GrantKillExperience(recipient, expGain, accountId, dirty);
        if (progressSessionId is { } psid)
        {
            MonsterViewerRegistry.TryUpdateProgress(
                psid,
                recipient.Experience,
                recipient.Level,
                (uint)Math.Clamp(recipient.Zen, 0, uint.MaxValue));
        }
    }

    static bool TryLoadRosterCharacter(
        MonsterViewerSession viewer,
        out GameRosterEntry entry,
        out string accountId,
        out List<GameRosterEntry> roster)
    {
        entry = null!;
        roster = new List<GameRosterEntry>();
        accountId = viewer.AccountLogin?.Trim() ?? string.Empty;
        if (accountId.Length == 0 || string.IsNullOrWhiteSpace(viewer.CharacterName))
        {
            return false;
        }

        roster = GameRosterDisk.LoadEntries(accountId);
        var want = viewer.CharacterName.Trim();
        foreach (var e in roster)
        {
            var name = Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' ');
            if (!string.Equals(name, want, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            entry = e;
            return true;
        }

        return false;
    }
}
