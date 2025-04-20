/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


#define DECLARE_TYPE(type) juce::String getTypeString() const override { return getTypeStringStatic() ; } \
static juce::String getTypeStringStatic() { return type; }


template <class T, class G>
class NestingManager :
	public Manager<T>
{
public:

	static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemGroup<T>");

	NestingManager(const juce::String& name);
	virtual ~NestingManager();

	juce::OwnedArray<G, juce::CriticalSection> groups;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NestingManager)
};

template<class T, class G>
NestingManager<T, G>::NestingManager(const juce::String& name)
{
}

template<class T, class G>
NestingManager<T, G>::~NestingManager()
{
}
