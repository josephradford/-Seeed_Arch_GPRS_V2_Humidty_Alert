#ifndef __CIRC_BUFF_H__
#define __CIRC_BUFF_H__

#include "mbed.h"

/*!
 * \brief The CircBuff class writes in and reads out byte arrays into a circular buffer
 */
class CircBuff {
public:
    CircBuff(uint16_t buffSize = 256);
    ~CircBuff();

    /*!
     * \brief putc adds a single byte, \a c, into the array
     * \param c is the byte copied into the circular buffer.
     */
    void putc(unsigned char c);

    /*!
     * \brief add adds \a s into the buffer, up until the NULL byte
     * \param s is the byte array copied into the buffer
     */
    void add(unsigned char *s);

    /*!
     * \brief read puts the current data from the buffer into \a s
     * \param s is the reference buffer to copy current data into. Caller's responsibility to make it the correct size
     * \param len is the number of bytes to copy into \a s
     * \return the number of bytes copied into \a s. Will be less than \a len if there was less data in the buffer.
     */
    uint16_t read(unsigned char *s, uint16_t len);
    bool dataAvailable() { return (m_start != m_end); }
    

private:
    uint16_t m_start;       ///< The start index of the circular buffer, where the current data starts
    uint16_t m_end;         ///< The end index, where the current data goes to
    unsigned char *m_buf;   ///< the byte array
    uint16_t m_buffSize;    ///< size of \a m_buf
    
    // helper
    uint16_t remainingSize();
};

#endif // __CIRC_BUFF_H__
