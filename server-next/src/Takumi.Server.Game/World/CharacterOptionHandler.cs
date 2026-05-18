using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary><c>CGOptionDataRecv</c> — client saves skill hotkeys / QWER via <c>C1 F3 30</c>.</summary>
public static class CharacterOptionHandler
{
    public static bool TryHandle(
        GameRosterEntry player,
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
        return true;
    }
}
