using Takumi.Server.Shared;

namespace Takumi.Server.Game;

/// <summary>Configuration for <see cref="TakumiGameHost"/>.</summary>
public sealed class GameHostOptions
{
    public int Port { get; init; } = ListenPortFallbacks.GameTcp;

    public int TcpBacklog { get; init; } = 256;

    public bool Verbose { get; init; }

    /// <summary>When true, sets SO_REUSEADDR on the listener (same opt-in as LegacyLoginHost via TAKUMI_REUSE_ADDR).</summary>
    public bool ReuseAddress { get; init; }

    public byte JoinResult { get; init; } = 1;

    public ushort JoinIndex { get; init; }

    /// <summary>Exactly 5 bytes (wire join version, e.g. ASCII "10405").</summary>
    public required byte[] JoinVersion5 { get; init; }

    /// <summary>When greater than zero, sends C1 03 71 periodically after accept.</summary>
    public TimeSpan KeepAliveInterval { get; init; } = TimeSpan.FromSeconds(25);
}
