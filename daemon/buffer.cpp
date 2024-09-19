#include <QtGlobal>
#include <cstring>

#include "buffer.h"

#define ALIGN(len, to) (((len) + (to) - 1) & ~((to) - 1))

Buffer::Buffer(long initial_size)
{
	allocate(initial_size);
}

Buffer::~Buffer()
{
	delete m_pBuffer;
	m_pBuffer = nullptr;
	m_iCapacity = 0;
	m_iIterator = 0;
	m_iSize = 0;
}

long Buffer::resize(long new_size)
{
	long size = new_size;
	if (new_size < 0)
		size = 0;
	if (new_size > m_iCapacity)
		size = m_iCapacity;

	m_iSize = size;
	if (m_iIterator > m_iSize)
		m_iIterator = m_iSize;

	return m_iSize;
}

long Buffer::allocate(long new_size)
{
	long size = new_size;
	char *pBuffer = nullptr;

	if (size < 0 || (unsigned)size < PAGE_SIZE)
		size = PAGE_SIZE;

	size = PAGE_ALIGN(size);

	if (size == m_iCapacity)
		return m_iCapacity;

	pBuffer = new char[size];
	Q_CHECK_PTR(pBuffer);

	if (pBuffer) {
		if (m_pBuffer) {
			memcpy(pBuffer, m_pBuffer, size < m_iCapacity ? size : m_iCapacity);
			delete m_pBuffer;
		}
		m_pBuffer = pBuffer;
		if (size < m_iSize)
			m_iSize = size;
		m_iCapacity = size;
		if (m_iIterator > m_iSize)
			m_iIterator = m_iSize;
	}

	return m_iCapacity;
}

long Buffer::seekSet(long pos)
{
	if (pos < 0  || pos > m_iSize) //Check bounds
		return -1;

	m_iIterator = pos;

	return m_iIterator; //Return new pos
}

bool Buffer::align(unsigned long bits)
{
	long new_pos = ALIGN(m_iIterator, bits);
	if (new_pos > m_iSize)
		new_pos = m_iSize;

	m_iIterator = new_pos;

	return atEnd();
/*
	NLA_ALIGN(g);
	NLA_ALIGNTO(g);
	NLMSG_ALIGN(h);
	NLMSG_ALIGNTO(g);
	NLMSG_NEXT(g,k);
	NLMSG_DATA(g);
*/
}

Buffer &Buffer::operator>>(nlmsghdr **s)
{
	seekSet(NLMSG_ALIGN(m_iIterator));
	nlmsghdr *cur_pos = (nlmsghdr *) cur();

	if (seekRel(NLMSG_HDRLEN) == -1)
		*s = nullptr;
	else
		*s = cur_pos;

	return *this;
}

Buffer &Buffer::operator>>(genlmsghdr **s)
{
	seekSet(NLMSG_ALIGN(m_iIterator));
	genlmsghdr *cur_pos = (genlmsghdr *) cur();

	if (seekRel(GENL_HDRLEN) == -1)
		*s = nullptr;
	else
		*s = cur_pos;

	return *this;
}

Buffer &Buffer::operator>>(nlattr **s)
{
	seekSet(NLA_ALIGN(m_iIterator));
	nlattr *cur_pos = (nlattr *) cur();

	//if (seekRel(NLA_HDRLEN) == -1)
	if (seekRel(cur_pos->nla_len) == -1) {
		*s = nullptr;
	} else {
		*s = cur_pos;
		align(NLA_ALIGNTO);
	}

	return *this;
}
