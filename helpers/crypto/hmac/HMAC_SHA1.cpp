/*
  ==============================================================================

    HMAC_SHA1.cpp
    Created: 27 Jun 2012 7:48:28am
    Author:  Joe Fitzpatrick

  ==============================================================================
*/

#include "HMAC_SHA1.h"
#include "SHA1.h"

const static int HMAC_BLOCK_SIZE = OrganicCrypto::SHA1::SHA1_BLOCK_SIZE;


//==============================================================================
//==============================================================================
MemoryBlock HMAC_SHA1::encode (const String& text, const String& key)
{
    return encode (text.toUTF8(), text.length(), key.toUTF8(), key.length());
}

//==============================================================================
MemoryBlock HMAC_SHA1::encode (const char* text, int textLen, const char* key, int keyLen)
{
    OrganicCrypto::SHA1 sha1;
    
	// Step 1, key
    MemoryBlock sha1Key (HMAC_BLOCK_SIZE, true);

	if (keyLen > HMAC_BLOCK_SIZE)
	{
		sha1.reset();
		sha1.update (key, keyLen);
		MemoryBlock b = sha1.finalize();
        sha1Key.copyFrom (b.getData(), 0, b.getSize());
	}
	else
		sha1Key.copyFrom(key, 0, keyLen);
    
	// Step 2, apply key to ipad
    MemoryBlock ipad (HMAC_BLOCK_SIZE);
    ipad.fillWith (0x36);

	for (size_t i = 0; i < ipad.getSize(); ++i)
		ipad[i] ^= sha1Key[i];		

	// Step 3, combine ipad with text
    MemoryBlock appendBuf1 (ipad.getSize() + textLen);
    appendBuf1.copyFrom (ipad.getData(), 0, ipad.getSize());
    appendBuf1.copyFrom (text, (int)ipad.getSize(), textLen);
    
	// Step 4, encode it
	sha1.reset();
	sha1.update (appendBuf1);
	MemoryBlock report = sha1.finalize();
    
	// Step 5, apply key to opad
    MemoryBlock opad (HMAC_BLOCK_SIZE);
    opad.fillWith (0x5c);
    
	for (size_t i = 0; i < opad.getSize(); ++i)
		opad[i] ^= sha1Key[i];
    
	// Step 6, combine opad with previous report
    MemoryBlock appendBuf2 (opad.getSize() + report.getSize());
    appendBuf2.copyFrom (opad.getData(), 0, opad.getSize());
    appendBuf2.copyFrom (report.getData(), (int)opad.getSize(), report.getSize());
    
	// Step 7, encode and return that
	sha1.reset();
	sha1.update (appendBuf2);
    return sha1.finalize();
}
