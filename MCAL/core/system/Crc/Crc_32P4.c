/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */

/* @req SWS_CRC_00024 */
/* @req SWS_CRC_00022 */
#include "Crc.h"

/* Check that the CRC 32 P4 configuration is consistent */
#ifndef CRC_32P4_MODE
#error "CRC_32P4_MODE is not defined in configuration"
#elif CRC_32P4_MODE == CRC_32P4_HARDWARE
#error "CRC_32P4_MODE is set to CRC_32_HARDWARE which isn't supported"
#endif


/* For CRC32  */
#define CRC32P4_START_VALUE    0xFFFFFFFFU
#define CRC32P4_POLYNOMIAL     0xF4ACFB13U
#define CRC32P4_XOR            0xFFFFFFFFU


/*lint -esym(9003, crc32p4_tab) MISRA Exception: Readability (MISRA 2012 Rule 8.9, advisory) */
#if CRC_32P4_MODE == CRC_32P4_TABLE
static const uint32 crc32p4_tab[] = {
        0x00000000U, 0x30850FF5U, 0x610A1FEAU, 0x518F101FU, 0xC2143FD4U, 0xF2913021U, 0xA31E203EU, 0x939B2FCBU,
        0x159615F7U, 0x25131A02U, 0x749C0A1DU, 0x441905E8U, 0xD7822A23U, 0xE70725D6U, 0xB68835C9U, 0x860D3A3CU,
        0x2B2C2BEEU, 0x1BA9241BU, 0x4A263404U, 0x7AA33BF1U, 0xE938143AU, 0xD9BD1BCFU, 0x88320BD0U, 0xB8B70425U,
        0x3EBA3E19U, 0x0E3F31ECU, 0x5FB021F3U, 0x6F352E06U, 0xFCAE01CDU, 0xCC2B0E38U, 0x9DA41E27U, 0xAD2111D2U,
        0x565857DCU, 0x66DD5829U, 0x37524836U, 0x07D747C3U, 0x944C6808U, 0xA4C967FDU, 0xF54677E2U, 0xC5C37817U,
        0x43CE422BU, 0x734B4DDEU, 0x22C45DC1U, 0x12415234U, 0x81DA7DFFU, 0xB15F720AU, 0xE0D06215U, 0xD0556DE0U,
        0x7D747C32U, 0x4DF173C7U, 0x1C7E63D8U, 0x2CFB6C2DU, 0xBF6043E6U, 0x8FE54C13U, 0xDE6A5C0CU, 0xEEEF53F9U,
        0x68E269C5U, 0x58676630U, 0x09E8762FU, 0x396D79DAU, 0xAAF65611U, 0x9A7359E4U, 0xCBFC49FBU, 0xFB79460EU,
        0xACB0AFB8U, 0x9C35A04DU, 0xCDBAB052U, 0xFD3FBFA7U, 0x6EA4906CU, 0x5E219F99U, 0x0FAE8F86U, 0x3F2B8073U,
        0xB926BA4FU, 0x89A3B5BAU, 0xD82CA5A5U, 0xE8A9AA50U, 0x7B32859BU, 0x4BB78A6EU, 0x1A389A71U, 0x2ABD9584U,
        0x879C8456U, 0xB7198BA3U, 0xE6969BBCU, 0xD6139449U, 0x4588BB82U, 0x750DB477U, 0x2482A468U, 0x1407AB9DU,
        0x920A91A1U, 0xA28F9E54U, 0xF3008E4BU, 0xC38581BEU, 0x501EAE75U, 0x609BA180U, 0x3114B19FU, 0x0191BE6AU,
        0xFAE8F864U, 0xCA6DF791U, 0x9BE2E78EU, 0xAB67E87BU, 0x38FCC7B0U, 0x0879C845U, 0x59F6D85AU, 0x6973D7AFU,
        0xEF7EED93U, 0xDFFBE266U, 0x8E74F279U, 0xBEF1FD8CU, 0x2D6AD247U, 0x1DEFDDB2U, 0x4C60CDADU, 0x7CE5C258U,
        0xD1C4D38AU, 0xE141DC7FU, 0xB0CECC60U, 0x804BC395U, 0x13D0EC5EU, 0x2355E3ABU, 0x72DAF3B4U, 0x425FFC41U,
        0xC452C67DU, 0xF4D7C988U, 0xA558D997U, 0x95DDD662U, 0x0646F9A9U, 0x36C3F65CU, 0x674CE643U, 0x57C9E9B6U,
        0xC8DF352FU, 0xF85A3ADAU, 0xA9D52AC5U, 0x99502530U, 0x0ACB0AFBU, 0x3A4E050EU, 0x6BC11511U, 0x5B441AE4U,
        0xDD4920D8U, 0xEDCC2F2DU, 0xBC433F32U, 0x8CC630C7U, 0x1F5D1F0CU, 0x2FD810F9U, 0x7E5700E6U, 0x4ED20F13U,
        0xE3F31EC1U, 0xD3761134U, 0x82F9012BU, 0xB27C0EDEU, 0x21E72115U, 0x11622EE0U, 0x40ED3EFFU, 0x7068310AU,
        0xF6650B36U, 0xC6E004C3U, 0x976F14DCU, 0xA7EA1B29U, 0x347134E2U, 0x04F43B17U, 0x557B2B08U, 0x65FE24FDU,
        0x9E8762F3U, 0xAE026D06U, 0xFF8D7D19U, 0xCF0872ECU, 0x5C935D27U, 0x6C1652D2U, 0x3D9942CDU, 0x0D1C4D38U,
        0x8B117704U, 0xBB9478F1U, 0xEA1B68EEU, 0xDA9E671BU, 0x490548D0U, 0x79804725U, 0x280F573AU, 0x188A58CFU,
        0xB5AB491DU, 0x852E46E8U, 0xD4A156F7U, 0xE4245902U, 0x77BF76C9U, 0x473A793CU, 0x16B56923U, 0x263066D6U,
        0xA03D5CEAU, 0x90B8531FU, 0xC1374300U, 0xF1B24CF5U, 0x6229633EU, 0x52AC6CCBU, 0x03237CD4U, 0x33A67321U,
        0x646F9A97U, 0x54EA9562U, 0x0565857DU, 0x35E08A88U, 0xA67BA543U, 0x96FEAAB6U, 0xC771BAA9U, 0xF7F4B55CU,
        0x71F98F60U, 0x417C8095U, 0x10F3908AU, 0x20769F7FU, 0xB3EDB0B4U, 0x8368BF41U, 0xD2E7AF5EU, 0xE262A0ABU,
        0x4F43B179U, 0x7FC6BE8CU, 0x2E49AE93U, 0x1ECCA166U, 0x8D578EADU, 0xBDD28158U, 0xEC5D9147U, 0xDCD89EB2U,
        0x5AD5A48EU, 0x6A50AB7BU, 0x3BDFBB64U, 0x0B5AB491U, 0x98C19B5AU, 0xA84494AFU, 0xF9CB84B0U, 0xC94E8B45U,
        0x3237CD4BU, 0x02B2C2BEU, 0x533DD2A1U, 0x63B8DD54U, 0xF023F29FU, 0xC0A6FD6AU, 0x9129ED75U, 0xA1ACE280U,
        0x27A1D8BCU, 0x1724D749U, 0x46ABC756U, 0x762EC8A3U, 0xE5B5E768U, 0xD530E89DU, 0x84BFF882U, 0xB43AF777U,
        0x191BE6A5U, 0x299EE950U, 0x7811F94FU, 0x4894F6BAU, 0xDB0FD971U, 0xEB8AD684U, 0xBA05C69BU, 0x8A80C96EU,
        0x0C8DF352U, 0x3C08FCA7U, 0x6D87ECB8U, 0x5D02E34DU, 0xCE99CC86U, 0xFE1CC373U, 0xAF93D36CU, 0x9F16DC99U };

#endif

#if CRC_32P4_MODE == CRC_32P4_RUNTIME

/* Reflects the result bit by bit */
static INLINE uint32 reflectResult(uint32 data)
{
    uint32 reflection = 0x00000000U;
    uint8 bit;
    uint32 tmpData = data;

    for (bit = 0; bit < 32; bit++) {

        /* Set reflection  */
        if ((tmpData & 0x01U) != 0) {
            reflection |= (1UL << (31UL - bit));
        }

        tmpData = tmpData >> 1;
    }

    return reflection;

}


/* Reflects the indata bit by bit */
static INLINE uint8 reflectInData(uint8 data)
{
    uint8 reflection = 0x00;
    uint8 bit;
    uint32 tmpData = data;

    for (bit = 0; bit < 8; bit++) {

        if ((tmpData & 0x01U) != 0) {
            /*lint -e{734} Lint exception:  False warning since bit <= 7 */
            reflection |= (1U << (7U-bit));
        }

        tmpData = (tmpData >> 1U);
    }

    return reflection;

}


static uint32 CalculateCRC32P4(const uint8* message, uint32 nBytes, uint32 start)
{
    uint32 remainder = reflectResult(start);
    uint8  bit;
    const uint32  topbit = 0x80000000U;

    /* Perform modulo-2 division, a byte at a time. */
    for (uint32 byte = 0; byte < nBytes; byte++) {
        /* Bring the next byte into the remainder. */
        uint32 reflectedData = reflectInData(*message);
        remainder ^= (reflectedData << 24u);
        message++;

        /* Perform modulo-2 division, a bit at a time. */
        for (bit = 8; bit > 0; bit--) {

            /* Try to divide the current data bit. */
            if ((remainder & topbit) != 0) {
                remainder = (remainder << 1u) ^ CRC32P4_POLYNOMIAL;
            }
            else {
                remainder = (remainder << 1u);
            }
        }
    }

    return reflectResult(remainder);
}

#endif



/* @req SWS_CRC_00056 */
/* @req SWS_CRC_00057 */
/* @req SWS_CRC_00058 */
/* @req SWS_CRC_00059 */
uint32 Crc_CalculateCRC32P4( const uint8* Crc_DataPtr, uint32 Crc_Length, uint32 Crc_StartValue32, boolean Crc_IsFirstCall ) {

    uint32 crc = 0; /* Default return value if NULL pointer */

    if (Crc_DataPtr != NULL) {

        crc = (TRUE == Crc_IsFirstCall) ? CRC32P4_START_VALUE : (Crc_StartValue32 ^ CRC32P4_XOR);

#if CRC_32P4_MODE == CRC_32P4_RUNTIME
        crc = CalculateCRC32P4(Crc_DataPtr, Crc_Length, crc);
#elif CRC_32P4_MODE == CRC_32P4_TABLE
        for( uint32 byte = 0; byte < Crc_Length; byte++) {
            crc = ((crc >> 8) & 0x00FFFFFFU) ^ crc32p4_tab[(crc ^ *Crc_DataPtr) & 0xFFU];
            Crc_DataPtr++;
        }
#endif

        crc = crc ^ CRC32P4_XOR;
    }

    return crc;
}
