using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary><c>CGOptionDataRecv</c> — client saves skill hotkeys / QWER via <c>C1 F3 30</c>.</summary>
public static class CharacterOptionHandler
{
    public static bool TryHandle(
        GameRosterEntry player,
        string? accountId,
        byte[]? characterName10,
        ReadOnlySpan<byte> packet,
        Action? onRosterDirty,
        Action? onRosterSave)
    {
        if (!OptionDataWire602.TryFindSaveRequest(packet, out var configuration))
        {
            return false;
        }

        player.KeyConfiguration = CharacterKeyConfiguration.Normalize(configuration);
        onRosterDirty?.Invoke();
        onRosterSave?.Invoke();

        if (!string.IsNullOrEmpty(accountId) && characterName10 is { Length: > 0 })
        {
            var name = Encoding.ASCII.GetString(characterName10).TrimEnd('\0');
            if (!string.IsNullOrWhiteSpace(name))
            {
                CharacterRosterMirrorWriter.ScheduleKeyConfigurationUpsert(
                    accountId,
                    name,
                    player.KeyConfiguration);
            }
        }

        return true;
    }
}
