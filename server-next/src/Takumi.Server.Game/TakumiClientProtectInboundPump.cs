using System.Buffers;
using System.IO.Pipelines;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>
/// Byte-stream helper that applies <see cref="TakumiClientProtectWire602.DecryptInPlace"/> to each TCP read chunk
/// <b>before</b> <see cref="MUnique.OpenMU.Network.PipelinedDecryptor"/>.
/// Android <c>CWsctlc::sSend</c>: <c>SendPacket(..., TRUE)</c> builds SimpleModulus <c>C3</c> on the buffer, then
/// <c>gProtect.EncryptData</c> on the <b>whole</b> send buffer when the peer port is in <c>GSPortMin..GSPortMax</c>
/// (see <c>android_link_stubs.cpp</c>). Wire order is <c>gProtect( SM_packet )</c>, not SM inside gProtect.
/// </summary>
internal static class TakumiClientProtectInboundPump
{
    static bool DumpProtectInboundHex =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_DUMP_PROTECT_IN"), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_DUMP_PROTECT_IN"), "true", StringComparison.OrdinalIgnoreCase);

    static int s_protectInDumpBudget = 4;

    public static Task RunAsync(
        PipeReader source,
        PipeWriter destination,
        byte encDecKey1,
        byte encDecKey2,
        CancellationToken cancellationToken)
    {
        return PumpCoreAsync(source, destination, encDecKey1, encDecKey2, cancellationToken);
    }

    static async Task PumpCoreAsync(
        PipeReader source,
        PipeWriter destination,
        byte k1,
        byte k2,
        CancellationToken cancellationToken)
    {
        try
        {
            for (;;)
            {
                ReadResult read;
                try
                {
                    read = await source.ReadAsync(cancellationToken).ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    break;
                }

                var buffer = read.Buffer;
                if (buffer.IsEmpty && read.IsCompleted)
                {
                    break;
                }

                if (!buffer.IsEmpty)
                {
                    var total = (int)buffer.Length;
                    if (total > 0)
                    {
                        var combined = new byte[total];
                        buffer.CopyTo(combined.AsSpan());
                        TakumiClientProtectWire602.DecryptInPlace(combined.AsSpan(), k1, k2);
                        if (DumpProtectInboundHex && Interlocked.Decrement(ref s_protectInDumpBudget) >= 0)
                        {
                            var preview = Math.Min(24, combined.Length);
                            Console.Error.WriteLine(
                                "[game-host] protect_in chunk len={0} preview={1}",
                                combined.Length,
                                Convert.ToHexString(combined.AsSpan(0, preview)));
                        }

                        await destination.WriteAsync(combined, cancellationToken).ConfigureAwait(false);
                    }
                }

                source.AdvanceTo(buffer.End);
                var flush = await destination.FlushAsync(cancellationToken).ConfigureAwait(false);
                if (flush.IsCanceled)
                {
                    break;
                }

                if (read.IsCompleted)
                {
                    break;
                }
            }
        }
        catch (OperationCanceledException)
        {
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[game-host] protect inbound pump exception: {0}", ex);
            try
            {
                await destination.CompleteAsync(ex).ConfigureAwait(false);
            }
            catch
            {
            }

            return;
        }

        try
        {
            await destination.CompleteAsync().ConfigureAwait(false);
        }
        catch
        {
        }
    }
}
