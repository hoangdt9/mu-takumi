#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace mu
{
struct PacketKeySet
{
    std::array<uint32_t, 4> modulus {};
    std::array<uint32_t, 4> key {};
    std::array<uint32_t, 4> xorKey {};
};

static constexpr uint8_t kXor32Key[32] = {
    0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
    0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
    0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
    0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D,
};

inline void Xor32Encrypt(uint8_t* buffer, int length, int headerSize = 2)
{
    for (int i = headerSize + 1; i < length; ++i)
    {
        buffer[i] = static_cast<uint8_t>(buffer[i] ^ buffer[i - 1] ^ kXor32Key[i % 32]);
    }
}

inline void Xor32Decrypt(uint8_t* buffer, int length, int headerSize = 2)
{
    for (int i = length - 1; i > headerSize; --i)
    {
        buffer[i] = static_cast<uint8_t>(buffer[i] ^ buffer[i - 1] ^ kXor32Key[i % 32]);
    }
}

inline PacketKeySet gClientToServerKey {
    { 128079, 164742, 70235, 106898 },
    { 23489, 11911, 19816, 13647 },
    { 48413, 46165, 15171, 37433 },
};

inline PacketKeySet gServerToClientKey {
    { 73326, 109989, 98843, 171058 },
    { 18035, 30340, 24701, 11141 },
    { 62004, 64409, 35374, 64599 },
};

static constexpr int kSmDecBlockSize = 8;
static constexpr int kSmEncBlockSize = 11;
static constexpr uint8_t kSmBlockSizeXorKey = 0x3D;
static constexpr uint8_t kSmBlockChecksumXorKey = 0xF8;
static constexpr int kSmBitsPerValue = 18;

inline uint32_t ByteSwap32(uint32_t value)
{
    return ((value & 0x000000FFu) << 24u)
        | ((value & 0x0000FF00u) << 8u)
        | ((value & 0x00FF0000u) >> 8u)
        | ((value & 0xFF000000u) >> 24u);
}

inline int ByteOffset(int index)
{
    return (index * kSmBitsPerValue) / 8;
}

inline int BitOffset(int index)
{
    return (index * kSmBitsPerValue) % 8;
}

inline int FirstMask(int index)
{
    return 0xFF >> BitOffset(index);
}

inline uint8_t RemainderMask(int index)
{
    const int bitOffset = BitOffset(index);
    return static_cast<uint8_t>(((0xFF << (6 - bitOffset)) & 0xFF) - ((0xFF << (8 - bitOffset)) & 0xFF));
}

inline uint32_t ReadResult(const uint8_t* buffer, int index)
{
    const int bitOffset = BitOffset(index);
    int offset = ByteOffset(index);
    const uint8_t firstMask = static_cast<uint8_t>(FirstMask(index));
    uint32_t result = 0;
    result += static_cast<uint32_t>(static_cast<uint32_t>(buffer[offset++] & firstMask) << (24 + bitOffset));
    result += static_cast<uint32_t>(static_cast<uint32_t>(buffer[offset++]) << (16 + bitOffset));
    result += static_cast<uint32_t>(static_cast<uint32_t>(buffer[offset] & static_cast<uint8_t>(0xFF << (8 - bitOffset))) << (8 + bitOffset));
    result = ByteSwap32(result);
    const uint8_t remainderMask = RemainderMask(index);
    const uint8_t remainder = static_cast<uint8_t>(buffer[offset] & remainderMask);
    result += static_cast<uint32_t>(remainder << 16) >> (6 - bitOffset);
    return result;
}

inline void WriteResult(uint8_t* buffer, int index, uint32_t result)
{
    const int bitOffset = BitOffset(index);
    int offset = ByteOffset(index);
    const uint8_t firstMask = static_cast<uint8_t>(FirstMask(index));
    const uint32_t swapped = ByteSwap32(result);

    buffer[offset++] |= static_cast<uint8_t>((swapped >> (24 + bitOffset)) & firstMask);
    buffer[offset++] = static_cast<uint8_t>(swapped >> (16 + bitOffset));
    buffer[offset] = static_cast<uint8_t>((swapped >> (8 + bitOffset)) & static_cast<uint32_t>(0xFF << (8 - bitOffset)));

    const uint32_t remainder = (result >> 16u) << static_cast<uint32_t>(6 - bitOffset);
    buffer[offset] |= static_cast<uint8_t>(remainder & RemainderMask(index));
}

// Encrypts one block using Client->Server keys
inline void SetClientToServerKey(const PacketKeySet& keySet)
{
    gClientToServerKey = keySet;
}

inline void SetServerToClientKey(const PacketKeySet& keySet)
{
    gServerToClientKey = keySet;
}

inline void EncryptBlockWithKey(const uint8_t* inputBlock, uint8_t* outputBlock, int actualBlockSize, const PacketKeySet& keySet)
{
    uint16_t in16[4];
    std::memcpy(in16, inputBlock, 8);

    uint32_t result[4];
    result[0] = ((static_cast<uint32_t>(keySet.xorKey[0] ^ static_cast<uint32_t>(in16[0])) * keySet.key[0]) % keySet.modulus[0]);
    for (int i = 1; i < 4; ++i)
    {
        const uint32_t xored = static_cast<uint32_t>(in16[i]) ^ (result[i - 1] & 0xFFFFu);
        result[i] = ((keySet.xorKey[i] ^ xored) * keySet.key[i]) % keySet.modulus[i];
    }

    for (int i = 0; i < 3; ++i)
    {
        result[i] = result[i] ^ keySet.xorKey[i] ^ (result[i + 1] & 0xFFFFu);
    }

    for (int i = 0; i < 4; ++i)
    {
        WriteResult(outputBlock, i, result[i]);
    }

    uint8_t checksum = kSmBlockChecksumXorKey;
    for (int i = 0; i < actualBlockSize; ++i)
    {
        checksum ^= inputBlock[i];
    }

    const uint8_t size = static_cast<uint8_t>(actualBlockSize ^ kSmBlockSizeXorKey) ^ checksum;
    outputBlock[kSmEncBlockSize - 2] = size;
    outputBlock[kSmEncBlockSize - 1] = checksum;
}

inline std::vector<uint8_t> SM_EncryptPacketWithKey(const std::vector<uint8_t>& plain, uint8_t counter, const PacketKeySet& keySet)
{
    if (plain.size() < 3 || plain[0] < 0xC3)
    {
        return plain;
    }

    static constexpr int headerSize = 2;
    const int plainSize = static_cast<int>(plain.size());
    const int contentSize = plainSize - headerSize;
    const int totalWithCounter = contentSize + 1;
    const int blockCount = (totalWithCounter + kSmDecBlockSize - 1) / kSmDecBlockSize;
    const int encryptedContentSize = blockCount * kSmEncBlockSize;
    const int encryptedPacketSize = headerSize + encryptedContentSize;

    std::vector<uint8_t> encrypted(encryptedPacketSize, 0);
    encrypted[0] = plain[0];
    encrypted[1] = static_cast<uint8_t>(encryptedPacketSize);

    const uint8_t* src = plain.data() + headerSize;
    uint8_t* dst = encrypted.data() + headerSize;

    uint8_t block[kSmDecBlockSize] = {};
    block[0] = counter;
    const int firstLength = std::min(kSmDecBlockSize - 1, contentSize);
    std::memcpy(block + 1, src, firstLength);
    EncryptBlockWithKey(block, dst, 1 + firstLength, keySet);
    dst += kSmEncBlockSize;

    int sourceOffset = kSmDecBlockSize - 1;
    while (sourceOffset < contentSize)
    {
        std::memset(block, 0, kSmDecBlockSize);
        const int length = std::min(kSmDecBlockSize, contentSize - sourceOffset);
        std::memcpy(block, src + sourceOffset, length);
        EncryptBlockWithKey(block, dst, length, keySet);
        dst += kSmEncBlockSize;
        sourceOffset += kSmDecBlockSize;
    }

    return encrypted;
}

inline std::vector<uint8_t> SM_EncryptPacket(const std::vector<uint8_t>& plain, uint8_t counter)
{
    return SM_EncryptPacketWithKey(plain, counter, gClientToServerKey);
}

// Decrypts one block using Server->Client keys
inline bool SM_DecryptBlockWithKey(const uint8_t* inputBlock, uint8_t* outputBlock, uint8_t& blockSize, const PacketKeySet& keySet)
{
    if (!inputBlock || !outputBlock)
    {
        return false;
    }

    uint32_t encryptionResult[4] = {};
    for (int i = 0; i < 4; ++i)
    {
        encryptionResult[i] = ReadResult(inputBlock, i);
    }

    // Undo the XOR chain applied during server encryption (uses Server->Client xor key)
    for (int i = 3; i > 0; --i)
    {
        encryptionResult[i - 1] = encryptionResult[i - 1] ^ keySet.xorKey[i - 1] ^ (encryptionResult[i] & 0xFFFFu);
    }

    // Apply modular inverse using Server->Client decrypt keys
    for (int i = 0; i < 4; ++i)
    {
        uint32_t result = keySet.xorKey[i] ^ ((encryptionResult[i] * keySet.key[i]) % keySet.modulus[i]);
        if (i > 0)
        {
            result ^= (encryptionResult[i - 1] & 0xFFFFu);
        }

        const uint16_t word = static_cast<uint16_t>(result & 0xFFFFu);
        outputBlock[(i * 2) + 0] = static_cast<uint8_t>(word & 0xFF);
        outputBlock[(i * 2) + 1] = static_cast<uint8_t>((word >> 8) & 0xFF);
    }

    const uint8_t encryptedBlockSize = inputBlock[kSmEncBlockSize - 2];
    const uint8_t expectedChecksum = inputBlock[kSmEncBlockSize - 1];
    blockSize = static_cast<uint8_t>(encryptedBlockSize ^ expectedChecksum ^ kSmBlockSizeXorKey);
    if (blockSize > kSmDecBlockSize)
    {
        return false;
    }

    uint8_t calculatedChecksum = kSmBlockChecksumXorKey;
    for (int i = 0; i < kSmDecBlockSize; ++i)
    {
        calculatedChecksum ^= outputBlock[i];
    }

    return calculatedChecksum == expectedChecksum;
}

inline std::vector<uint8_t> SM_DecryptPacketWithKey(
    const std::vector<uint8_t>& encrypted,
    const PacketKeySet& keySet,
    bool* success = nullptr,
    uint8_t* packetCounter = nullptr)
{
    if (success)
    {
        *success = false;
    }

    if (encrypted.size() < 3 || encrypted[0] < 0xC3)
    {
        if (success)
        {
            *success = true;
        }

        return encrypted;
    }

    const uint8_t encryptedType = encrypted[0];
    const int headerSize = (encryptedType == 0xC4) ? 3 : 2;
    if (encrypted.size() < static_cast<size_t>(headerSize))
    {
        return {};
    }

    const size_t encryptedContentSize = encrypted.size() - static_cast<size_t>(headerSize);
    if (encryptedContentSize == 0 || (encryptedContentSize % kSmEncBlockSize) != 0)
    {
        return {};
    }

    std::vector<uint8_t> decryptedContent;
    decryptedContent.reserve((encryptedContentSize / kSmEncBlockSize) * kSmDecBlockSize);

    bool firstBlock = true;
    for (size_t offset = static_cast<size_t>(headerSize); offset < encrypted.size(); offset += kSmEncBlockSize)
    {
        uint8_t outputBlock[kSmDecBlockSize] = {};
        uint8_t blockSize = 0;
        if (!SM_DecryptBlockWithKey(encrypted.data() + offset, outputBlock, blockSize, keySet))
        {
            return {};
        }

        size_t dataOffset = 0;
        if (firstBlock)
        {
            firstBlock = false;
            if (packetCounter)
            {
                *packetCounter = outputBlock[0];
            }

            dataOffset = 1;
        }

        if (blockSize < dataOffset)
        {
            return {};
        }

        decryptedContent.insert(
            decryptedContent.end(),
            outputBlock + static_cast<std::ptrdiff_t>(dataOffset),
            outputBlock + static_cast<std::ptrdiff_t>(blockSize));
    }

    const uint8_t plainType = encryptedType;
    const size_t plainSize = static_cast<size_t>(headerSize) + decryptedContent.size();
    if (plainSize > 0xFFFFu || ((plainType == 0xC1 || plainType == 0xC3) && plainSize > 0xFFu))
    {
        return {};
    }

    std::vector<uint8_t> plain(plainSize, 0);
    plain[0] = plainType;
    if (plainType == 0xC1 || plainType == 0xC3)
    {
        plain[1] = static_cast<uint8_t>(plainSize);
    }
    else
    {
        plain[1] = static_cast<uint8_t>((plainSize >> 8u) & 0xFFu);
        plain[2] = static_cast<uint8_t>(plainSize & 0xFFu);
    }

    std::copy(decryptedContent.begin(), decryptedContent.end(), plain.begin() + headerSize);

    if (success)
    {
        *success = true;
    }

    return plain;
}

inline std::vector<uint8_t> SM_DecryptPacket(const std::vector<uint8_t>& encrypted, bool* success = nullptr, uint8_t* packetCounter = nullptr)
{
    return SM_DecryptPacketWithKey(encrypted, gServerToClientKey, success, packetCounter);
}
} // namespace mu
