namespace Takumi.Server.Game.World;

/// <summary>Parity <c>CKeyGenerater::GenerateKeyValue</c> / <c>CheckKeyValue</c> (client <c>KeyGenerater.cpp</c>).</summary>
public static class MoveMapKeyGenerator
{
    const int FilterCount = 10;

    static readonly int[][] Filter =
    {
        new[] { 321, 37531879, 8734, 32 },
        new[] { 873, 64374332, 3546, 87 },
        new[] { 537, 24798765, 5798, 32 },
        new[] { 654, 32498765, 3573, 73 },
        new[] { 546, 98465432, 6459, 12 },
        new[] { 987, 24654876, 5616, 54 },
        new[] { 357, 34599876, 8764, 98 },
        new[] { 665, 78641332, 6547, 54 },
        new[] { 813, 85132165, 8421, 98 },
        new[] { 454, 57684216, 6875, 45 },
    };

    public static uint GenerateKeyValue(uint keyValue)
    {
        var idx = (int)(keyValue % FilterCount);
        var f = Filter[idx];
        // Client uses DWORD math (unsigned division); signed C# `/` diverges (e.g. 8734/32).
        return keyValue * (uint)f[0] + (uint)f[1] - (uint)f[2] / (uint)f[3];
    }

    /// <summary>Returns false when <paramref name="receivedKey"/> does not match the next key from <paramref name="seed"/>.</summary>
    public static bool TryAcceptKey(ref uint seed, uint receivedKey)
    {
        var expected = GenerateKeyValue(seed);
        if (receivedKey != expected)
        {
            return false;
        }

        seed = expected;
        return true;
    }

    public static uint CreateSeed() => (uint)Random.Shared.Next(1, int.MaxValue);
}
