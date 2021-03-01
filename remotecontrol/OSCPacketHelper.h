/*
  ==============================================================================

	OSCInputHelper.h
	Created: 21 Oct 2020 7:13:54pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

const OSCType OSCTypes::int32 = 'i';
const OSCType OSCTypes::float32 = 'f';
const OSCType OSCTypes::string = 's';
const OSCType OSCTypes::blob = 'b';
const OSCType OSCTypes::colour = 'r';

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
	uint64 getPosition() { return (uint64)input.getPosition(); }

	/** Attempts to set the current position of the stream. Returns true if this was successful. */
	bool setPosition(int64 pos) { return input.setPosition(pos); }

	/** Returns the total amount of data in bytes accessible by this stream. */
	int64 getTotalLength() { return input.getTotalLength(); }

	/** Returns true if the stream has no more data to read. */
	bool isExhausted() { return input.isExhausted(); }

	//==============================================================================
	int32 readInt32()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading int32");
		return input.readIntBigEndian();
	}

	uint64 readUint64()
	{
		checkBytesAvailable(8, "OSC input stream exhausted while reading uint64");
		return (uint64)input.readInt64BigEndian();
	}

	float readFloat32()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading float");
		return input.readFloatBigEndian();
	}

	String readString()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading string");

		auto posBegin = (size_t)getPosition();
		auto s = input.readString();
		auto posEnd = (size_t)getPosition();

		if (static_cast<const char*> (getData())[posEnd - 1] != '\0')
			throw OSCFormatError("OSC input stream exhausted before finding null terminator of string");

		size_t bytesRead = posEnd - posBegin;
		readPaddingZeros(bytesRead);

		return s;
	}

	MemoryBlock readBlob()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading blob");

		auto blobDataSize = input.readIntBigEndian();
		checkBytesAvailable((blobDataSize + 3) % 4, "OSC input stream exhausted before reaching end of blob");

		MemoryBlock blob;
		auto bytesRead = input.readIntoMemoryBlock(blob, (ssize_t)blobDataSize);
		readPaddingZeros(bytesRead);

		return blob;
	}

	OSCColour readColour()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading colour");
		return OSCColour::fromInt32((uint32)input.readIntBigEndian());
	}

	OSCTimeTag readTimeTag()
	{
		checkBytesAvailable(8, "OSC input stream exhausted while reading time tag");
		return OSCTimeTag(uint64(input.readInt64BigEndian()));
	}

	OSCAddress readAddress()
	{
		return OSCAddress(readString());
	}

	OSCAddressPattern readAddressPattern()
	{
		return OSCAddressPattern(readString());
	}

	//==============================================================================
	OSCTypeList readTypeTagString()
	{
		OSCTypeList typeList;

		checkBytesAvailable(4, "OSC input stream exhausted while reading type tag string");

		if (input.readByte() != ',')
			throw OSCFormatError("OSC input stream format error: expected type tag string");

		for (;;)
		{
			if (isExhausted())
				throw OSCFormatError("OSC input stream exhausted while reading type tag string");

			const OSCType type = input.readByte();

			if (type == 0)
				break;  // encountered null terminator. list is complete.

			if (!OSCTypes::isSupportedType(type))
				throw OSCFormatError("OSC input stream format error: encountered unsupported type tag \"" + String::charToString(type) + "\"");

			typeList.add(type);
		}

		auto bytesRead = (size_t)typeList.size() + 2;
		readPaddingZeros(bytesRead);

		return typeList;
	}

	//==============================================================================
	OSCArgument readArgument(OSCType type)
	{
		switch (type)
		{
		case 'i':       return OSCArgument(readInt32());
		case 'f':     return OSCArgument(readFloat32());
		case 's':      return OSCArgument(readString());
		case 'b':        return OSCArgument(readBlob());
		case 'r':      return OSCArgument(readColour());

		default:
			// You supplied an invalid OSCType when calling readArgument! This should never happen.
			jassertfalse;
			throw OSCInternalError("OSC input stream: internal error while reading message argument");
		}
	}

	//==============================================================================
	OSCMessage readMessage()
	{
		auto ap = readAddressPattern();
		auto types = readTypeTagString();

		OSCMessage msg(ap);

#if JUCE_IP_AND_PORT_DETECTION
		msg.setSenderIPAddress(senderIPAddress);
		msg.setSenderPortNumber(senderPortNumber);
#endif

		for (auto& type : types)
			msg.addArgument(readArgument(type));

		return msg;
	}

	//==============================================================================
	OSCBundle readBundle(size_t maxBytesToRead = std::numeric_limits<size_t>::max())
	{
		// maxBytesToRead is only passed in here in case this bundle is a nested
		// bundle, so we know when to consider the next element *not* part of this
		// bundle anymore (but part of the outer bundle) and return.

		checkBytesAvailable(16, "OSC input stream exhausted while reading bundle");

		if (readString() != "#bundle")
			throw OSCFormatError("OSC input stream format error: bundle does not start with string '#bundle'");

		OSCBundle bundle(readTimeTag());

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
	OSCBundle::Element readElement()
	{
		checkBytesAvailable(4, "OSC input stream exhausted while reading bundle element size");

		auto elementSize = (size_t)readInt32();

		if (elementSize < 4)
			throw OSCFormatError("OSC input stream format error: invalid bundle element size");

		return readElementWithKnownSize(elementSize);
	}

	//==============================================================================
	OSCBundle::Element readElementWithKnownSize(size_t elementSize)
	{
		checkBytesAvailable((int64)elementSize, "OSC input stream exhausted while reading bundle element content");

		auto firstContentChar = static_cast<const char*> (getData())[getPosition()];

		if (firstContentChar == '/')  return OSCBundle::Element(readMessageWithCheckedSize(elementSize));
		if (firstContentChar == '#')  return OSCBundle::Element(readBundleWithCheckedSize(elementSize));

		throw OSCFormatError("OSC input stream: invalid bundle element content");
	}

private:
	MemoryInputStream input;

#if JUCE_IP_AND_PORT_DETECTION
	String senderIPAddress;
	int senderPortNumber;
#endif

	//==============================================================================
	void readPaddingZeros(size_t bytesRead)
	{
		size_t numZeros = ~(bytesRead - 1) & 0x03;

		while (numZeros > 0)
		{
			if (isExhausted() || input.readByte() != 0)
				throw OSCFormatError("OSC input stream format error: missing padding zeros");

			--numZeros;
		}
	}

	OSCBundle readBundleWithCheckedSize(size_t size)
	{
		auto begin = (size_t)getPosition();
		auto maxBytesToRead = size - 4; // we've already read 4 bytes (the bundle size)

		OSCBundle bundle(readBundle(maxBytesToRead));

		if (getPosition() - begin != size)
			throw OSCFormatError("OSC input stream format error: wrong element content size encountered while reading");

		return bundle;
	}

	OSCMessage readMessageWithCheckedSize(size_t size)
	{
		auto begin = (size_t)getPosition();
		auto message = readMessage();

		if (getPosition() - begin != size)
			throw OSCFormatError("OSC input stream format error: wrong element content size encountered while reading");

		return message;
	}

	void checkBytesAvailable(int64 requiredBytes, const char* message)
	{
		if (input.getNumBytesRemaining() < requiredBytes)
			throw OSCFormatError(message);
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
	bool writeInt32(int32 value)
	{
		return output.writeIntBigEndian(value);
	}

	bool writeUint64(uint64 value)
	{
		return output.writeInt64BigEndian(int64(value));
	}

	bool writeFloat32(float value)
	{
		return output.writeFloatBigEndian(value);
	}

	bool writeString(const String& value)
	{
		if (!output.writeString(value))
			return false;

		const size_t numPaddingZeros = ~value.getNumBytesAsUTF8() & 3;

		return output.writeRepeatedByte('\0', numPaddingZeros);
	}

	bool writeBlob(const MemoryBlock& blob)
	{
		if (!(output.writeIntBigEndian((int)blob.getSize())
			&& output.write(blob.getData(), blob.getSize())))
			return false;

		const size_t numPaddingZeros = ~(blob.getSize() - 1) & 3;

		return output.writeRepeatedByte(0, numPaddingZeros);
	}

	bool writeColour(OSCColour colour)
	{
		return output.writeIntBigEndian((int32)colour.toInt32());
	}

	bool writeTimeTag(OSCTimeTag timeTag)
	{
		return output.writeInt64BigEndian(int64(timeTag.getRawTimeTag()));
	}

	bool writeAddress(const OSCAddress& address)
	{
		return writeString(address.toString());
	}

	bool writeAddressPattern(const OSCAddressPattern& ap)
	{
		return writeString(ap.toString());
	}

	bool writeTypeTagString(const OSCTypeList& typeList)
	{
		output.writeByte(',');

		if (typeList.size() > 0)
			output.write(typeList.begin(), (size_t)typeList.size());

		output.writeByte('\0');

		size_t bytesWritten = (size_t)typeList.size() + 1;
		size_t numPaddingZeros = ~bytesWritten & 0x03;

		return output.writeRepeatedByte('\0', numPaddingZeros);
	}

	bool writeArgument(const OSCArgument& arg)
	{
		switch (arg.getType())
		{
		case OSCTypes::int32:       return writeInt32(arg.getInt32());
		case OSCTypes::float32:     return writeFloat32(arg.getFloat32());
		case OSCTypes::string:      return writeString(arg.getString());
		case OSCTypes::blob:        return writeBlob(arg.getBlob());
		case OSCTypes::colour:      return writeColour(arg.getColour());

		default:
			// In this very unlikely case you supplied an invalid OSCType!
			jassertfalse;
			return false;
		}
	}

	//==============================================================================
	bool writeMessage(const OSCMessage& msg)
	{
		if (!writeAddressPattern(msg.getAddressPattern()))
			return false;

		OSCTypeList typeList;

		for (auto& arg : msg)
			typeList.add(arg.getType());

		if (!writeTypeTagString(typeList))
			return false;

		for (auto& arg : msg)
			if (!writeArgument(arg))
				return false;

		return true;
	}

	bool writeBundle(const OSCBundle& bundle)
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
	bool writeBundleElement(const OSCBundle::Element& element)
	{
		const int64 startPos = output.getPosition();

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

		const int64 endPos = output.getPosition();
		const int64 elementSize = endPos - (startPos + 4);

		return output.setPosition(startPos)
			&& writeInt32((int32)elementSize)
			&& output.setPosition(endPos);
	}

private:
	MemoryOutputStream output;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCPacketPacker)
};