// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/agg/fx_agg_driver.h"
#include "core/fxge/cfx_folderfontinfo.h"
#include "public/fpdf_sysfontinfo.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_fontmgr.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_QT_

#define CHARSET_FLAG_ANSI (1 << 0)
#define CHARSET_FLAG_SYMBOL (1 << 1)
#define CHARSET_FLAG_SHIFTJIS (1 << 2)
#define CHARSET_FLAG_BIG5 (1 << 3)
#define CHARSET_FLAG_GB (1 << 4)
#define CHARSET_FLAG_KOREAN (1 << 5)

#include <QStandardPaths>
#include <QDebug>
static const struct {
  const char* m_pName;
  const char* m_pSubstName;
} Base14Substs[] = {
    {"Courier", "Courier New"},
    {"Courier-Bold", "Courier New Bold"},
    {"Courier-BoldOblique", "Courier New Bold Italic"},
    {"Courier-Oblique", "Courier New Italic"},
    {"Helvetica", "Arial"},
    {"Helvetica-Bold", "Arial Bold"},
    {"Helvetica-BoldOblique", "Arial Bold Italic"},
    {"Helvetica-Oblique", "Arial Italic"},
    {"Times-Roman", "Times New Roman"},
    {"Times-Bold", "Times New Roman Bold"},
    {"Times-BoldItalic", "Times New Roman Bold Italic"},
    {"Times-Italic", "Times New Roman Italic"},
};
class CFX_QtFontInfo : public CFX_FolderFontInfo {
 public:
  void* MapFont(int weight,
                bool bItalic,
                int charset,
                int pitch_family,
                const char* family) override;
  bool ParseFontCfg();
  void* FindFont(int weight,
                 bool bItalic,
                 int charset,
                 int pitch_family,
                 const char* family,
                 bool bMatchName);
};
#define Qt_GPNAMESIZE 6
static const struct {
  const char* NameArr[Qt_GPNAMESIZE];
} QtGpFontList[] = {
    {{"TakaoPGothic", "VL PGothic", "IPAPGothic", "VL Gothic", "Kochi Gothic",
      "VL Gothic regular"}},
    {{"TakaoGothic", "VL Gothic", "IPAGothic", "Kochi Gothic", NULL,
      "VL Gothic regular"}},
    {{"TakaoPMincho", "IPAPMincho", "VL Gothic", "Kochi Mincho", NULL,
      "VL Gothic regular"}},
    {{"TakaoMincho", "IPAMincho", "VL Gothic", "Kochi Mincho", NULL,
      "VL Gothic regular"}},
};
static const char* const g_QtGbFontList[] = {
    "AR PL UMing CN Light", "WenQuanYi Micro Hei", "AR PL UKai CN",
};
static const char* const g_QtB5FontList[] = {
    "AR PL UMing TW Light", "WenQuanYi Micro Hei", "AR PL UKai TW",
};
static const char* const g_QtHGFontList[] = {
    "UnDotum",
};
static int32_t GetJapanesePreference(ByteString& face,
                                     int weight,
                                     int picth_family) {
  if (face.Contains("Gothic") ||
      face.Contains("\x83\x53\x83\x56\x83\x62\x83\x4e")) {
    if (face.Contains("PGothic") ||
        face.Contains("\x82\x6f\x83\x53\x83\x56\x83\x62\x83\x4e")) {
      return 0;
    }
    return 1;
  }
  if (face.Contains("Mincho") || face.Contains("\x96\xbe\x92\xa9")) {
    if (face.Contains("PMincho") ||
        face.Contains("\x82\x6f\x96\xbe\x92\xa9")) {
      return 2;
    }
    return 3;
  }
  if (!(picth_family & FXFONT_FF_ROMAN) && weight > 400) {
    return 0;
  }
  return 2;
}
void* CFX_QtFontInfo::MapFont(int weight,
                                 bool bItalic,
                                 int charset,
                                 int pitch_family,
                                 const char* cstr_face) {
  return nullptr;
  ByteString face = cstr_face;
  int iBaseFont;
  for (iBaseFont = 0; iBaseFont < 12; iBaseFont++)
    if (face == ByteStringView(Base14Substs[iBaseFont].m_pName)) {
      face = Base14Substs[iBaseFont].m_pSubstName;
      break;
    }
  if (iBaseFont < 12) {
    return GetFont(face.c_str());
  }
  bool bCJK = true;
  switch (charset) {
    case FXFONT_SHIFTJIS_CHARSET: {
      int32_t index = GetJapanesePreference(face, weight, pitch_family);
      if (index < 0) {
        break;
      }
      for (int32_t i = 0; i < Qt_GPNAMESIZE; i++) {
        auto it = m_FontList.find(QtGpFontList[index].NameArr[i]);
        if (it != m_FontList.end()) {
          return it->second.get();
        }
      }
    } break;
    case FXFONT_GB2312_CHARSET: {
      for (uint32_t i = 0; i < FX_ArraySize(g_QtGbFontList); ++i) {
        auto it = m_FontList.find(g_QtGbFontList[i]);
        if (it != m_FontList.end()) {
          return it->second.get();
        }
      }
    } break;
    case FXFONT_CHINESEBIG5_CHARSET: {
      for (uint32_t i = 0; i < FX_ArraySize(g_QtB5FontList); ++i) {
        auto it = m_FontList.find(g_QtB5FontList[i]);
        if (it != m_FontList.end()) {
          return it->second.get();
        }
      }
    } break;
    case FXFONT_HANGEUL_CHARSET: {
      for (uint32_t i = 0; i < FX_ArraySize(g_QtHGFontList); ++i) {
        auto it = m_FontList.find(g_QtHGFontList[i]);
        if (it != m_FontList.end()) {
          return it->second.get();
        }
      }
    } break;
    default:
      bCJK = false;
      break;
  }
  if (charset == FXFONT_ANSI_CHARSET && (pitch_family & FXFONT_FF_FIXEDPITCH)) {
    return GetFont("Courier New");
  }
  return FindFont(weight, bItalic, charset, pitch_family, cstr_face, !bCJK);
}
static quint32 _QtGetCharset(int charset) {
  switch (charset) {
    case FXFONT_SHIFTJIS_CHARSET:
      return CHARSET_FLAG_SHIFTJIS;
    case FXFONT_GB2312_CHARSET:
      return CHARSET_FLAG_GB;
    case FXFONT_CHINESEBIG5_CHARSET:
      return CHARSET_FLAG_BIG5;
    case FXFONT_HANGEUL_CHARSET:
      return CHARSET_FLAG_KOREAN;
    case FXFONT_SYMBOL_CHARSET:
      return CHARSET_FLAG_SYMBOL;
    case FXFONT_ANSI_CHARSET:
      return CHARSET_FLAG_ANSI;
    default:
      break;
  }
  return 0;
}
static int32_t _QtGetSimilarValue(int weight,
                                     bool bItalic,
                                     int pitch_family,
                                     quint32 style) {
  int32_t iSimilarValue = 0;
  if ((style & FXFONT_BOLD) == (weight > 400)) {
    iSimilarValue += 16;
  }
  if ((style & FXFONT_ITALIC) == bItalic) {
    iSimilarValue += 16;
  }
  if ((style & FXFONT_SERIF) == (pitch_family & FXFONT_FF_ROMAN)) {
    iSimilarValue += 16;
  }
  if ((style & FXFONT_SCRIPT) == (pitch_family & FXFONT_FF_SCRIPT)) {
    iSimilarValue += 8;
  }
  if ((style & FXFONT_FIXED_PITCH) == (pitch_family & FXFONT_FF_FIXEDPITCH)) {
    iSimilarValue += 8;
  }
  return iSimilarValue;
}
void* CFX_QtFontInfo::FindFont(int weight,
                                  bool bItalic,
                                  int charset,
                                  int pitch_family,
                                  const char* family,
                                  bool bMatchName) {
  return nullptr;
  FontFaceInfo* pFind = NULL;
  quint32 charset_flag = _QtGetCharset(charset);
  int32_t iBestSimilar = 0;
  for (const auto& it : m_FontList) {
    const ByteString& bsName = it.first;
    FontFaceInfo* pFont = it.second.get();
    if (!(pFont->m_Charsets & charset_flag) &&
        charset != FXFONT_DEFAULT_CHARSET) {
      continue;
    }
    int32_t iSimilarValue = 0;
    bool found = bsName.Contains(family);
    if (bMatchName && !found) {
      continue;
    }
    if (!bMatchName && found) {
      iSimilarValue += 64;
    }
    iSimilarValue =
        _QtGetSimilarValue(weight, bItalic, pitch_family, pFont->m_Styles);
    if (iSimilarValue > iBestSimilar) {
      iBestSimilar = iSimilarValue;
      pFind = pFont;
    }
  }
  return pFind;
}
std::unique_ptr<SystemFontInfoIface> SystemFontInfoIface::CreateDefault(const char** pUserPaths)
{
    Q_UNUSED(pUserPaths);
    CFX_QtFontInfo* pInfo = new CFX_QtFontInfo;
    if (!pInfo->ParseFontCfg()) {
        foreach(QString path, QStandardPaths::standardLocations(QStandardPaths::FontsLocation)){
            pInfo->AddPath(path.toLocal8Bit().constData());
        }
    }
    return std::unique_ptr<SystemFontInfoIface>(pInfo);
}
bool CFX_QtFontInfo::ParseFontCfg() {
    return false;
}
void CFX_GEModule::InitPlatform() {
  m_pFontMgr->SetSystemFontInfo(
      SystemFontInfoIface::CreateDefault(m_pUserFontPaths));
}
void CFX_GEModule::DestroyPlatform() {}
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_Qt_
