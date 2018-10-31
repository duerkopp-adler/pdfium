// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "cfx_fileaccess_qt.h"

#include "third_party/base/ptr_util.h"

namespace {

void GetFileMode(quint32 dwModes, QIODevice::OpenMode &nFlags) {
  if (dwModes & FX_FILEMODE_ReadOnly) {
    nFlags |= QIODevice::ReadOnly;
  } else {
    nFlags |= QIODevice::ReadWrite;
    if (dwModes & FX_FILEMODE_Truncate) {
      nFlags |= QIODevice::Truncate;
    }
  }
}

}  // namespace

// static
std::unique_ptr<FileAccessIface> FileAccessIface::Create() {
  return pdfium::MakeUnique<CFX_FileAccess_Qt>();
}

CFX_FileAccess_Qt::CFX_FileAccess_Qt()
{
}

CFX_FileAccess_Qt::~CFX_FileAccess_Qt()
{
    Close();
}

bool CFX_FileAccess_Qt::Open(const ByteStringView& fileName, quint32 dwMode)
{
    QString filename = QString::fromUtf8(fileName.unterminated_c_str(), fileName.GetLength());
    m_file.setFileName(filename);
    QIODevice::OpenMode nFlags;
    GetFileMode(dwMode, nFlags);
    return m_file.open(nFlags);
}

bool CFX_FileAccess_Qt::Open(const WideStringView& fileName, quint32 dwMode)
{
    return Open(FX_UTF8Encode(fileName).AsStringView(), dwMode);
}

void CFX_FileAccess_Qt::Close()
{
    m_file.close();
}

FX_FILESIZE CFX_FileAccess_Qt::GetSize() const
{
    return m_file.size();
}

FX_FILESIZE CFX_FileAccess_Qt::GetPosition() const
{
    return m_file.pos();
}

FX_FILESIZE CFX_FileAccess_Qt::SetPosition(FX_FILESIZE pos)
{
    if (!m_file.seek(pos)) {
        return (FX_FILESIZE) - 1;
    }
    return m_file.pos();
}

size_t CFX_FileAccess_Qt::Read(void* pBuffer, size_t szBuffer)
{
    return m_file.read((char*) pBuffer,szBuffer);
}

size_t CFX_FileAccess_Qt::Write(const void* pBuffer, size_t szBuffer)
{
    return m_file.write((char*) pBuffer, szBuffer);
}

size_t CFX_FileAccess_Qt::ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (pos >= GetSize()) {
        return 0;
    }
    if (SetPosition(pos) == (FX_FILESIZE) - 1) {
        return 0;
    }
    return Read(pBuffer, szBuffer);
}

size_t CFX_FileAccess_Qt::WritePos(const void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (SetPosition(pos) == (FX_FILESIZE) - 1) {
        return 0;
    }
    return Write(pBuffer, szBuffer);
}

bool CFX_FileAccess_Qt::Flush()
{
    return m_file.flush();
}

bool CFX_FileAccess_Qt::Truncate(FX_FILESIZE szFile)
{
    return m_file.resize(szFile);
}

