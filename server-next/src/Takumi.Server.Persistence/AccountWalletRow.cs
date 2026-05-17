namespace Takumi.Server.Persistence;

/// <summary>Per-account wallet columns from <c>account</c> (warehouse zen + coin shop balances).</summary>
public sealed record AccountWalletRow(
    long WarehouseZen = 0,
    long WCoinC = 0,
    long WCoinP = 0,
    long GoblinPoint = 0);
