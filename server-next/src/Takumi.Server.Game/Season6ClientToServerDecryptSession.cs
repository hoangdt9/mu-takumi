using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;

namespace Takumi.Server.Game;

/// <summary>
/// Takumi / Season6 game-login TCP: client→server uses OpenMU <see cref="PipelinedDecryptor"/> (SimpleModulus + XOR32).
/// Keys come from client <c>Data/Dec2.dat</c> — load via <see cref="Dec2ServerDecryptKeysLoader"/> (env <c>TAKUMI_DEC2_PATH</c>, <c>TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH</c>, …).
/// </summary>
public static class Season6ClientToServerDecryptSession
{
    /// <summary>Load server-side decrypt keys; falls back to OpenMU defaults when no Dec2 is found.</summary>
    public static (SimpleModulusKeys Keys, string SourceTag) LoadServerDecryptKeysFromDec2OrEnv() =>
        Dec2ServerDecryptKeysLoader.Load();
}
