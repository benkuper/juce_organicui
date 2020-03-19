/*
  ==============================================================================

    SHA1.h
    Created: 27 Jun 2012 7:48:01am
    Author:  Joe Fitzpatrick

  ==============================================================================
*/

#pragma once

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
	void update (const void* data, uint32 len);
    void update (MemoryBlock& data);
    
	// Finalize hash and report
	MemoryBlock finalize();

    //==============================================================================
    enum {
        SHA1_DIGEST_SIZE = 20,
        SHA1_BLOCK_SIZE	= 64
    };
    
    
private:
    //==============================================================================
	uint32 m_state[5];
	uint32 m_count[2];
	uint8  m_buffer[SHA1_BLOCK_SIZE];
	uint8  m_digest[SHA1_DIGEST_SIZE];

    //==============================================================================
    typedef union
    {
        uint8  c[SHA1_BLOCK_SIZE];
        uint32 l[SHA1_BLOCK_SIZE / sizeof(uint32)];
    } SHA1_WORKSPACE_BLOCK;
    
	// Private SHA-1 transformation
	void transform (uint32* state, const uint8* buffer);
    
	// Member variables
	uint8 m_workspace[SHA1_BLOCK_SIZE];
	SHA1_WORKSPACE_BLOCK *m_block; // SHA1 pointer to the byte array above
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SHA1)    
};
