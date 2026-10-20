#include "test.h"
#include "gdosinfo/decode.h"

void test_decode(void)
{
    char buf[80];

    CHECK_STR(gdos_vgdos_name(-2L), "no GDOS");
    CHECK_STR(gdos_vgdos_name(0x5F46534DL), "FSMGDOS");
    CHECK_STR(gdos_vgdos_name(0x5F464E54L), "FONTGDOS");
    CHECK_STR(gdos_vgdos_name(0x66564449L), "fVDI");
    CHECK_STR(gdos_vgdos_name(0x0007E88AL), "Atari GDOS 1.1");
    CHECK_STR(gdos_vgdos_name(0x0007E864L), "AMC-GDOS");
    CHECK_STR(gdos_vgdos_name(0x0007E8BAL), "GEMINI special GDOS");
    CHECK_STR(gdos_vgdos_name(0x3E5D0957L), "ttf-gdos");
    CHECK_STR(gdos_vgdos_name(0x00123456L), "GDOS 1.0 or 1.2");

    CHECK_STR(gdos_fsmc_name("_SPD"), "SpeedoGDOS");
    CHECK_STR(gdos_fsmc_name("_FSM"), "FSMGDOS");
    CHECK_STR(gdos_fsmc_name("_FNT"), "FONTGDOS");
    CHECK_STR(gdos_fsmc_name("TTFG"), "ttf-gdos");
    CHECK_STR(gdos_fsmc_name("????"), "unknown vector GDOS");

    gdos_format_version("NVDI", 0x0410, buf, sizeof buf);
    CHECK_STR(buf, "NVDI 4.10");
    gdos_format_version("NVDI", 0x0500, buf, sizeof buf);
    CHECK_STR(buf, "NVDI 5.00");
    gdos_format_version("EdDI", 0x0110, buf, sizeof buf);
    CHECK_STR(buf, "EdDI 1.10");
    gdos_format_version("NVDI", 0x1012, buf, sizeof buf);       /* two-digit major */
    CHECK_STR(buf, "NVDI 10.12");

    gdos_format_fsmc("_SPD", 0x0400, -1, buf, sizeof buf);
    CHECK_STR(buf, "SpeedoGDOS 4.00, quality -1");
    gdos_format_fsmc("_SPD", 0x0500, 2, buf, sizeof buf);
    CHECK_STR(buf, "SpeedoGDOS 5.00, quality 2");
    gdos_format_fsmc("_SPD", 0x1000, 0, buf, sizeof buf);
    CHECK_STR(buf, "SpeedoGDOS 10.00, quality 0");

    gdos_format_os(0x0206, 0, buf, sizeof buf);
    CHECK_STR(buf, "TOS 2.06");
    gdos_format_os(0x0206, 1, buf, sizeof buf);
    CHECK_STR(buf, "EmuTOS or pTOS (TOS 2.06)");
    CHECK(strlen(buf) <= 30);                                   /* a form_alert() line */

    /* a buffer that is too small is cut, never overrun */
    gdos_format_version("NVDI", 0x0410, buf, 5);
    CHECK_STR(buf, "NVDI");
}
