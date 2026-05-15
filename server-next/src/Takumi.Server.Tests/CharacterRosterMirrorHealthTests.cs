using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterMirrorHealthTests : IDisposable
{
    public CharacterRosterMirrorHealthTests() => CharacterRosterMirrorHealth.ResetCountersForTests();

    public void Dispose() => CharacterRosterMirrorHealth.ResetCountersForTests();

    [Fact]
    public void Merge_and_upsert_counters_increment_independently()
    {
        Assert.Equal(0, CharacterRosterMirrorHealth.MergeSuccessCount);
        CharacterRosterMirrorHealth.RecordMergeSuccess();
        CharacterRosterMirrorHealth.RecordMergeSuccess();
        CharacterRosterMirrorHealth.RecordMergeFail();
        CharacterRosterMirrorHealth.RecordUpsertSuccess();
        CharacterRosterMirrorHealth.RecordUpsertFail();
        CharacterRosterMirrorHealth.RecordUpsertFail();

        Assert.Equal(2, CharacterRosterMirrorHealth.MergeSuccessCount);
        Assert.Equal(1, CharacterRosterMirrorHealth.MergeFailCount);
        Assert.Equal(1, CharacterRosterMirrorHealth.UpsertSuccessCount);
        Assert.Equal(2, CharacterRosterMirrorHealth.UpsertFailCount);

        var snap = CharacterRosterMirrorHealth.FormatSnapshot();
        Assert.Contains("merge_ok=2", snap);
        Assert.Contains("merge_fail=1", snap);
        Assert.Contains("upsert_ok=1", snap);
        Assert.Contains("upsert_fail=2", snap);
    }
}
