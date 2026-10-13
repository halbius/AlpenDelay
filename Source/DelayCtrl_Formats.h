#pragma once
#ifndef DelayCtrl_FORMATS_H
#define DelayCtrl_FORMATS_H

// ============================================================================
// DelayCtrl_Formats.h
//
// Einzige Quelle der Wahrheit für alle unterstützten Stem-Formate:
//   - Plugin-ID pro Format
//   - Kanalanzahl
//   - Kanalrolle (L, C, R, Lss, Ltf, ...) pro Kanalposition
//
// Parameter-IDs der Kanal-Delays hängen an der ROLLE ("DlyL", "DlyLss", ...),
// nicht an der Kanalnummer. Dadurch können Page-Tables und Anzeigen die
// echten Kanalnamen tragen.
//
// Fehlt ein Format-Bezeichner in deinem SDK, einfach die betreffende Zeile in
// kTable löschen (und ggf. die ID in der Page-Table-Generierung).
// ============================================================================

#include "AAX.h"
#include <cstdint>

constexpr int kDelayCtrl_MaxChannels = 16;   // 9.1.6 und HOA 3rd Order

enum EDelayCtrl_Role
{
    eRole_L, eRole_C, eRole_R, eRole_Lc, eRole_Rc, eRole_Lw, eRole_Rw,
    eRole_Ls, eRole_Rs, eRole_Lss, eRole_Rss, eRole_Lsr, eRole_Rsr,
    eRole_S, eRole_LFE,
    eRole_Lts, eRole_Rts, eRole_Ltf, eRole_Rtf, eRole_Ltm, eRole_Rtm,
    eRole_Ltr, eRole_Rtr,
    // Ambisonics (ACN-Reihenfolge)
    eRole_W, eRole_Y, eRole_Z, eRole_X,
    eRole_A4, eRole_A5, eRole_A6, eRole_A7, eRole_A8,
    eRole_A9, eRole_A10, eRole_A11, eRole_A12, eRole_A13, eRole_A14, eRole_A15,
    eRole_Count
};

inline const char* DelayCtrl_RoleLabel(int role)
{
    static const char* const s[eRole_Count] = {
        "L", "C", "R", "Lc", "Rc", "Lw", "Rw",
        "Ls", "Rs", "Lss", "Rss", "Lsr", "Rsr",
        "S", "LFE",
        "Lts", "Rts", "Ltf", "Rtf", "Ltm", "Rtm",
        "Ltr", "Rtr",
        "W", "Y", "Z", "X",
        "A4", "A5", "A6", "A7", "A8",
        "A9", "A10", "A11", "A12", "A13", "A14", "A15" };
    return (role >= 0 && role < eRole_Count) ? s[role] : "?";
}

inline const char* DelayCtrl_RoleParamID(int role)
{
    static const char* const s[eRole_Count] = {
        "DlyL", "DlyC", "DlyR", "DlyLc", "DlyRc", "DlyLw", "DlyRw",
        "DlyLs", "DlyRs", "DlyLss", "DlyRss", "DlyLsr", "DlyRsr",
        "DlyS", "DlyLFE",
        "DlyLts", "DlyRts", "DlyLtf", "DlyRtf", "DlyLtm", "DlyRtm",
        "DlyLtr", "DlyRtr",
        "DlyW", "DlyY", "DlyZ", "DlyX",
        "DlyA4", "DlyA5", "DlyA6", "DlyA7", "DlyA8",
        "DlyA9", "DlyA10", "DlyA11", "DlyA12", "DlyA13", "DlyA14", "DlyA15" };
    return (role >= 0 && role < eRole_Count) ? s[role] : "DlyL";
}

struct SDelayCtrl_Format
{
    AAX_EStemFormat stem;
    AAX_CTypeID     plugInID;
    const char*     name;
    int             numChannels;
    int             roles[kDelayCtrl_MaxChannels];  // eRole_* pro Kanalposition
};

// Wiederkehrende Bett-Anordnungen (Pro-Tools-Kanalreihenfolge)
#define DC_BED_5   eRole_L, eRole_C, eRole_R, eRole_Ls, eRole_Rs
#define DC_BED_7   eRole_L, eRole_C, eRole_R, eRole_Lss, eRole_Rss, eRole_Lsr, eRole_Rsr
#define DC_BED_9   eRole_L, eRole_C, eRole_R, eRole_Lw, eRole_Rw, eRole_Lss, eRole_Rss, eRole_Lsr, eRole_Rsr
#define DC_TOP_2   eRole_Lts, eRole_Rts
#define DC_TOP_4   eRole_Ltf, eRole_Rtf, eRole_Ltr, eRole_Rtr
#define DC_TOP_6   eRole_Ltf, eRole_Rtf, eRole_Ltm, eRole_Rtm, eRole_Ltr, eRole_Rtr

inline const SDelayCtrl_Format* DelayCtrl_Formats(int* oCount)
{
    static const SDelayCtrl_Format kTable[] = {
        // ---- Klassisch ------------------------------------------------------
        { AAX_eStemFormat_Mono,   'DC01', "Mono",   1,  { } },
        { AAX_eStemFormat_Stereo, 'DC02', "Stereo", 2,  { eRole_L, eRole_R } },
        { AAX_eStemFormat_LCR,    'DC03', "LCR",    3,  { eRole_L, eRole_C, eRole_R } },
        { AAX_eStemFormat_LCRS,   'DC04', "LCRS",   4,  { eRole_L, eRole_C, eRole_R, eRole_S } },
        { AAX_eStemFormat_Quad,   'DC05', "Quad",   4,  { eRole_L, eRole_R, eRole_Ls, eRole_Rs } },
        { AAX_eStemFormat_5_0,    'DC06', "5.0",    5,  { DC_BED_5 } },
        { AAX_eStemFormat_5_1,    'DC07', "5.1",    6,  { DC_BED_5, eRole_LFE } },
        // ---- 7.x (SDDS- und DTS-Kanalreihenfolge) -----------------------------
        { AAX_eStemFormat_7_0_SDDS, 'DC08', "7.0 SDDS", 7,
          { eRole_L, eRole_Lc, eRole_C, eRole_Rc, eRole_R, eRole_Ls, eRole_Rs } },
        { AAX_eStemFormat_7_1_SDDS, 'DC09', "7.1 SDDS", 8,
          { eRole_L, eRole_Lc, eRole_C, eRole_Rc, eRole_R, eRole_Ls, eRole_Rs, eRole_LFE } },
        { AAX_eStemFormat_7_0_DTS,  'DC10', "7.0 DTS",  7,  { DC_BED_7 } },
        { AAX_eStemFormat_7_1_DTS,  'DC11', "7.1 DTS",  8,  { DC_BED_7, eRole_LFE } },
        // ---- Immersive 5.x.y --------------------------------------------------
        { AAX_eStemFormat_5_0_2, 'DC12', "5.0.2", 7,  { DC_BED_5, DC_TOP_2 } },
        { AAX_eStemFormat_5_1_2, 'DC13', "5.1.2", 8,  { DC_BED_5, eRole_LFE, DC_TOP_2 } },
        { AAX_eStemFormat_5_0_4, 'DC14', "5.0.4", 9,  { DC_BED_5, DC_TOP_4 } },
        { AAX_eStemFormat_5_1_4, 'DC15', "5.1.4", 10, { DC_BED_5, eRole_LFE, DC_TOP_4 } },
        // ---- Immersive 7.x.y --------------------------------------------------
        { AAX_eStemFormat_7_0_2, 'DC16', "7.0.2", 9,  { DC_BED_7, DC_TOP_2 } },
        { AAX_eStemFormat_7_1_2, 'DC17', "7.1.2", 10, { DC_BED_7, eRole_LFE, DC_TOP_2 } },
        { AAX_eStemFormat_7_0_4, 'DC18', "7.0.4", 11, { DC_BED_7, DC_TOP_4 } },
        { AAX_eStemFormat_7_1_4, 'DC19', "7.1.4", 12, { DC_BED_7, eRole_LFE, DC_TOP_4 } },
        { AAX_eStemFormat_7_0_6, 'DC20', "7.0.6", 13, { DC_BED_7, DC_TOP_6 } },
        { AAX_eStemFormat_7_1_6, 'DC21', "7.1.6", 14, { DC_BED_7, eRole_LFE, DC_TOP_6 } },
        // ---- Immersive 9.x.y --------------------------------------------------
        { AAX_eStemFormat_9_0_4, 'DC22', "9.0.4", 13, { DC_BED_9, DC_TOP_4 } },
        { AAX_eStemFormat_9_1_4, 'DC23', "9.1.4", 14, { DC_BED_9, eRole_LFE, DC_TOP_4 } },
        { AAX_eStemFormat_9_0_6, 'DC24', "9.0.6", 15, { DC_BED_9, DC_TOP_6 } },
        { AAX_eStemFormat_9_1_6, 'DC25', "9.1.6", 16, { DC_BED_9, eRole_LFE, DC_TOP_6 } },
        // ---- Higher Order Ambisonics (ACN) ------------------------------------
        { AAX_eStemFormat_Ambi_1_ACN, 'DC26', "HOA 1st", 4,
          { eRole_W, eRole_Y, eRole_Z, eRole_X } },
        { AAX_eStemFormat_Ambi_2_ACN, 'DC27', "HOA 2nd", 9,
          { eRole_W, eRole_Y, eRole_Z, eRole_X, eRole_A4, eRole_A5, eRole_A6, eRole_A7, eRole_A8 } },
        { AAX_eStemFormat_Ambi_3_ACN, 'DC28', "HOA 3rd", 16,
          { eRole_W, eRole_Y, eRole_Z, eRole_X, eRole_A4, eRole_A5, eRole_A6, eRole_A7, eRole_A8,
            eRole_A9, eRole_A10, eRole_A11, eRole_A12, eRole_A13, eRole_A14, eRole_A15 } },
    };
    if (oCount) *oCount = static_cast<int>(sizeof(kTable) / sizeof(kTable[0]));
    return kTable;
}

#undef DC_BED_5
#undef DC_BED_7
#undef DC_BED_9
#undef DC_TOP_2
#undef DC_TOP_4
#undef DC_TOP_6

inline const SDelayCtrl_Format* DelayCtrl_FindFormat(AAX_EStemFormat fmt)
{
    int n = 0;
    const SDelayCtrl_Format* t = DelayCtrl_Formats(&n);
    for (int i = 0; i < n; ++i)
        if (t[i].stem == fmt) return &t[i];
    return nullptr;
}

#endif // DelayCtrl_FORMATS_H
