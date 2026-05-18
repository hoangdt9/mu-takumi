using System.Text;
using Takumi.Server.Game;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterOpsTests
{
    [Fact]
    public void ValidateCreate_rejects_roster_full()
    {
        var roster = new List<GameRosterEntry>();
        for (var i = 0; i < CharacterRosterErrorCodes.MaxCharactersPerAccount; i++)
        {
            roster.Add(MakeEntry($"hero{i:000}"));
        }

        var name = Encoding.ASCII.GetBytes("newhero00");
        var reason = CharacterRosterOps.ValidateCreate(roster, name, packedClass: 0x10);
        Assert.Equal(CharacterRosterOps.CreateRejectReason.RosterFull, reason);
        Assert.Equal(CharacterRosterErrorCodes.CreateFailed, CharacterRosterOps.MapCreateRejectToWire(reason));
    }

    [Fact]
    public void ValidateCreate_rejects_duplicate_name()
    {
        var roster = new List<GameRosterEntry> { MakeEntry("testhero01") };
        var name = Encoding.ASCII.GetBytes("testhero01");
        var reason = CharacterRosterOps.ValidateCreate(roster, name, packedClass: 0x10);
        Assert.Equal(CharacterRosterOps.CreateRejectReason.DuplicateName, reason);
    }

    [Fact]
    public void ValidateDelete_accepts_zero_resident()
    {
        var roster = new List<GameRosterEntry> { MakeEntry("testhero01") };
        var name = Encoding.ASCII.GetBytes("testhero01");
        var resident = new byte[20];
        var reason = CharacterRosterOps.ValidateDelete(roster, name, resident);
        Assert.Equal(CharacterRosterOps.DeleteRejectReason.None, reason);
    }

    [Fact]
    public void ValidateDelete_rejects_nonzero_resident()
    {
        var roster = new List<GameRosterEntry> { MakeEntry("testhero01") };
        var name = Encoding.ASCII.GetBytes("testhero01");
        var resident = new byte[20];
        resident[0] = 1;
        var reason = CharacterRosterOps.ValidateDelete(roster, name, resident);
        Assert.Equal(CharacterRosterOps.DeleteRejectReason.ResidentWrong, reason);
    }

    static GameRosterEntry MakeEntry(string name)
    {
        var nm = new byte[10];
        Encoding.ASCII.GetBytes(name).AsSpan(0, Math.Min(10, name.Length)).CopyTo(nm);
        return new GameRosterEntry { Name10 = nm, ServerClass = 0x20, Level = 1 };
    }
}
