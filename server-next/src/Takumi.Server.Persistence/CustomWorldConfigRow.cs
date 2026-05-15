namespace Takumi.Server.Persistence;

public sealed class CustomWorldConfigRow
{
    public string ConfigKey { get; init; } = "";

    public string Format { get; init; } = "table";

    public string PayloadJson { get; init; } = "{}";
}
