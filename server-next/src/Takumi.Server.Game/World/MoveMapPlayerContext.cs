namespace Takumi.Server.Game.World;

/// <summary>Session + roster fields for <c>CMove::Move</c> validation (M8).</summary>
public readonly record struct MoveMapPlayerContext(
    int Level,
    long Zen,
    int Reset,
    int AccountLevel,
    byte PkLevel,
    byte GensFamily,
    byte CurrentMapId,
    byte ServerClass,
    Guid PresenceSessionId,
    bool ShopWarehouseOrTradeOpen,
    bool IsDead,
    bool TeleportInProgress);
