namespace Takumi.Client.Protocol;

/// <summary>MU head bytes used by Takumi <c>Source/5.Main</c> (see server-next <c>M1-PROTOCOL-PARITY-MAP.md</c>).</summary>
public static class Season6HeadCodes
{
    public const byte JoinServer = 0xF1;
    public const byte Character = 0xF3;
    public const byte ConnectServer = 0xF4;
    public const byte MoveCharacter = 0xD4;
    public const byte MovePosition = 0x15;
}
