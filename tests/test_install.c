#include <string.h>
#include "install/install.h"
#include "test.h"

static AboutFacts facts(void)
{
    AboutFacts f;

    memset(&f, 0, sizeof f);
    return f;
}

static void test_kind(void)
{
    AboutFacts f = facts();

    CHECK_INT(install_kind(&f), INSTALL_NONE);

    f.have_fsmc = 1;
    memcpy(f.fsmc_magic, "_SPD", 4);
    f.fsmc_version = 0x0400;
    CHECK_INT(install_kind(&f), INSTALL_BUILTIN);

    f.fsmc_version = 0x0501;                    /* a SpeedoGDOS from disk */
    CHECK_INT(install_kind(&f), INSTALL_DISK);

    f = facts();
    f.have_fsmc = 1;
    memcpy(f.fsmc_magic, "_FSM", 4);
    CHECK_INT(install_kind(&f), INSTALL_DISK);

    f = facts();                                /* FONTGDOS: bitmap fonts only */
    f.have_fsmc = 1;
    memcpy(f.fsmc_magic, "_FNT", 4);
    CHECK_INT(install_kind(&f), INSTALL_NONE);
    f.have_nvdi = 1;
    f.nvdi_bcd = 0x0250;
    CHECK_INT(install_kind(&f), INSTALL_NONE);
    f.nvdi_bcd = 0x0500;                        /* with an NVDI that has outlines */
    CHECK_INT(install_kind(&f), INSTALL_DISK);

    f = facts();
    f.have_nvdi = 1;
    f.nvdi_bcd = 0x0250;
    CHECK_INT(install_kind(&f), INSTALL_NONE);  /* NVDI 2 has no scalable fonts */
    f.nvdi_bcd = 0x0500;
    CHECK_INT(install_kind(&f), INSTALL_DISK);
}

static void test_extension(void)
{
    CHECK(install_extension_ok(INSTALL_BUILTIN, "C:\\FONTS\\DJVUSANS.TTF"));
    CHECK(install_extension_ok(INSTALL_BUILTIN, "x.otf"));
    CHECK(install_extension_ok(INSTALL_BUILTIN, "X.Ttc"));
    CHECK(!install_extension_ok(INSTALL_BUILTIN, "X.SPD"));
    CHECK(install_extension_ok(INSTALL_DISK, "X.SPD"));
    CHECK(!install_extension_ok(INSTALL_BUILTIN, "A.FNT"));     /* bitmap fonts: not yet */
    CHECK(!install_extension_ok(INSTALL_DISK, "A.FNT"));
    CHECK(!install_extension_ok(INSTALL_BUILTIN, "NOEXT"));
    CHECK(!install_extension_ok(INSTALL_BUILTIN, "C:\\DIR.TTF\\FILE"));
    CHECK(!install_extension_ok(INSTALL_NONE, "A.TTF"));
}

static void test_basename(void)
{
    CHECK_STR(install_basename("C:\\A\\B.TTF"), "B.TTF");
    CHECK_STR(install_basename("B.TTF"), "B.TTF");
    CHECK_STR(install_basename("C:B.TTF"), "B.TTF");
    CHECK_STR(install_basename("C:\\A\\"), "");
}

static int assign(const char *text, char *out)
{
    return install_parse_assign_path(text, strlen(text), out, 64);
}

static void test_assign(void)
{
    char out[64];

    CHECK(assign("PATH=C:\\GEMSYS\r\n01 FONT.FNT\r\n", out));
    CHECK_STR(out, "C:\\GEMSYS");
    CHECK(assign("; fonts\n  path = d:\\fonts\\  \n", out));
    CHECK_STR(out, "d:\\fonts");
    CHECK(assign("01 A.FNT\npath=\\GEMSYS\\", out));
    CHECK_STR(out, "\\GEMSYS");
    CHECK(assign("path=C:\\", out));
    CHECK_STR(out, "C:\\");
    CHECK(assign("path=\\\n", out));            /* the root of the boot drive */
    CHECK_STR(out, "\\");
    CHECK(assign("PATH = /\r\n", out));
    CHECK_STR(out, "/");
    CHECK(!assign("01 A.FNT\n", out));
    CHECK(!assign("pathology=1\n", out));
    CHECK(!assign("path=\n", out));
    CHECK(!assign("", out));
    CHECK(install_parse_assign_path("path=C:\\LONG", 12, out, 5) == 0);     /* does not fit */
    CHECK(install_parse_assign_path("path=C:\\X junk", 10, out, 64) == 1);  /* len bounds the text */
    CHECK_STR(out, "C:\\X");
}

static void test_dest(void)
{
    char dir[64], file[96];

    CHECK_INT(install_dest_dir(INSTALL_BUILTIN, 'c', "D:\\IGNORED", dir, sizeof dir), 0);
    CHECK_STR(dir, "C:\\GEMSYS\\FONTS");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'C', NULL, dir, sizeof dir), 0);
    CHECK_STR(dir, "C:\\GEMSYS");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'C', "d:\\fonts", dir, sizeof dir), 0);
    CHECK_STR(dir, "D:\\FONTS");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'E', "\\FONTS", dir, sizeof dir), 0);
    CHECK_STR(dir, "E:\\FONTS");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'E', "FONTS", dir, sizeof dir), 0);
    CHECK_STR(dir, "E:\\FONTS");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'E', "C:\\", dir, sizeof dir), 0);
    CHECK_STR(dir, "C:\\");
    CHECK_INT(install_dest_dir(INSTALL_DISK, 'e', "\\", dir, sizeof dir), 0);    /* path=\ */
    CHECK_STR(dir, "E:\\");
    CHECK_INT(install_dest_file(dir, "x.ttf", file, sizeof file), 0);
    CHECK_STR(file, "E:\\X.TTF");
    CHECK_INT(install_dest_dir(INSTALL_NONE, 'C', NULL, dir, sizeof dir), -1);
    CHECK_INT(install_dest_dir(INSTALL_BUILTIN, 'C', NULL, dir, 8), -1);

    CHECK_INT(install_dest_file("C:\\GEMSYS\\FONTS", "A:\\dl\\Dejavu.ttf", file, sizeof file), 0);
    CHECK_STR(file, "C:\\GEMSYS\\FONTS\\DEJAVU.TTF");
    CHECK_INT(install_dest_file("C:\\", "x.ttf", file, sizeof file), 0);
    CHECK_STR(file, "C:\\X.TTF");
    CHECK_INT(install_dest_file("C:\\GEMSYS\\FONTS", "A:\\dl\\", file, sizeof file), -1);
    CHECK_INT(install_dest_file("C:\\GEMSYS\\FONTS", "A.TTF", file, 20), -1);
}

static void test_same_and_fit(void)
{
    char s[32];

    CHECK(install_same_path("c:\\gemsys\\fonts\\a.ttf", "C:\\GEMSYS\\FONTS\\A.TTF"));
    CHECK(!install_same_path("C:\\A.TTF", "C:\\B.TTF"));
    CHECK(!install_same_path("C:\\A.TTF", "C:\\A.TTF2"));

    install_fit("C:\\GEMSYS\\FONTS", 30, s);
    CHECK_STR(s, "C:\\GEMSYS\\FONTS");
    install_fit("C:\\GEMSYS\\FONTS", 10, s);
    CHECK_STR(s, "...S\\FONTS");
    CHECK_INT(strlen(s), 10);
    install_fit("C:\\GEMSYS", 3, s);
    CHECK_STR(s, "SYS");
}

void test_install(void)
{
    test_kind();
    test_extension();
    test_basename();
    test_assign();
    test_dest();
    test_same_and_fit();
}
