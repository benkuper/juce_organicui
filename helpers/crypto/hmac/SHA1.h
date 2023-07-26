/*
  ==============================================================================

    SHA1.h
    Created: 27 Jun 2012 7:48:01am
    Author:  Joe Fitzpatrick

  ==============================================================================
*/

#pragma once

namespace OrganicCrypto
{

    class SHA1
    {
    public:
        //==============================================================================
        // Constructor and Destructor
        SHA1();
        ~SHA1();

        //==============================================================================
        void reset();

        // Update the hash value
        void update(const void* data, juce::uint32 len);
        void update(juce::MemoryBlock& data);

        // Finalize hash and report
        juce::MemoryBlock finalize();

        static juce::String convert(const juce::String& input);

        //==============================================================================
        enum {
            SHA1_DIGEST_SIZE = 20,
            SHA1_BLOCK_SIZE = 64
        };


    private:
        //==============================================================================
        juce::uint32 m_state[5];
        juce::uint32 m_count[2];
        juce::uint8  m_buffer[SHA1_BLOCK_SIZE];
        juce::uint8  m_digest[SHA1_DIGEST_SIZE];

        //==============================================================================
        typedef union
        {
            juce::uint8  c[SHA1_BLOCK_SIZE];
            juce::uint32 l[SHA1_BLOCK_SIZE / sizeof(juce::uint32)];
        } SHA1_WORKSPACE_BLOCK;

        // Private SHA-1 transformation
        void transform(juce::uint32* state, const juce::uint8* buffer);

        // Member variables
        juce::uint8 m_workspace[SHA1_BLOCK_SIZE];
        SHA1_WORKSPACE_BLOCK* m_block; // SHA1 pointer to the byte array above

        //==============================================================================
    };
}
