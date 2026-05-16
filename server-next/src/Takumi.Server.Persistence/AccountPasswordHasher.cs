using BCrypt.Net;

namespace Takumi.Server.Persistence;

/// <summary>BCrypt password hashing (OpenMU-style; legacy MEMB_INFO stored plaintext).</summary>
public static class AccountPasswordHasher
{
    public static string Hash(string plainPassword) =>
        BCrypt.Net.BCrypt.HashPassword(plainPassword, workFactor: 11);

    public static bool Verify(string plainPassword, string passwordHash)
    {
        if (string.IsNullOrEmpty(passwordHash))
        {
            return false;
        }

        try
        {
            return BCrypt.Net.BCrypt.Verify(plainPassword, passwordHash);
        }
        catch (SaltParseException)
        {
            return false;
        }
    }
}
