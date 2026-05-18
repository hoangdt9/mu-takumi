using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Quest NPC classes (parity <c>gQuest.NpcTalk</c> subset — stub dialog).</summary>
public static class NpcQuestCatalog
{
    static readonly HashSet<int> QuestNpcClasses =
    [
        229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
        568, 569, 570,
    ];

    public static bool IsQuestNpc(int monsterClass) => QuestNpcClasses.Contains(monsterClass);

    public static byte DefaultQuestIndexForClass(int monsterClass) =>
        monsterClass switch
        {
            >= 229 and <= 249 => (byte)((monsterClass - 229) % 16),
            568 => 0,
            569 => 1,
            570 => 2,
            _ => 0,
        };

    public static byte DefaultQuestStateForClass(int monsterClass)
    {
        _ = monsterClass;
        var raw = Environment.GetEnvironmentVariable("TAKUMI_QUEST_NPC_DEFAULT_STATE")?.Trim();
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return 0;
        }

        return (byte)Math.Clamp(v, 0, 255);
    }

    /// <summary>Build 50-byte quest slot mask for <c>C1 A0</c> (active quest = 0, inactive = 0xFF).</summary>
    public static byte[] BuildQuestInfoMask(byte activeQuestIndex)
    {
        var mask = new byte[50];
        for (var i = 0; i < mask.Length; i++)
        {
            mask[i] = 0xFF;
        }

        if (activeQuestIndex < mask.Length)
        {
            mask[activeQuestIndex] = 0;
        }

        return mask;
    }
}
