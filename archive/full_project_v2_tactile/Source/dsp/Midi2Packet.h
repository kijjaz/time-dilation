#pragma once

#include <cstdint>

namespace time_dilation
{

enum class Midi2MessageType : uint8_t
{
    Utility             = 0x0,
    System              = 0x1,
    Midi1ChannelVoice   = 0x2,
    Data64              = 0x3,
    Midi2ChannelVoice   = 0x4,
    Data128             = 0x5
};

enum class Midi2Status : uint8_t
{
    NoteOff             = 0x8,
    NoteOn              = 0x9,
    PolyPressure        = 0xA,
    PerNoteController   = 0xB,
    ChannelPressure     = 0xD,
    PitchBend           = 0xE,
    PerNoteManagement   = 0xF
};

// Universal MIDI Packet (UMP) - 128-bit structure
struct Midi2Packet
{
    uint32_t word0 { 0 };
    uint32_t word1 { 0 };
    uint32_t word2 { 0 };
    uint32_t word3 { 0 };

    double timestamp { 0.0 }; // Coordinate time t in seconds

    static Midi2Packet createNoteOn (uint8_t group, uint8_t channel, uint8_t noteNumber, uint32_t velocity32, double timeSec)
    {
        Midi2Packet packet;
        packet.timestamp = timeSec;
        packet.word0 = (static_cast<uint8_t>(Midi2MessageType::Midi2ChannelVoice) << 28)
                     | ((group & 0x0F) << 24)
                     | (static_cast<uint8_t>(Midi2Status::NoteOn) << 20)
                     | ((channel & 0x0F) << 16)
                     | (noteNumber << 8);
        packet.word1 = velocity32;
        return packet;
    }

    static Midi2Packet createNoteOff (uint8_t group, uint8_t channel, uint8_t noteNumber, uint32_t velocity32, double timeSec)
    {
        Midi2Packet packet;
        packet.timestamp = timeSec;
        packet.word0 = (static_cast<uint8_t>(Midi2MessageType::Midi2ChannelVoice) << 28)
                     | ((group & 0x0F) << 24)
                     | (static_cast<uint8_t>(Midi2Status::NoteOff) << 20)
                     | ((channel & 0x0F) << 16)
                     | (noteNumber << 8);
        packet.word1 = velocity32;
        return packet;
    }

    static Midi2Packet createPerNoteController (uint8_t group, uint8_t channel, uint8_t noteNumber, uint8_t index, uint32_t value32, double timeSec)
    {
        Midi2Packet packet;
        packet.timestamp = timeSec;
        packet.word0 = (static_cast<uint8_t>(Midi2MessageType::Midi2ChannelVoice) << 28)
                     | ((group & 0x0F) << 24)
                     | (static_cast<uint8_t>(Midi2Status::PerNoteController) << 20)
                     | ((channel & 0x0F) << 16)
                     | (noteNumber << 8)
                     | index;
        packet.word1 = value32;
        return packet;
    }

    uint8_t getMessageType() const { return static_cast<uint8_t>((word0 >> 28) & 0x0F); }
    uint8_t getStatus() const { return static_cast<uint8_t>((word0 >> 20) & 0x0F); }
    uint8_t getChannel() const { return static_cast<uint8_t>((word0 >> 16) & 0x0F); }
    uint8_t getNoteNumber() const { return static_cast<uint8_t>((word0 >> 8) & 0xFF); }
    uint32_t getVelocity32() const { return word1; }
};

} // namespace time_dilation
