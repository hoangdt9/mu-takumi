using System.Text;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>Create/delete validation shared by minimal game hosts (explicit client result codes).</summary>
public static class CharacterRosterOps
{
    public enum CreateRejectReason
    {
        None,
        NotLoggedIn,
        RosterFull,
        InvalidName,
        DuplicateName,
        InvalidClass,
    }

    public enum DeleteRejectReason
    {
        None,
        NotLoggedIn,
        NotFound,
        ResidentWrong,
    }

    public static CreateRejectReason ValidateCreate(
        IReadOnlyList<GameRosterEntry> roster,
        ReadOnlySpan<byte> name10,
        byte packedClass)
    {
        if (!TryNormalizeName(name10, out var name))
        {
            return CreateRejectReason.InvalidName;
        }

        if (roster.Count >= CharacterRosterErrorCodes.MaxCharactersPerAccount)
        {
            return CreateRejectReason.RosterFull;
        }

        foreach (var entry in roster)
        {
            if (NameMatches(entry.Name10, name))
            {
                return CreateRejectReason.DuplicateName;
            }
        }

        var serverClass = CharacterCreateWire602.MapPackedClassToServerProtocol(packedClass);
        if (!IsSupportedServerClass(serverClass))
        {
            return CreateRejectReason.InvalidClass;
        }

        return CreateRejectReason.None;
    }

    public static byte MapCreateRejectToWire(CreateRejectReason reason) =>
        reason switch
        {
            CreateRejectReason.DuplicateName => CharacterRosterErrorCodes.CreateDuplicateOrInvalidName,
            CreateRejectReason.InvalidName => CharacterRosterErrorCodes.CreateDuplicateOrInvalidName,
            CreateRejectReason.RosterFull => CharacterRosterErrorCodes.CreateFailed,
            CreateRejectReason.InvalidClass => CharacterRosterErrorCodes.CreateFailed,
            _ => CharacterRosterErrorCodes.CreateFailed,
        };

    public static DeleteRejectReason ValidateDelete(
        IReadOnlyList<GameRosterEntry> roster,
        ReadOnlySpan<byte> name10,
        ReadOnlySpan<byte> resident20)
    {
        if (!TryNormalizeName(name10, out var name))
        {
            return DeleteRejectReason.NotFound;
        }

        var found = roster.Any(e => NameMatches(e.Name10, name));
        if (!found)
        {
            return DeleteRejectReason.NotFound;
        }

        if (!ValidateDeleteResident(resident20))
        {
            return DeleteRejectReason.ResidentWrong;
        }

        return DeleteRejectReason.None;
    }

    public static byte MapDeleteRejectToWire(DeleteRejectReason reason) =>
        reason switch
        {
            DeleteRejectReason.ResidentWrong => CharacterRosterErrorCodes.DeleteResidentWrong,
            DeleteRejectReason.NotFound => CharacterRosterErrorCodes.DeleteResidentWrong,
            _ => CharacterRosterErrorCodes.DeleteResidentWrong,
        };

    /// <summary>
    /// Dev/QA: when client sends all-zero resident (no captcha UI), accept delete.
    /// Non-zero resident must match stored hash later (OPEN).
    /// </summary>
    public static bool ValidateDeleteResident(ReadOnlySpan<byte> resident20)
    {
        foreach (var b in resident20)
        {
            if (b != 0)
            {
                return false;
            }
        }

        return true;
    }

    static bool IsSupportedServerClass(byte serverClass) =>
        serverClass is 0x00 or 0x20 or 0x40 or 0x60 or 0x80 or 0xA0 or 0xC0;

    static bool TryNormalizeName(ReadOnlySpan<byte> name10, out string name)
    {
        name = Encoding.ASCII.GetString(name10).TrimEnd('\0', ' ');
        if (name.Length is < 4 or > 10)
        {
            return false;
        }

        foreach (var ch in name)
        {
            if (ch is < (char)32 or > (char)126)
            {
                return false;
            }
        }

        return true;
    }

    static bool NameMatches(byte[] entryName10, string name)
    {
        var entryName = Encoding.ASCII.GetString(entryName10).TrimEnd('\0', ' ');
        return string.Equals(entryName, name, StringComparison.OrdinalIgnoreCase);
    }
}
