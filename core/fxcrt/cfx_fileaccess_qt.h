#ifndef CORE_FXCRT_CFX_FILEACCESS_QT_H_
#define CORE_FXCRT_CFX_FILEACCESS_QT_H_

#include "core/fxcrt/fileaccess_iface.h"
#include "core/fxcrt/fx_system.h"

#if _FX_PLATFORM_ != _FX_PLATFORM_QT_
#error "Included on the wrong platform"
#endif

#include <QFile>

class CFX_FileAccess_Qt final : public FileAccessIface {
 public:
  CFX_FileAccess_Qt();
  ~CFX_FileAccess_Qt() override;

  // FileAccessIface:
  bool Open(const ByteStringView& fileName, uint32_t dwMode) override;
  bool Open(const WideStringView& fileName, uint32_t dwMode) override;
  void Close() override;
  FX_FILESIZE GetSize() const override;
  FX_FILESIZE GetPosition() const override;
  FX_FILESIZE SetPosition(FX_FILESIZE pos) override;
  size_t Read(void* pBuffer, size_t szBuffer) override;
  size_t Write(const void* pBuffer, size_t szBuffer) override;
  size_t ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos) override;
  size_t WritePos(const void* pBuffer,
                  size_t szBuffer,
                  FX_FILESIZE pos) override;
  bool Flush() override;
  bool Truncate(FX_FILESIZE szFile) override;

 private:
  QFile m_file;
};

#endif  // CORE_FXCRT_CFX_FILEACCESS_QT_H_
