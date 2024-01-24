/*
  ==============================================================================

	OSCInputHelper.h
	Created: 21 Oct 2020 7:13:54pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once



class OSCPacketParser
{
public:
	/** Creates an OSCPacketParser.

		@param sourceData               the block of data to use as the stream's source
		@param sourceDataSize           the number of bytes in the source data block
	*/
	OSCPacketParser(const void* sourceData, size_t sourceDataSize) :
		input(sourceData, sourceDataSize, false)
	{}

	//==============================================================================
	/** Returns a pointer to the source data block from which this stream is reading. */
	const void* getData() const noexcept { return input.getData(); }

	/** Returns the number of bytes of source data in the block from which this stream is reading. */
	size_t getDataSize() const noexcept { return input.getDataSize(); }

	/** Returns the current position of the stream. */
	juce::uint64 getPosition() { return (juce::uint64)input.getPosition(); }

	/** Attempts to set the current position of the stream. Returns true if this was successful. */
	bool setPosition(juce::int64 pos) { return input.setPosition(pos); }

	/** Returns the total amount of data in bytes accessible by this stream. */
	juce::int64 getTotalLength() { return input.getTotalLength(); }

	/** Returns true if the stream has no more data to read. */
	bool isExhausted() { return input.isExhausted(); }

	//==============================================================================
	juce::int32 readInt32()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading int32");
		return input.readIntBigEndian();
	}

	juce::uint64 readUint64()
	{
		checkBytesAvailable(8, "OSC input stream exhausted while reading uint64");
		return (juce::uint64)input.readInt64BigEndian();
	}

	float readFloat32()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading float");
		return input.readFloatBigEndian();
	}

	juce::String readString()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading string");

		auto posBegin = (size_t)getPosition();
		auto s = input.readString();
		auto posEnd = (size_t)getPosition();

		if (static_cast<const char*> (getData())[posEnd - 1] != '\0')
			throw juce::OSCFormatError("OSC input stream exhausted before finding null terminator of string");

		size_t bytesRead = posEnd - posBegin;
		readPaddingZeros(bytesRead);

		return s;
	}

	juce::MemoryBlock readBlob()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading blob");

		auto blobDataSize = input.readIntBigEndian();
		checkBytesAvailable((blobDataSize + 3) % 4, "OSC input stream exhausted before reaching end of blob");

		juce::MemoryBlock blob;
		auto bytesRead = input.readIntoMemoryBlock(blob, blobDataSize);
		readPaddingZeros(bytesRead);

		return blob;
	}

	juce::OSCColour readColour()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading colour");
		return juce::OSCColour::fromInt32((juce::uint32)input.readIntBigEndian());
	}

	juce::OSCTimeTag readTimeTag()
	{
		checkBytesAvailable(8, "OSC input stream exhausted while reading time tag");
		return juce::OSCTimeTag(juce::uint64(input.readInt64BigEndian()));
	}

	juce::OSCAddress readAddress()
	{
		return juce::OSCAddress(readString());
	}

	juce::OSCAddressPattern readAddressPattern()
	{
		return juce::OSCAddressPattern(readString());
	}

	//==============================================================================
	juce::OSCTypeList readTypeTagString()
	{
		juce::OSCTypeList typeList;

		checkBytesAvailable(4, "OSC input stream exhausted while reading type tag string");

		if (input.readByte() != ',')
			throw juce::OSCFormatError("OSC input stream format error: expected type tag string");

		for (;;)
		{
			if (isExhausted())
				throw juce::OSCFormatError("OSC input stream exhausted while reading type tag string");

			const juce::OSCType type = input.readByte();

			if (type == 0)
				break;  // encountered null terminator. list is complete.

			if (!juce::OSCTypes::isSupportedType(type))
				throw juce::OSCFormatError("OSC input stream format error: encountered unsupported type tag \"" + juce::String::charToString(type) + "\"");

			typeList.add(type);
		}

		auto bytesRead = (size_t)typeList.size() + 2;
		readPaddingZeros(bytesRead);

		return typeList;
	}

	//==============================================================================
	juce::OSCArgument readArgument(juce::OSCType type)
	{
		switch (type)
		{
		case 'i':       return juce::OSCArgument(readInt32());
		case 'f':     return juce::OSCArgument(readFloat32());
		case 's':      return juce::OSCArgument(readString());
		case 'b':        return juce::OSCArgument(readBlob());
		case 'r':      return juce::OSCArgument(readColour());
		case 'T': 	           return juce::OSCArgument(true);
		case 'F':		      return juce::OSCArgument(false);
		default:
			// You supplied an invalid OSCType when calling readArgument! This should never happen.
			jassertfalse;
			throw juce::OSCInternalError("OSC input stream: internal error while reading message argument");
		}
	}

	//==============================================================================
	juce::OSCMessage readMessage()
	{
		auto ap = readAddressPattern();
		auto types = readTypeTagString();

		juce::OSCMessage msg(ap);

#if JUCE_IP_AND_PORT_DETECTION
		msg.setSenderIPAddress(senderIPAddress);
		msg.setSenderPortNumber(senderPortNumber);
#endif

		for (auto& type : types)
			msg.addArgument(readArgument(type));

		return msg;
	}

	//==============================================================================
	juce::OSCBundle readBundle(size_t maxBytesToRead = std::numeric_limits<size_t>::max())
	{
		// maxBytesToRead is only passed in here in case this bundle is a nested
		// bundle, so we know when to consider the next element *not* part of this
		// bundle anymore (but part of the outer bundle) and return.

		checkBytesAvailable(16, "OSC input stream exhausted while reading bundle");

		if (readString() != "#bundle")
			throw juce::OSCFormatError("OSC input stream format error: bundle does not start with string '#bundle'");

		juce::OSCBundle bundle(readTimeTag());

		size_t bytesRead = 16; // already read "#bundle" and timeTag
		auto pos = getPosition();

		while (!isExhausted() && bytesRead < maxBytesToRead)
		{
			bundle.addElement(readElement());

			auto newPos = getPosition();
			bytesRead += (size_t)(newPos - pos);
			pos = newPos;
		}

		return bundle;
	}

	//==============================================================================
	juce::OSCBundle::Element readElement()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading bundle element size");

		auto elementSize = (size_t)readInt32();

		if (elementSize < 4)
			throw juce::OSCFormatError("OSC input stream format error: invalid bundle element size");

		return readElementWithKnownSize(elementSize);
	}

	//==============================================================================
	juce::OSCBundle::Element readElementWithKnownSize(size_t elementSize)
	{
		checkBytesAvailable((juce::int64)elementSize, "OSC input stream exhausted while reading bundle element content");

		auto firstContentChar = static_cast<const char*> (getData())[getPosition()];

		if (firstContentChar == '/')  return juce::OSCBundle::Element(readMessageWithCheckedSize(elementSize));
		if (firstContentChar == '#')  return juce::OSCBundle::Element(readBundleWithCheckedSize(elementSize));

		throw juce::OSCFormatError("OSC input stream: invalid bundle element content");
	}

private:
	juce::MemoryInputStream input;

#if JUCE_IP_AND_PORT_DETECTION
	juce::String senderIPAddress;
	int senderPortNumber;
#endif

	//==============================================================================
	void readPaddingZeros(size_t bytesRead)
	{
		size_t numZeros = ~(bytesRead - 1) & 0x03;

		while (numZeros > 0)
		{
			if (isExhausted() || input.readByte() != 0)
				throw juce::OSCFormatError("OSC input stream format error: missing padding zeros");

			--numZeros;
		}
	}

	juce::OSCBundle readBundleWithCheckedSize(size_t size)
	{
		auto begin = (size_t)getPosition();
		auto maxBytesToRead = size - 4; // we've already read 4 bytes (the bundle size)

		juce::OSCBundle bundle(readBundle(maxBytesToRead));

		if (getPosition() - begin != size)
			throw juce::OSCFormatError("OSC input stream format error: wrong element content size encountered while reading");

		return bundle;
	}

	juce::OSCMessage readMessageWithCheckedSize(size_t size)
	{
		auto begin = (size_t)getPosition();
		auto message = readMessage();

		if (getPosition() - begin != size)
			throw juce::OSCFormatError("OSC input stream format error: wrong element content size encountered while reading");

		return message;
	}

	void checkBytesAvailable(juce::int64 requiredBytes, const char* message)
	{
		if (input.getNumBytesRemaining() < requiredBytes)
			throw juce::OSCFormatError(message);
	}
};


class OSCPacketPacker
{
public:
	OSCPacketPacker() noexcept {}

	/** Returns a pointer to the data that has been written to the stream. */
	const void* getData() const noexcept { return output.getData(); }

	/** Returns the number of bytes of data that have been written to the stream. */
	size_t getDataSize() const noexcept { return output.getDataSize(); }

	//==============================================================================
	bool writeInt32(juce::int32 value)
	{
		return output.writeIntBigEndian(value);
	}

	bool writeUint64(juce::uint64 value)
	{
		return output.writeInt64BigEndian(juce::int64(value));
	}

	bool writeFloat32(float value)
	{
		return output.writeFloatBigEndian(value);
	}

	bool writeString(const juce::String& value)
	{
		if (!output.writeString(value))
			return false;

		const size_t numPaddingZeros = ~value.getNumBytesAsUTF8() & 3;

		return output.writeRepeatedByte('\0', numPaddingZeros);
	}

	bool writeBlob(const juce::MemoryBlock& blob)
	{
		if (!(output.writeIntBigEndian((int)blob.getSize())
			&& output.write(blob.getData(), blob.getSize())))
			return false;

		const size_t numPaddingZeros = ~(blob.getSize() - 1) & 3;

		return output.writeRepeatedByte(0, numPaddingZeros);
	}

	bool writeColour(juce::OSCColour colour)
	{
		return output.writeIntBigEndian((juce::int32)colour.toInt32());
	}

	bool writeTimeTag(juce::OSCTimeTag timeTag)
	{
		return output.writeInt64BigEndian(juce::int64(timeTag.getRawTimeTag()));
	}

	bool writeAddress(const juce::OSCAddress& address)
	{
		return writeString(address.toString());
	}

	bool writeAddressPattern(const juce::OSCAddressPattern& ap)
	{
		return writeString(ap.toString());
	}

	bool writeTypeTagString(const juce::OSCTypeList& typeList)
	{
		output.writeByte(',');

		if (typeList.size() > 0)
			output.write(typeList.begin(), (size_t)typeList.size());

		output.writeByte('\0');

		size_t bytesWritten = (size_t)typeList.size() + 1;
		size_t numPaddingZeros = ~bytesWritten & 0x03;

		return output.writeRepeatedByte('\0', numPaddingZeros);
	}

	bool writeArgument(const juce::OSCArgument& arg)
	{
		switch (arg.getType())
		{
		case 'i':       return writeInt32(arg.getInt32());
		case 'f':     return writeFloat32(arg.getFloat32());
		case 's':      return writeString(arg.getString());
		case 'b':        return writeBlob(arg.getBlob());
		case 'r':      return writeColour(arg.getColour());

		default:
			// In this very unlikely case you supplied an invalid OSCType!
			jassertfalse;
			return false;
		}
	}

	//==============================================================================
	bool writeMessage(const juce::OSCMessage& msg)
	{
		if (!writeAddressPattern(msg.getAddressPattern()))
			return false;

		juce::OSCTypeList typeList;

		for (auto& arg : msg)
			typeList.add(arg.getType());

		if (!writeTypeTagString(typeList))
			return false;

		for (auto& arg : msg)
			if (!writeArgument(arg))
				return false;

		return true;
	}

	bool writeBundle(const juce::OSCBundle& bundle)
	{
		if (!writeString("#bundle"))
			return false;

		if (!writeTimeTag(bundle.getTimeTag()))
			return false;

		for (auto& element : bundle)
			if (!writeBundleElement(element))
				return false;

		return true;
	}

	//==============================================================================
	bool writeBundleElement(const juce::OSCBundle::Element& element)
	{
		const juce::int64 startPos = output.getPosition();

		if (!writeInt32(0))   // writing dummy value for element size
			return false;

		if (element.isBundle())
		{
			if (!writeBundle(element.getBundle()))
				return false;
		}
		else
		{
			if (!writeMessage(element.getMessage()))
				return false;
		}

		const juce::int64 endPos = output.getPosition();
		const juce::int64 elementSize = endPos - (startPos + 4);

		return output.setPosition(startPos)
			&& writeInt32((juce::int32)elementSize)
			&& output.setPosition(endPos);
	}

private:
	juce::MemoryOutputStream output;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCPacketPacker)
};