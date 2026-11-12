/*
  ==============================================================================

    QueuedNotifier.h
    Created: 11 May 2016 4:29:20pm
    Author:  Martin Hermant

  ==============================================================================
*/
#pragma once

#include <atomic>

#define ENUM_LIST(...) {__VA_ARGS__}

#define EVENT_NO_CHECK true
#define EVENT_ITEM_CHECK !isClearing && !isBeingDestroyed
#define EVENT_INSPECTABLE_CHECK !isBeingDestroyed
;
#define DECLARE_ASYNC_EVENT(BaseClass, EventPrefix, NotifierPrefix, Types, RemoveCheck) \
class  EventPrefix ## Event \
{ \
public: \
	enum Type Types; \
	 EventPrefix ## Event(Type t, BaseClass * item) : type(t), item(item) {} \
	Type type; \
	BaseClass* item; \
}; \
QueuedNotifier<EventPrefix ## Event> NotifierPrefix ## Notifier; \
typedef QueuedNotifier<EventPrefix ## Event>::Listener AsyncListener; \
void addAsync ## EventPrefix ## Listener(AsyncListener* newListener) { NotifierPrefix ## Notifier.addListener(newListener); } \
void addAsyncCoalesced ## EventPrefix ## Listener(AsyncListener* newListener) { NotifierPrefix ## Notifier.addAsyncCoalescedListener(newListener); } \
void removeAsync ## EventPrefix ## Listener(AsyncListener* listener) { if(RemoveCheck) NotifierPrefix ## Notifier.removeListener(listener); };



template<typename MessageClass,class CriticalSectionToUse = juce::CriticalSection>
class QueuedNotifier:
	public  juce::AsyncUpdater
{
public:

    QueuedNotifier(int _maxSize, bool _dropMessageOnOverflow = true) :
    dropMessageOnOverflow(_dropMessageOnOverflow),
    fifo(_maxSize)
    {
        maxSize = _maxSize;

    }

    bool dropMessageOnOverflow;

    virtual ~QueuedNotifier() {
        isBeingDestroyed.store(true, std::memory_order_release);
        cancelPendingUpdate();
        const typename QueueLock::ScopedLockType lock(queueLock);
        listeners.clear();
        lastListeners.clear();
        messageQueue.clear();
        coalescedMessage.reset();
        fifo.reset();
        //cancelPendingUpdate();
    }



    class Listener{
    public:
        virtual ~Listener(){}
        virtual void newMessage(const MessageClass&) = 0;
    };

    // Keep ordinary listeners synchronous when requested, but allow display
    // listeners to receive only the latest value on the next message-loop turn.
    void addMessage( MessageClass * msg,bool forceSendNow = false, bool deferCoalesced = false){
        if (msg == nullptr)
            return;

        if (isBeingDestroyed.load(std::memory_order_acquire))
        {
            delete msg;
            return;
        }

        if(listeners.size()==0 && lastListeners.size()==0){
            delete msg;
            return;
        }

        if (auto* mm = juce::MessageManager::getInstanceWithoutCreating())
            forceSendNow |= mm->isThisTheMessageThread();

        if(forceSendNow){
            listeners.call(&Listener::newMessage,*msg);
            if (deferCoalesced && lastListeners.size() > 0)
            {
                {
                    const typename QueueLock::ScopedLockType lock(queueLock);
                    coalescedMessage.reset(msg);
                }
                triggerAsyncUpdate();
                return;
            }
            {
                const typename QueueLock::ScopedLockType lock(queueLock);
                coalescedMessage.reset();
            }
            lastListeners.call(&Listener::newMessage,*msg);
            delete msg;
            return;
        }

        // add if we are in a decent array size
        int start1{},size1{},start2{},size2{};

        for (;;)
        {
            if (isBeingDestroyed.load(std::memory_order_acquire))
            {
                delete msg;
                return;
            }

            {
                const typename QueueLock::ScopedLockType lock(queueLock);
                fifo.prepareToWrite(1, start1, size1, start2, size2);

                if (size1 > 0)
                {
                    if (messageQueue.size() <= start1)
                    {
                        while (messageQueue.size() <= start1)
                            messageQueue.add(nullptr);
                    }

                    messageQueue.set(start1, msg);

                    fifo.finishedWrite (size1 + size2);
                    break;
                }
            }

            if (dropMessageOnOverflow)
            {
                delete msg;
                return;
            }

            juce::Thread::sleep(10);
        }

        triggerAsyncUpdate();
    }

    // allow to stack all values or get oly last updated value
    void addListener(Listener* newListener) 
    {
		if (isBeingDestroyed.load(std::memory_order_acquire))
			return;
        
        listeners.add(newListener); 
    }

    void addAsyncCoalescedListener(Listener* newListener) 
    {
		if (isBeingDestroyed.load(std::memory_order_acquire))
			return;
		
        lastListeners.add(newListener);
	}

    void removeListener(Listener* listener) 
    { 
        if (isBeingDestroyed.load(std::memory_order_acquire))
			return;
        
        listeners.remove(listener);
        lastListeners.remove(listener); 
    }

    void clearQueue()
    {
        const typename QueueLock::ScopedLockType lock(queueLock);
        messageQueue.clear();
        coalescedMessage.reset();
        fifo.reset();
    }

private:

    void handleAsyncUpdate() override
    {
        if (isBeingDestroyed.load(std::memory_order_acquire))
            return;

        // Take ownership of ready messages before releasing queueLock. Once
        // finishedRead() makes a slot writable, a producer may immediately
        // reuse it; leaving the message owned by messageQueue would let set()
        // delete it while listeners are still reading it.
        juce::OwnedArray<MessageClass> messagesToDeliver;
        std::unique_ptr<MessageClass> deferredMessage;
        MessageClass* lastMessage = nullptr;

        {
            const typename QueueLock::ScopedLockType lock(queueLock);
            deferredMessage = std::move(coalescedMessage);

            int start1{},size1{},start2{},size2{};
            fifo.prepareToRead(fifo.getNumReady(), start1, size1, start2, size2);

            bool outOfRange = false;

            for(int i = start1 ;i <start1+ size1 ; ++i)
            {
                if ((unsigned) i >= (unsigned) messageQueue.size())
                {
                    outOfRange = true;
                    break;
                }

                if (auto* message = messageQueue.getUnchecked(i))
                {
                    messagesToDeliver.add(message);
                    messageQueue.set(i, nullptr, false);
                    lastMessage = message;
                }
            }

            if (! outOfRange)
            {
                for(int i = start2 ;i <start2+ size2 ; ++i)
                {
                    if ((unsigned) i >= (unsigned) messageQueue.size())
                    {
                        outOfRange = true;
                        break;
                    }

                    if (auto* message = messageQueue.getUnchecked(i))
                    {
                        messagesToDeliver.add(message);
                        messageQueue.set(i, nullptr, false);
                        lastMessage = message;
                    }
                }
            }

            if (outOfRange)
            {
                messageQueue.clear();
                fifo.reset();
                return;
            }

            fifo.finishedRead(size1 + size2);
        }

        for (auto* message : messagesToDeliver)
            listeners.call(&Listener::newMessage,*message);

        if (deferredMessage != nullptr)
            lastListeners.call(&Listener::newMessage,*deferredMessage);
        else if (lastMessage != nullptr)
            lastListeners.call(&Listener::newMessage,*lastMessage);
    }



    using QueueLock = CriticalSectionToUse;

    juce::AbstractFifo fifo;
    int maxSize;
    juce::OwnedArray<MessageClass,CriticalSectionToUse> messageQueue;
    QueueLock queueLock;
    std::unique_ptr<MessageClass> coalescedMessage;
    std::atomic<bool> isBeingDestroyed { false };

    using ThreadSafeListenerArray = juce::Array<Listener*, CriticalSectionToUse>;
    juce::ListenerList<Listener, ThreadSafeListenerArray> listeners;
    juce::ListenerList<Listener, ThreadSafeListenerArray> lastListeners;
};
