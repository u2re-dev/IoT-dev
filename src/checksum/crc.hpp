//////////////////////////////////////////////////////////////////////////////
////////////////////// CRC 32/64 calculator for x86-64 ///////////////////////
//////////////////////////////////////////////////////////////////////////////
#pragma once
// Don't use this file in user's source, it's internal details and Don't change anything!
#include <stdint.h>
#include <stddef.h>

static const unsigned int crc32_table[] = {
    (uint32_t)(0x00000000), (uint32_t)(0x77073096), (uint32_t)(0xee0e612c), (uint32_t)(0x990951ba),
    (uint32_t)(0x076dc419), (uint32_t)(0x706af48f), (uint32_t)(0xe963a535), (uint32_t)(0x9e6495a3),
    (uint32_t)(0x0edb8832), (uint32_t)(0x79dcb8a4), (uint32_t)(0xe0d5e91e), (uint32_t)(0x97d2d988),
    (uint32_t)(0x09b64c2b), (uint32_t)(0x7eb17cbd), (uint32_t)(0xe7b82d07), (uint32_t)(0x90bf1d91),
    (uint32_t)(0x1db71064), (uint32_t)(0x6ab020f2), (uint32_t)(0xf3b97148), (uint32_t)(0x84be41de),
    (uint32_t)(0x1adad47d), (uint32_t)(0x6ddde4eb), (uint32_t)(0xf4d4b551), (uint32_t)(0x83d385c7),
    (uint32_t)(0x136c9856), (uint32_t)(0x646ba8c0), (uint32_t)(0xfd62f97a), (uint32_t)(0x8a65c9ec),
    (uint32_t)(0x14015c4f), (uint32_t)(0x63066cd9), (uint32_t)(0xfa0f3d63), (uint32_t)(0x8d080df5),
    (uint32_t)(0x3b6e20c8), (uint32_t)(0x4c69105e), (uint32_t)(0xd56041e4), (uint32_t)(0xa2677172),
    (uint32_t)(0x3c03e4d1), (uint32_t)(0x4b04d447), (uint32_t)(0xd20d85fd), (uint32_t)(0xa50ab56b),
    (uint32_t)(0x35b5a8fa), (uint32_t)(0x42b2986c), (uint32_t)(0xdbbbc9d6), (uint32_t)(0xacbcf940),
    (uint32_t)(0x32d86ce3), (uint32_t)(0x45df5c75), (uint32_t)(0xdcd60dcf), (uint32_t)(0xabd13d59),
    (uint32_t)(0x26d930ac), (uint32_t)(0x51de003a), (uint32_t)(0xc8d75180), (uint32_t)(0xbfd06116),
    (uint32_t)(0x21b4f4b5), (uint32_t)(0x56b3c423), (uint32_t)(0xcfba9599), (uint32_t)(0xb8bda50f),
    (uint32_t)(0x2802b89e), (uint32_t)(0x5f058808), (uint32_t)(0xc60cd9b2), (uint32_t)(0xb10be924),
    (uint32_t)(0x2f6f7c87), (uint32_t)(0x58684c11), (uint32_t)(0xc1611dab), (uint32_t)(0xb6662d3d),
    (uint32_t)(0x76dc4190), (uint32_t)(0x01db7106), (uint32_t)(0x98d220bc), (uint32_t)(0xefd5102a),
    (uint32_t)(0x71b18589), (uint32_t)(0x06b6b51f), (uint32_t)(0x9fbfe4a5), (uint32_t)(0xe8b8d433),
    (uint32_t)(0x7807c9a2), (uint32_t)(0x0f00f934), (uint32_t)(0x9609a88e), (uint32_t)(0xe10e9818),
    (uint32_t)(0x7f6a0dbb), (uint32_t)(0x086d3d2d), (uint32_t)(0x91646c97), (uint32_t)(0xe6635c01),
    (uint32_t)(0x6b6b51f4), (uint32_t)(0x1c6c6162), (uint32_t)(0x856530d8), (uint32_t)(0xf262004e),
    (uint32_t)(0x6c0695ed), (uint32_t)(0x1b01a57b), (uint32_t)(0x8208f4c1), (uint32_t)(0xf50fc457),
    (uint32_t)(0x65b0d9c6), (uint32_t)(0x12b7e950), (uint32_t)(0x8bbeb8ea), (uint32_t)(0xfcb9887c),
    (uint32_t)(0x62dd1ddf), (uint32_t)(0x15da2d49), (uint32_t)(0x8cd37cf3), (uint32_t)(0xfbd44c65),
    (uint32_t)(0x4db26158), (uint32_t)(0x3ab551ce), (uint32_t)(0xa3bc0074), (uint32_t)(0xd4bb30e2),
    (uint32_t)(0x4adfa541), (uint32_t)(0x3dd895d7), (uint32_t)(0xa4d1c46d), (uint32_t)(0xd3d6f4fb),
    (uint32_t)(0x4369e96a), (uint32_t)(0x346ed9fc), (uint32_t)(0xad678846), (uint32_t)(0xda60b8d0),
    (uint32_t)(0x44042d73), (uint32_t)(0x33031de5), (uint32_t)(0xaa0a4c5f), (uint32_t)(0xdd0d7cc9),
    (uint32_t)(0x5005713c), (uint32_t)(0x270241aa), (uint32_t)(0xbe0b1010), (uint32_t)(0xc90c2086),
    (uint32_t)(0x5768b525), (uint32_t)(0x206f85b3), (uint32_t)(0xb966d409), (uint32_t)(0xce61e49f),
    (uint32_t)(0x5edef90e), (uint32_t)(0x29d9c998), (uint32_t)(0xb0d09822), (uint32_t)(0xc7d7a8b4),
    (uint32_t)(0x59b33d17), (uint32_t)(0x2eb40d81), (uint32_t)(0xb7bd5c3b), (uint32_t)(0xc0ba6cad),
    (uint32_t)(0xedb88320), (uint32_t)(0x9abfb3b6), (uint32_t)(0x03b6e20c), (uint32_t)(0x74b1d29a),
    (uint32_t)(0xead54739), (uint32_t)(0x9dd277af), (uint32_t)(0x04db2615), (uint32_t)(0x73dc1683),
    (uint32_t)(0xe3630b12), (uint32_t)(0x94643b84), (uint32_t)(0x0d6d6a3e), (uint32_t)(0x7a6a5aa8),
    (uint32_t)(0xe40ecf0b), (uint32_t)(0x9309ff9d), (uint32_t)(0x0a00ae27), (uint32_t)(0x7d079eb1),
    (uint32_t)(0xf00f9344), (uint32_t)(0x8708a3d2), (uint32_t)(0x1e01f268), (uint32_t)(0x6906c2fe),
    (uint32_t)(0xf762575d), (uint32_t)(0x806567cb), (uint32_t)(0x196c3671), (uint32_t)(0x6e6b06e7),
    (uint32_t)(0xfed41b76), (uint32_t)(0x89d32be0), (uint32_t)(0x10da7a5a), (uint32_t)(0x67dd4acc),
    (uint32_t)(0xf9b9df6f), (uint32_t)(0x8ebeeff9), (uint32_t)(0x17b7be43), (uint32_t)(0x60b08ed5),
    (uint32_t)(0xd6d6a3e8), (uint32_t)(0xa1d1937e), (uint32_t)(0x38d8c2c4), (uint32_t)(0x4fdff252),
    (uint32_t)(0xd1bb67f1), (uint32_t)(0xa6bc5767), (uint32_t)(0x3fb506dd), (uint32_t)(0x48b2364b),
    (uint32_t)(0xd80d2bda), (uint32_t)(0xaf0a1b4c), (uint32_t)(0x36034af6), (uint32_t)(0x41047a60),
    (uint32_t)(0xdf60efc3), (uint32_t)(0xa867df55), (uint32_t)(0x316e8eef), (uint32_t)(0x4669be79),
    (uint32_t)(0xcb61b38c), (uint32_t)(0xbc66831a), (uint32_t)(0x256fd2a0), (uint32_t)(0x5268e236),
    (uint32_t)(0xcc0c7795), (uint32_t)(0xbb0b4703), (uint32_t)(0x220216b9), (uint32_t)(0x5505262f),
    (uint32_t)(0xc5ba3bbe), (uint32_t)(0xb2bd0b28), (uint32_t)(0x2bb45a92), (uint32_t)(0x5cb36a04),
    (uint32_t)(0xc2d7ffa7), (uint32_t)(0xb5d0cf31), (uint32_t)(0x2cd99e8b), (uint32_t)(0x5bdeae1d),
    (uint32_t)(0x9b64c2b0), (uint32_t)(0xec63f226), (uint32_t)(0x756aa39c), (uint32_t)(0x026d930a),
    (uint32_t)(0x9c0906a9), (uint32_t)(0xeb0e363f), (uint32_t)(0x72076785), (uint32_t)(0x05005713),
    (uint32_t)(0x95bf4a82), (uint32_t)(0xe2b87a14), (uint32_t)(0x7bb12bae), (uint32_t)(0x0cb61b38),
    (uint32_t)(0x92d28e9b), (uint32_t)(0xe5d5be0d), (uint32_t)(0x7cdcefb7), (uint32_t)(0x0bdbdf21),
    (uint32_t)(0x86d3d2d4), (uint32_t)(0xf1d4e242), (uint32_t)(0x68ddb3f8), (uint32_t)(0x1fda836e),
    (uint32_t)(0x81be16cd), (uint32_t)(0xf6b9265b), (uint32_t)(0x6fb077e1), (uint32_t)(0x18b74777),
    (uint32_t)(0x88085ae6), (uint32_t)(0xff0f6a70), (uint32_t)(0x66063bca), (uint32_t)(0x11010b5c),
    (uint32_t)(0x8f659eff), (uint32_t)(0xf862ae69), (uint32_t)(0x616bffd3), (uint32_t)(0x166ccf45),
    (uint32_t)(0xa00ae278), (uint32_t)(0xd70dd2ee), (uint32_t)(0x4e048354), (uint32_t)(0x3903b3c2),
    (uint32_t)(0xa7672661), (uint32_t)(0xd06016f7), (uint32_t)(0x4969474d), (uint32_t)(0x3e6e77db),
    (uint32_t)(0xaed16a4a), (uint32_t)(0xd9d65adc), (uint32_t)(0x40df0b66), (uint32_t)(0x37d83bf0),
    (uint32_t)(0xa9bcae53), (uint32_t)(0xdebb9ec5), (uint32_t)(0x47b2cf7f), (uint32_t)(0x30b5ffe9),
    (uint32_t)(0xbdbdf21c), (uint32_t)(0xcabac28a), (uint32_t)(0x53b39330), (uint32_t)(0x24b4a3a6),
    (uint32_t)(0xbad03605), (uint32_t)(0xcdd70693), (uint32_t)(0x54de5729), (uint32_t)(0x23d967bf),
    (uint32_t)(0xb3667a2e), (uint32_t)(0xc4614ab8), (uint32_t)(0x5d681b02), (uint32_t)(0x2a6f2b94),
    (uint32_t)(0xb40bbe37), (uint32_t)(0xc30c8ea1), (uint32_t)(0x5a05df1b), (uint32_t)(0x2d02ef8d)
};

uint32_t _crc32_(uint8_t const *buf, size_t len, uint32_t init = 0xFFFFFFFF)
{
  uint32_t crc = init;
  for (uint I=0;I<len;I++) {
      crc = crc32_table[(crc ^ buf[I]) & 0xff] ^ ((crc) >> 8);//(crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
  }
  return (crc ^ 0xFFFFFFFF);
}



static const unsigned int xcrc32_table[] =
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/*

@deftypefn Extension {unsigned int} crc32 (const unsigned char *@var{buf}, @
  int @var{len}, unsigned int @var{init})

Compute the 32-bit CRC of @var{buf} which has length @var{len}.  The
starting value is @var{init}; this may be used to compute the CRC of
data split across multiple buffers by passing the return value of each
call as the @var{init} parameter of the next.

This is used by the @command{gdb} remote protocol for the @samp{qCRC}
command.  In order to get the same results as gdb for a block of data,
you must pass the first CRC parameter as @code{0xffffffff}.

This CRC can be specified as:

  Width  : 32
  Poly   : 0x04c11db7
  Init   : parameter, typically 0xffffffff
  RefIn  : false
  RefOut : false
  XorOut : 0

This differs from the "standard" CRC-32 algorithm in that the values
are not reflected, and there is no final XOR value.  These differences
make it easy to compose the values of multiple blocks.

@end deftypefn

*/

uint32_t _xcrc32_(uint8_t const *buf, size_t len, uint32_t init = 0xffffffff)
{
  uint32_t crc = init;
  for (uint I=0;I<len;I++) {
      crc = (crc << 8) ^ xcrc32_table[((crc >> 24) ^ buf[I]) & 0xff];
  }
  return crc;
}
