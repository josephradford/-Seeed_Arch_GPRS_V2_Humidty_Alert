#include "circbuff.h"

CircBuff::CircBuff(uint16_t buffSize)
{
    // set up the buffer with parsed size
    m_buffSize = buffSize;
    m_buf = new unsigned char [m_buffSize];
    for (int i = 0; i < m_buffSize; i++) {
        m_buf[i] = 0;
    }

    // init indexes
    m_start = 0;
    m_end   = 0;
}

CircBuff::~CircBuff()
{
    delete m_buf;
}

void CircBuff::putc(unsigned char c)
{
    // get the remaining space left in the buffer by checking end and start idx
    uint16_t remSize = remainingSize();

    // check we have enough room for the new array passed in
    if (remSize == 0) {
        return;
    }

    // else copy the byte in
    m_buf[m_end++] = c;
    if (m_end == m_buffSize) {
        m_end = 0;  // wrap around
    }

}

void CircBuff::add(unsigned char *s)
{
    // check if can write? How to check if we have connected.
    uint16_t sSize = 0, i = 0, j = 0;

    // get the length of the passed array (it should be null terminated, but include max buffer size for caution)
    for (sSize = 0; (sSize < m_buffSize) && (s[sSize] != 0); sSize++);

    // get the remaining space left in the buffer by checking end and start idx
    uint16_t remSize = remainingSize();

    // check we have enough room for the new array passed in
    if (sSize > remSize) {
        return;
    }

    // copy the array in
    for (i = 0; i < sSize; i++) {
        m_buf[m_end++] = s[i];
        if (m_end == m_buffSize) {
            m_end = 0;  // wrap around
        }
    }
}

uint16_t CircBuff::remainingSize()
{
    // get the remaining space left in the buffer by checking end and start idx
    uint16_t retval = 0;
    if (m_start == m_end) {
        retval = m_buffSize;
    }
    else if (m_start < m_end) {
        // the distance between the start and end point
        // subtract that from the total length of the buffer
        retval = m_buffSize - (m_end - m_start);
    }
    else {
        // the amount of space left is whatever is between the end point and the start point
        retval = m_start - m_end;
    }
    return retval;
}

uint16_t CircBuff::read(unsigned char *s, uint16_t len)
{
    if (m_start == m_end) {
        return 0; // there is nothing stored in the circular buffer
    }

    // start copying the desired amount over
    for (int i = 0; i < len; i++) {
        s[i] = m_buf[m_start++];
        
        if (m_start == m_buffSize) {
            m_start = 0;    // wrap start pointer
        }

        if (m_start == m_end) {
            s[++i] = 0;
            return (i); // we have reached the end of the buffer
        }
    }
    return len;
}


