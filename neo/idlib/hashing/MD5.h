#ifndef __MD5_H__
#define __MD5_H__

/*
===============================================================================

        Calculates a checksum for a block of data
        using the MD5 message-digest algorithm.

===============================================================================
*/

uint32_t MD5_BlockChecksum(const void* data, size_t length);

#endif /* !__MD5_H__ */
