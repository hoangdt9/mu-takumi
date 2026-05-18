namespace Takumi.Server.Protocol;

/// <summary>
/// Takumi client result bytes for <c>C1 F3 01</c> (create) and <c>C1 F3 02</c> (delete).
/// Parity <c>ReceiveCreateCharacter</c> / <c>ReceiveDeleteCharacter</c> in <c>WSclient.cpp</c>.
/// </summary>
public static class CharacterRosterErrorCodes
{
    public const byte CreateSuccess = 1;

    /// <summary>Generic failure — <c>RECEIVE_CREATE_CHARACTER_FAIL</c>.</summary>
    public const byte CreateFailed = 0;

    /// <summary>Duplicate / invalid name — <c>RECEIVE_CREATE_CHARACTER_FAIL2</c>.</summary>
    public const byte CreateDuplicateOrInvalidName = 2;

    public const byte DeleteSuccess = 1;

    /// <summary>Guild block — <c>MESSAGE_DELETE_CHARACTER_GUILDWARNING</c>.</summary>
    public const byte DeleteGuildBlocked = 0;

    /// <summary>Resident / captcha mismatch — <c>MESSAGE_STORAGE_RESIDENTWRONG</c>.</summary>
    public const byte DeleteResidentWrong = 2;

    /// <summary>Item lock — <c>MESSAGE_DELETE_CHARACTER_ITEM_BLOCK</c>.</summary>
    public const byte DeleteItemBlocked = 3;

    public const int MaxCharactersPerAccount = 5;
}
