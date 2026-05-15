using System.Globalization;

namespace Takumi.Server.Game;

/// <summary>Per-connection limits after decrypt (DoS / accidental huge frames). Env: <c>TAKUMI_MAX_DECRYPTED_PACKET_BYTES</c>, <c>TAKUMI_MAX_PACKETS_PER_SECOND</c>.</summary>
public static class DecryptedPacketSafety602
{
    public const int DefaultMaxDecryptedPacketBytes = 12288;

    /// <summary>Clamp 512..65535; default <see cref="DefaultMaxDecryptedPacketBytes"/>.</summary>
    public static int ParseMaxDecryptedPacketBytes()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MAX_DECRYPTED_PACKET_BYTES")?.Trim();
        if (string.IsNullOrEmpty(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return DefaultMaxDecryptedPacketBytes;
        }

        return Math.Clamp(v, 512, 65535);
    }

    /// <summary>0 = disabled. Otherwise clamp 1..5000 packets per rolling 1s window.</summary>
    public static int ParseMaxPacketsPerSecond()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MAX_PACKETS_PER_SECOND")?.Trim();
        if (string.IsNullOrEmpty(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return 0;
        }

        return Math.Clamp(v, 0, 5000);
    }

    /// <summary>0 = disabled; else clamp 1..500. Limits outbound <c>0x15</c>/<c>0x18</c> peer broadcasts per session.</summary>
    public static int ParseMaxPresenceBroadcastsPerSecond()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND")?.Trim();
        if (string.IsNullOrEmpty(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return 0;
        }

        return Math.Clamp(v, 0, 500);
    }
}

/// <summary>Fixed 1s sliding window; intended under per-connection <see cref="System.Threading.SemaphoreSlim"/> serialization.</summary>
public sealed class DecryptedPacketRateGate
{
    private readonly int _maxPerSecond;
    private readonly Queue<DateTimeOffset> _hits = new();

    public DecryptedPacketRateGate(int maxPerSecond) => this._maxPerSecond = maxPerSecond;

    public bool TryAllow(DateTimeOffset nowUtc)
    {
        if (this._maxPerSecond <= 0)
        {
            return true;
        }

        var cutoff = nowUtc.AddSeconds(-1);
        while (this._hits.Count > 0 && this._hits.Peek() < cutoff)
        {
            this._hits.Dequeue();
        }

        if (this._hits.Count >= this._maxPerSecond)
        {
            return false;
        }

        this._hits.Enqueue(nowUtc);
        return true;
    }
}
