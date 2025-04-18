#ifndef CONFIG_H
#define CONFIG_H
#define MOD Mod4Mask
const char* menu[]    = {"dmenu_run",      0};
const char* term[]    = {"st",             0};
const char* stup[]   = {"stup",            0};
const char* brup[]   = {"brup",          0};
const char* acmeup[]   = {"acmeup",          0};
const char* zoom[]   = {"boomer",          0};
const char* bookmark[]   = {"bookmark",          0};
const char* ploomb[]   = {"ploomb",            0};
const char* scr[]   = {"scr",          0};
static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD, XK_b,      run, {.com = bookmark}},
    {MOD, XK_d,      run, {.com = menu}},
    {MOD, XK_f,      run, {.com = ploomb}},
    {MOD, XK_z,      run, {.com = zoom}},
    {MOD, XK_Return, run, {.com = term}},
    {0,   XK_Print,  run, {.com = scr}},
    {0,   XF86XK_AudioLowerVolume,  run, {.com = brup}},
    {0,   XF86XK_AudioRaiseVolume,  run, {.com = stup}},
    {0,   XF86XK_AudioMute,         run, {.com = acmeup}},
};
#endif

