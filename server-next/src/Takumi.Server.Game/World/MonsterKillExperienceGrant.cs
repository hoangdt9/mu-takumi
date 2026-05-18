using System.Text;
using Takumi.Server.Game;
using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game.World;

/// <summary>Kill EXP: party proportional split, top damage, or last hitter.</summary>
public static class MonsterKillExperienceGrant
{
    static readonly List<(int PlayerKey, int Damage)> ContributorScratch = new(8);

    public static bool TopDamageGrantEnabled() =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_COMBAT_TOP_DAMAGE_GRANT_EXP")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static bool PartyExpShareEnabled() =>
        string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_COMBAT_PARTY_EXP_SHARE")?.Trim(),
            "1",
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

        if (PartyExpShareEnabled() && TryGrantPartyShare(monster, expGain))
        {
            return;
        }

        GrantSingleRecipient(
            monster,
            expGain,
            attackerPlayer,
            attackerAccountId,
            presenceSessionId,
            onAttackerRosterDirty);
    }

    static bool TryGrantPartyShare(MapMonsterInstance monster, ushort expGain)
    {
        monster.CopyDamageContributors(ContributorScratch);
        if (ContributorScratch.Count == 0)
        {
            return false;
        }

        var totalDamage = monster.TotalRecordedDamage();
        if (totalDamage <= 0)
        {
            return false;
        }

        var granted = 0;
        foreach (var (playerKey, damage) in ContributorScratch)
        {
            var share = (int)((long)expGain * damage / totalDamage);
            if (share <= 0)
            {
                continue;
            }

            if (!TryResolveOnlineRecipient(playerKey, out var entry, out var accountId, out var roster, out var sessionId))
            {
                continue;
            }

            RosterExperienceCombat.GrantKillExperience(
                entry,
                share,
                accountId,
                () => GameRosterDisk.SaveEntries(accountId, roster));
            MonsterViewerRegistry.TryUpdateProgress(
                sessionId,
                entry.Experience,
                entry.Level,
                (uint)Math.Clamp(entry.Zen, 0, uint.MaxValue));
            granted++;
        }

        return granted > 0;
    }

    static void GrantSingleRecipient(
        MapMonsterInstance monster,
        ushort expGain,
        GameRosterEntry attackerPlayer,
        string? attackerAccountId,
        Guid? presenceSessionId,
        Action? onAttackerRosterDirty)
    {
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
            && TryResolveOnlineRecipient(topKey, out var topEntry, out var topAccount, out topRoster, out var topSessionId))
        {
            recipient = topEntry;
            accountId = topAccount;
            progressSessionId = topSessionId;
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

    static bool TryResolveOnlineRecipient(
        int playerObjectKey,
        out GameRosterEntry entry,
        out string accountId,
        out List<GameRosterEntry> roster,
        out Guid sessionId)
    {
        entry = null!;
        roster = new List<GameRosterEntry>();
        accountId = string.Empty;
        sessionId = Guid.Empty;

        if (!MonsterViewerRegistry.TryGetByPlayerKey(playerObjectKey, out var viewer))
        {
            return false;
        }

        sessionId = viewer.SessionId;
        if (!TryLoadRosterCharacter(viewer, out entry, out accountId, out roster))
        {
            return false;
        }

        return true;
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
