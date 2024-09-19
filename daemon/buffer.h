#ifndef BUFFER_H
#define BUFFER_H

#include <linux/netlink.h>
#include <linux/genetlink.h>

#include <dispenser.h>

#include "daemon.h"

class Buffer
{
public:
	Buffer(long initial_size = PAGE_SIZE);
	virtual ~Buffer();

	long resize(long new_size);
	long allocate(long new_size);

	long size() { return m_iSize; }
	long capacity() { return m_iCapacity; }
	long capacityLeft() { return m_iCapacity - m_iIterator; }
	char *begin() { seekSet(0); return cur(); }
	//char *cur() { return atEnd() ? nullptr : m_pBuffer + m_iIterator; }
	char *cur() { return m_pBuffer + m_iIterator; }
	long seekSet(long pos);
	long seekRel(long seek) { return seekSet(seek + m_iIterator); }
	bool align(unsigned long bits);
	long spaceLeft() { return m_iSize - m_iIterator; }
	bool atEnd() { return !(m_iIterator < m_iSize); }
	void clear() { m_iSize = m_iIterator = 0; }

	Buffer &operator>>(nlmsghdr **s);
	Buffer &operator>>(genlmsghdr **s);
	Buffer &operator>>(nlattr **s);


private:
	char *m_pBuffer = nullptr;
	long m_iCapacity = 0;
	long m_iSize = 0;
	long m_iIterator = 0;
};

#endif // BUFFER_H
