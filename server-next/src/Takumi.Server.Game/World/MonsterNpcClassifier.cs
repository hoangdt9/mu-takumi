namespace Takumi.Server.Game.World;

/// <summary>Parity <c>gObjSetMonster</c> NPC class ranges (Season 6).</summary>
public static class MonsterNpcClassifier
{
    public static bool IsNpc(int monsterClass)
    {
        if (monsterClass is >= 204 and <= 259)
        {
            return true;
        }

        if (monsterClass is >= 367 and <= 371)
        {
            return true;
        }

        if (monsterClass is >= 375 and <= 385)
        {
            return true;
        }

        if (monsterClass is 406 or 407 or 408 or 415 or 416 or 417 or 464 or 465 or 478 or 479 or 492 or 522)
        {
            return true;
        }

        if (monsterClass is >= 450 and <= 453)
        {
            return true;
        }

        if (monsterClass is >= 467 and <= 475)
        {
            return true;
        }

        if (monsterClass is >= 540 and <= 547)
        {
            return true;
        }

        if (monsterClass is >= 566 and <= 568)
        {
            return true;
        }

        if (monsterClass is >= 577 and <= 584)
        {
            return true;
        }

        return monsterClass is 604 or 643 or 651;
    }
}
