[Defines]
  PLATFORM_NAME                  = Espresso
  PLATFORM_GUID                  = 57c209ef-5616-4e19-a739-97d9d0b2c2ab
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/Espresso
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = EspressoPkg/EspressoPkg.fdf

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  SerialPortLib|Omap4Pkg/Library/Omap4SerialPortLib/Omap4SerialPortLib.inf
  ArmPlatformLib|EspressoPkg/Library/EspressoBoardLib/EspressoBoardLib.inf
  ArmLib|ArmPkg/Library/ArmLib/ArmThumbLib.inf
  ArmCpuLib|ArmPkg/Drivers/ArmCpuLib/ArmCpuLib.inf
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf

[PcdsFixedAtBuild]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x40000000
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x80000000
  gArmTokenSpaceGuid.PcdFdSize|0x00100000

[Components]
  ArmPkg/Drivers/CpuPei/CpuPei.inf
