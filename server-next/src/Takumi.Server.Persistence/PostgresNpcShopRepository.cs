using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

public sealed class PostgresNpcShopRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresNpcShopRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<(IReadOnlyList<NpcShopRow> Shops, IReadOnlyList<NpcShopItemRow> Items)> LoadAllAsync(CancellationToken ct = default)
    {
        var shops = new List<NpcShopRow>(64);
        var items = new List<NpcShopItemRow>(512);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);

        await using (var cmd = new NpgsqlCommand(
                         """
                         SELECT shop_index, monster_class, map_id, pos_x, pos_y, comment
                         FROM npc_shop
                         ORDER BY shop_index
                         """,
                         conn))
        {
            await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                shops.Add(
                    new NpcShopRow
                    {
                        ShopIndex = reader.GetInt32(0),
                        MonsterClass = reader.GetInt32(1),
                        MapId = reader.IsDBNull(2) ? null : reader.GetInt16(2),
                        PosX = reader.IsDBNull(3) ? null : reader.GetInt16(3),
                        PosY = reader.IsDBNull(4) ? null : reader.GetInt16(4),
                        Comment = reader.IsDBNull(5) ? null : reader.GetString(5),
                    });
            }
        }

        await using (var cmd = new NpgsqlCommand(
                         """
                         SELECT id, shop_index, slot, item_group, item_index, item_level, durability, skill, luck, option,
                                exc_opt, anc, joh, oex, socket1, socket2, socket3, socket4, socket5, item_name
                         FROM npc_shop_item
                         ORDER BY shop_index, slot
                         """,
                         conn))
        {
            await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                items.Add(
                    new NpcShopItemRow
                    {
                        Id = reader.GetInt32(0),
                        ShopIndex = reader.GetInt32(1),
                        Slot = reader.GetInt16(2),
                        ItemGroup = reader.GetInt16(3),
                        ItemIndex = reader.GetInt16(4),
                        ItemLevel = reader.GetInt16(5),
                        Durability = reader.GetInt16(6),
                        Skill = reader.GetInt16(7),
                        Luck = reader.GetInt16(8),
                        Option = reader.GetInt16(9),
                        ExcOpt = reader.GetInt16(10),
                        Anc = reader.GetInt16(11),
                        Joh = reader.GetInt16(12),
                        Oex = reader.GetInt16(13),
                        Socket1 = reader.GetInt16(14),
                        Socket2 = reader.GetInt16(15),
                        Socket3 = reader.GetInt16(16),
                        Socket4 = reader.GetInt16(17),
                        Socket5 = reader.GetInt16(18),
                        ItemName = reader.IsDBNull(19) ? null : reader.GetString(19),
                    });
            }
        }

        return (shops, items);
    }

    public async Task ReplaceAllAsync(
        IReadOnlyList<NpcShopRow> shops,
        IReadOnlyList<NpcShopItemRow> items,
        string? sourceFile,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);

        await using (var delItems = new NpgsqlCommand("DELETE FROM npc_shop_item", conn, tx))
        {
            await delItems.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await using (var delShops = new NpgsqlCommand("DELETE FROM npc_shop", conn, tx))
        {
            await delShops.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        if (shops.Count > 0)
        {
            await using var copy = await conn.BeginBinaryImportAsync(
                "COPY npc_shop (shop_index, monster_class, map_id, pos_x, pos_y, comment, source_file) FROM STDIN (FORMAT BINARY)",
                ct).ConfigureAwait(false);
            foreach (var s in shops)
            {
                await copy.StartRowAsync(ct).ConfigureAwait(false);
                await copy.WriteAsync(s.ShopIndex, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                await copy.WriteAsync(s.MonsterClass, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                if (s.MapId is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(s.MapId.Value, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                }

                if (s.PosX is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(s.PosX.Value, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                }

                if (s.PosY is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(s.PosY.Value, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                }

                if (s.Comment is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(s.Comment, NpgsqlDbType.Text, ct).ConfigureAwait(false);
                }

                if (sourceFile is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(sourceFile, NpgsqlDbType.Text, ct).ConfigureAwait(false);
                }
            }

            await copy.CompleteAsync(ct).ConfigureAwait(false);
        }

        if (items.Count > 0)
        {
            await using var copy = await conn.BeginBinaryImportAsync(
                """
                COPY npc_shop_item (shop_index, slot, item_group, item_index, item_level, durability, skill, luck, option,
                                    exc_opt, anc, joh, oex, socket1, socket2, socket3, socket4, socket5, item_name, source_file)
                FROM STDIN (FORMAT BINARY)
                """,
                ct).ConfigureAwait(false);
            foreach (var i in items)
            {
                await copy.StartRowAsync(ct).ConfigureAwait(false);
                await copy.WriteAsync(i.ShopIndex, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Slot, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.ItemGroup, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.ItemIndex, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.ItemLevel, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Durability, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Skill, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Luck, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Option, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.ExcOpt, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Anc, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Joh, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Oex, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Socket1, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Socket2, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Socket3, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Socket4, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(i.Socket5, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                if (i.ItemName is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(i.ItemName, NpgsqlDbType.Text, ct).ConfigureAwait(false);
                }

                if (sourceFile is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(sourceFile, NpgsqlDbType.Text, ct).ConfigureAwait(false);
                }
            }

            await copy.CompleteAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public ValueTask DisposeAsync() => this._dataSource.DisposeAsync();
}
