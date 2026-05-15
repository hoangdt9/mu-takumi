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
}
