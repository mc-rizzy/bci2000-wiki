/**
 * @mainpage 欢迎！
 *
 * 请点击 \ref NeusenW4SDK.h "NeusenW4SDK.h" 访问W4 SDK API文档。
 * <br><br>
 * @author wangyang <wangyang@neuracle.cn>
 * @date 2025/01/16
 */

#ifndef __NEURACLE_W4_SDK_H__
#define __NEURACLE_W4_SDK_H__

#ifdef _MSC_VER
#ifdef NEUSENW4SDKDLL_EXPORTS
#define NEURACLE_W4SDK_LIBRARY_EXPORT __declspec(dllexport)
#else
#define NEURACLE_W4SDK_LIBRARY_EXPORT __declspec(dllimport)
#endif
#else
#define NEURACLE_W4SDK_LIBRARY_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 结果码枚举类型
 *
 * 这个枚举定义了几种执行结果的错误码。
 */
enum NeuracleV4ResultCode
{
    NeuracleV4Result_Succeed                             = 0,           /**< 成功。 */
    NeuracleV4Result_Information_SyncSucceeded           = 0x01000001,  /**< 同步成功。 */
    NeuracleV4Result_Information_TriggerArrived          = 0x01000002,  /**< 发生事件。 */
    NeuracleV4Result_Information_ReconnectSucceeded      = 0x01000003,  /**< 重连设备成功。 */
    NeuracleV4Result_Information_ValueNoChanging         = 0x01000004,  /**< 值没有变化。 */

    NeuracleV4Result_Warning_NeedMoreSpace               = 0x04000001,  /**< 警告，需要更多空间。 */
    NeuracleV4Result_Warning_NoEnoughData                = 0x04000002,  /**< 警告，没有足够多的数据。 */
    NeuracleV4Result_Warning_DongleLost                  = 0x04000003,  /**< 警告，没有授权狗。 */
    NeuracleV4Result_Warning_DeviceDisconnected          = 0x04000004,  /**< 警告，设备断线。 */

    NeuracleV4Result_Error_InvalidHandle                 = 0x08000001,  /**< 错误，无效句柄。 */
    NeuracleV4Result_Error_InvalidParameter              = 0x08000002,  /**< 错误，无效参数。 */
    NeuracleV4Result_Error_DoggleVerifyFailed            = 0x08000003,  /**< 错误，授权狗校验失败。 */
    NeuracleV4Result_Error_StartAmplifyFailed            = 0x08000004,  /**< 错误，启动采集失败。 */
    NeuracleV4Result_Error_StartTriggerFailed            = 0x08000005,  /**< 错误，启动同步盒失败。 */
    NeuracleV4Result_Error_MemoryAllocFailed             = 0x08000006,  /**< 错误，申请内存失败。 */
    NeuracleV4Result_Error_InstanceExisted               = 0x08000007,  /**< 错误，实例已经存在。 */
    NeuracleV4Result_Error_InvalidDeviceController       = 0x08000008,  /**< 错误，无效的设备控制器。 */
    NeuracleV4Result_Error_InvalidSyncboxType            = 0x08000009,  /**< 错误，无效的同步盒类型。 */
    NeuracleV4Result_Error_InvalidEEGHostType            = 0x0800000A,  /**< 错误，无效的脑电主机类型。 */
    NeuracleV4Result_Error_MoreThanOneHost               = 0x0800000B,  /**< 错误，超过1个主机。 */
    NeuracleV4Result_Error_MoreThanOneModule             = 0x0800000C,  /**< 错误，超过1个探头。 */
    NeuracleV4Result_Error_InvalidEEGModuleType          = 0x0800000D,  /**< 错误，无效的脑电主机探头类型。 */
    NeuracleV4Result_Error_Environment                   = 0x0800000E,  /**< 错误，环境错误。 */

    NeuracleV4Result_System_Error_NoEnoughMemory         = 0x0C000001,  /**< 系统错误，内存不足。 */
};

/**
 * @brief 传感器工作模式类型
 *
 * 这个枚举定义了若干传感器的工作模式。
 */
enum WorkModeV4
{
    WorkModeV4_Unused = 0,                  /**< 未使用，这是一个无效值。 */
    WorkModeV4_Wave = 4,                    /**< 自检方波。 */
    WorkModeV4_Short = 5,                   /**< 短路噪声。 */
    WorkModeV4_Impedance_Offline = 12,      /**< 离线阻抗。 */
    WorkModeV4_Signal = 14,                 /**< 信号。 */
    WorkModeV4_Impedance_Reference = 15,    /**< 阻抗参考。 */
};

/**
 * @brief 传感器类型
 *
 * 这个枚举定义了传感器类型。
 */
enum SensorTypeV4
{
    SensorTypeV4_Host = (1 << 0),           /**< 暂时没有用。 */

    SensorTypeV4_EEG = (1 << 1),            /**< 高速探头的EEG Sensor。 */
    SensorTypeV4_ECG = (1 << 2),            /**< 低速探头的ECG Sensor。 */
    SensorTypeV4_GSR = (1 << 3),            /**< 低速探头的GSR Sensor。 */
    SensorTypeV4_SPO2 = (1 << 4),           /**< 低速探头的SPO2 Sensor。 */
    SensorTypeV4_MEMsAll = (1 << 5),        /**< 低速探头的9轴 Sensor。 */

    SensorTypeV4_Event = (1 << 6),          /**< 暂时没有用。 */

    SensorTypeV4_EMG = (1 << 7),            /**< 无线肌电的EMG Sensor。 */
    SensorTypeV4_CMEEG = (1 << 8),          /**< 干电极CM sensor。 */
    SensorTypeV4_SKT = (1 << 9),            /**< 睡眠探头的皮温 Sensor。 */
    SensorTypeV4_RESP = (1 << 10),          /**< 睡眠探头的呼吸 Sensor。 */

    SensorTypeV4_EyeTracker = (1 << 31),    /**< 眼动仪Sensor。 */

    SensorTypeV4_UNKNOWN_TYPE = 0x0         /**< 无效类型    。 */
};

/**
 * @brief 数据种类的类型
 *
 * 这个枚举定义了数据种类的类型。
 */
enum ModuleTypeV4
{
    ModuleTypeV4_Device = 0x01,             /**< 主机。 */
    ModuleTypeV4_EEG = 0x02,                /**< 脑电。 */
    ModuleTypeV4_SpO2 = 0x05,               /**< 血氧。 */

    ModuleTypeV4_MEMsAll = 0x10,            /**< 九轴。 */
    ModuleTypeV4_ECG = 0x1B,                /**< 心电。 */
    ModuleTypeV4_GSR = 0x1C,                /**< 皮电。 */
    ModuleTypeV4_EMG = 0x1D,                /**< 无线肌电设备，仅EMG通道时，上报的Alert包头中的ModuleType(Format)用此值；以及阻抗模式下也是该值。 */
    ModuleTypeV4_EEG_CM = 0x1E,             /**< 干电极脑电。 */
    ModuleTypeV4_EMG_MEMSALL = 0x1F,        /**< EMG和九轴都用时，上报的Alert包头中的ModuleType(Format)用此值。 */

    ModuleTypeV4_SKT = 0x25,                /**< 皮温。 */
    ModuleTypeV4_RESP = 0x26,               /**< 呼吸。 */

    ModuleTypeV4_EyeTracker = 0x60,         /**< 眼球跟踪。 */

    ModuleTypeV4_UNKNOWN_TYPE = 0xFF        /**< 无效类型。 */
};

/**
 * @brief 传感器的设备类型
 *
 * 这个枚举定义了传感器的设备类型。
 */
struct SensorDeviceType
{
    const char* sensorType;                 /**< 类型字符串。 */
    unsigned int* supportedSampleRates;     /**< 支持的采样率数组。 */
    int supportedSampleRatesCount;          /**< 支持的采样率数组长度。 */
    unsigned int DefaultSampleRate;         /**< 默认采样率。 */
    bool canMontage;                        /**< 能否Montage。 */
    bool canImpedance;                      /**< 能否阻抗。 */
    bool enableGNDAndREFImpedance;          /**< 阻抗时开启接地和参考。 */
    unsigned char byteCount;                /**< 字节长度。 */
    enum WorkModeV4* supportedWorkModes;    /**< 支持的工作模式类型数组。 */
    int supportedWorkModesCount;            /**< 支持的工作模式类型数组长度。 */
    enum WorkModeV4 defaultWorkMode;        /**< 默认工作模式类型。 */

    int supportedChannelTypesCount;         /**< 支持的通道类型数量。 */
    int supportedCollectorTypesCount;       /**< 支持的采集器类型数量。 */
    float physicalMaximum;                  /**< 信号的物理最大值。 */
    float physicalMinimum;                  /**< 信号的物理最小值。 */
    int digitalMinimum;                     /**< 信号的数字最小值。 */
    int digitalMaximum;                     /**< 信号的数字最大值。 */
    int channelCount;                       /**< 通道数量。 */
    int gain;                               /**< 增益。 */
    const char* units;                      /**< 单位字符串。 */
    enum ModuleTypeV4 moduleTypeForProtocol;     /**< 协议中的数据种类的类型。 */
    enum SensorTypeV4 sensorTypeForProtocol;     /**< 协议中的传感器种类的类型。 */
};

/**
 * @brief 传感器信息
 *
 * 这个枚举定义了传感器信息。
 */
struct SensorInfoV4
{
    unsigned char DeviceType[13] = { 0 };   /**< 设备类型字符串数组，包括'\0'结尾，固定长度12。 */
    unsigned char  CapType[17] = { 0 };     /**< 脑电帽的类型，包括'\0'结尾，固定长度16。 */
    unsigned int CapSerialNumber;           /**< 脑电帽的序列号。 */
    char  CapManufactureDate[13] = { 0 };   /**< 脑电帽的生成日期，前8位有效 YYYYMMDD，后4位为补位。 */
    char CapSettingName[17] = { 0 };        /**< 脑电帽的配置名称，包括'\0'结尾，固定长度16。 */
};

/**
 * @brief 探头信息
 *
 * 这个枚举定义了探头信息。
 */
struct ModuleInfoV4
{
    unsigned char HardwareVersion;          /**< 硬件版本。 */
    unsigned char FirmwareVersion;          /**< 固件版本。 */
    unsigned char BatteryLevel;             /**< 电量。 */
    unsigned int SerialNumber;              /**< 序列号。 */
    unsigned short CpuFlag;                 /**< CPU标志。 */
    unsigned int BoardFlag;                 /**< 主板标志*/
    unsigned char DeviceType[13] = { 0 };   /**< 设备类型字符串数组，包括'\0'结尾，固定长度12。 */
    unsigned char SensorCount;              /**< 传感器信息数组的长度。 */
    struct SensorInfoV4* SensorInfos;       /**< 传感器信息数组。 */
};

/**
 * @brief 主机信息
 *
 * 这个枚举定义了主机信息。
 */
struct HostInfoV4
{
    unsigned char MajorAcpVersion;          /**< 主ACP版本。 */
    unsigned char MinorAcpVersion;          /**< 次ACP版本。 */
    unsigned short TcpListeningPort;        /**< 下位机（或模拟器）的TCP侦听端口，他们在通过UDP广播应答的时候，会将其TCP Server监听端口号上报给上位机。 */

    unsigned char HardwareVersion;          /**< 硬件版本。 */
    unsigned char FirmwareVersion;          /**< 固件版本。 */
    unsigned char BatteryLevel;             /**< 电量。 */
    unsigned int SerialNumber;              /**< 序列号。 */
    unsigned short CpuFlag;                 /**< CPU标志。 */
    unsigned int BoardFlag;                 /**< 主板标志*/
    unsigned char DeviceType[13] = { 0 };   /**< 设备类型字符串数组，包括'\0'结尾，固定长度12。 */

    char IPAddress[16] = { 0 };             /**< IPV4地址，包括'\0'结尾，固定长度15。 */

    unsigned char ModuleCount;              /**< 探头信息数组的长度。 */
    ModuleInfoV4* ModuleInfos;              /**< 探头信息数组。 */
};

/**
 * @struct DataBlock
 * @brief 采样数据块结构体
 */
typedef struct DataBlockV4 {
    float* Datas;                       /**< 采样数组，通道数ChannelCount简记为X，数据点数DataCountPerChannel简记为Y，数组长度是X * Y。
                                            每个通道的Y个数据点连续分布，然后是下一个通道。内存连续布局如下图从左到右、从上到下。
    * <table>
    * <tr>
    * <th>通道</th>
    * <th>数据点1</th>
    * <th>数据点2</th>
    * <th>数据点Y</th>
    * </tr>
    * <tr>
    * <td>通道1</td>
    * <td>数据</td>
    * <td>数据</td>
    * <td>数据</td>
    * </tr>
    * <tr>
    * <td>通道2</td>
    * <td>数据</td>
    * <td>数据</td>
    * <td>数据</td>
    * </tr>
    * <tr>
    * <td>通道X</td>
    * <td>数据</td>
    * <td>数据</td>
    * <td>数据</td>
    * </tr>
    * </table>
    */

    int* Markers;                       /**< 事件数组，没有事件时是空指针。有事件时非空，数组长度是DataCountPerChannel，需要注意的是仅仅当值是非0时才是事件。。 */
    unsigned int Timestamp;             /**< 时间戳。。 */
    int ChannelCount;                   /**< 通道总数。。 */
    int DataCountPerChannel;            /**< 每个通道的数据点数，也说明了数据的时间范围。。 */
    unsigned int DeviceSerialNumber;    /**< 设备序列号。。 */
    unsigned int BoardId;               /**< 主板序号。。 */
    unsigned short CpuID;               /**< CPU序号。。 */
    bool IsSynced;                      /**< 是否同步。当你使用了同步盒并且同步盒和主机信号同步后，这里将会是true，否则是false。。 */
    ModuleTypeV4 SensorType;            /**< 数据种类的类型。。 */
} DataBlockV4;

/**
 * @struct NeuracleV4DeviceStatus
 * @brief 设备状态结构体
 */
typedef struct NeuracleV4DeviceStatus {
    int BatteryLevel;                   /**< 0~100，表示剩余电量百分比，如果设备没有电池这里永远是100。 */
    bool SyncStatus;                    /**< 是否和同步盒同步上 */
    bool DeviceConnectStatus;           /**< 设备是否链接上 */
} NeuracleV4DeviceStatus;

/**
 * @struct NeuracleV4Controller
 * @brief 设备控制器结构体
 */
typedef struct NeuracleV4Controller NeuracleV4Controller;

/**
 * @struct NeuracleV4DiscoveryController
 * @brief 设备发现结构体
 *
 * 设备发现。
 */
typedef struct NeuracleV4DiscoveryController NeuracleV4DiscoveryController;

/**
 * @brief 读取Sdk版本号。
 *
 * @return 版本号的字符指针
 *
 * @retval NULL                                           失败。
 * @retval 非NULL                                         版本号的字符串地址，该指针不用释放。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT const char* NeuracleSdkV4GetVersion(void);

/**
 * @brief 获取加密狗功能是否启用。
 *
 * @return 是否启用的结果
 *
 * @retval 1                                              启用。
 * @retval 0                                              未启用。
 * @retval 其他                                           非法。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT int NeuracleSdkV4IsEnabledDog(void);

/**
 * @brief 读取错误码的字符串。
 *
 * @param[in]  code                                       错误码。
 *
 * @return 错误码的字符串。
 *
 * @retval 非NULL                                         该指针不用释放。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT const char* NeuracleV4GetResultCodeString(NeuracleV4ResultCode code);

/**
 * @brief 创建设备发现控制器的实例。
 * @note  该API和\ref NeuracleV4CreateDeviceDiscoveryWithPort "NeuracleV4CreateDeviceDiscoveryWithPort"类似，内部默认向UDP的1020端口广播设备发现包，同时侦听本机UDP的1020端口、接收响应包。
 *
 * @param[out]  ppController                              这是一个二级指针，成功时*ppController返回实例的地址。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                       成功。
 * @retval NeuracleV4Result_System_Error_NoEnoughMemory   内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT NeuracleV4ResultCode NeuracleV4CreateDeviceDiscovery(NeuracleV4DiscoveryController** ppController);

/**
 * @brief 创建设备发现控制器的实例。
 * @note  连接模拟器时，broadcastPort应该传入1021。
 *
 * @param[out]  ppController                              这是一个二级指针，成功时*ppController返回实例的地址。
 * @param[in]   broadcastPort                             向局域网内的该UDP的端口号，广播设备发现请求包。
 * @param[in]   listenPort                                侦听本机该UDP的端口号，接收响应包。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                       成功。
 * @retval NeuracleV4Result_System_Error_NoEnoughMemory   内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT NeuracleV4ResultCode NeuracleV4CreateDeviceDiscoveryWithPort(NeuracleV4DiscoveryController** ppController, int broadcastPort, int listenPort);

/**
 * @brief 销毁设备发现控制器的实例。
 *
 * @param[in]  controller                                 设备发现控制器的实例。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT void NeuracleV4DeleteDeviceDiscovery(NeuracleV4DiscoveryController *controller);

/**
 * @brief 启动设备发现。
 *
 * @param[in]  controller                                 设备发现控制器的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                       成功。
 * @retval NeuracleV4Result_Error_InvalidHandle           无效句柄，controller非法。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT NeuracleV4ResultCode NeuracleV4StartDeviceDiscovery(NeuracleV4DiscoveryController* controller);

/**
 * @brief 停止设备发现。
 *
 * @param[in]  controller                                 设备发现控制器的实例。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT void NeuracleV4StopDeviceDiscovery(NeuracleV4DiscoveryController* controller);

/**
 * @brief 读取广播发现到的设备数量。
 * @note  应在成功调用\ref NeuracleV4StartDeviceDiscovery "NeuracleV4StartDeviceDiscovery"之后调用本函数，多次调用间可睡眠若干毫秒、从而释放CPU。
 *
 * @param[in]  controller                                 设备发现控制器的实例。
 * @param[out] deviceCount                                成功时*deviceCount返回设备的数量。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                       成功。
 * @retval NeuracleV4Result_Error_InvalidHandle           无效句柄，controller非法。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT NeuracleV4ResultCode NeuracleV4GetFoundedDevicesCount(NeuracleV4DiscoveryController* controller, int* deviceCount);

/**
 * @brief 按照索引来读取设备。
 * @note  \ref NeuracleV4GetFoundedDevicesCount "NeuracleV4GetFoundedDevicesCount"读取设备数量，假设是5个，然后在一个循环中依次调用本API传入索引0~4，即可获取对应的设备信息。
 * @note  需要注意的是，本函数将会给调用者传入的参数device的某些字段，分配内存空间，故当使用完之后要调用\ref NeuracleV4FreeDevice "NeuracleV4FreeDevice"来释放。
 *
 * @param[in]  controller                               NeuracleDeviceDiscovery()函数返回的实例地址。
 * @param[out] device                                   设备指针，由调用者分配空间，设备长度是1。请注意不要修改结构体的任意字段，避免上述释放时出现异常。
 * @param[in]  index                                    设备索引。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidHandle         无效句柄，controller非法。
 * @retval NeuracleV4Result_Warning_NoEnoughData        没有对应索引的设备。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT NeuracleV4ResultCode NeuracleV4GetFoundedDevicesByIndex(NeuracleV4DiscoveryController* controller, struct HostInfoV4 *device, int index);

/**
 * @brief 释放设备内部的内容空间。
 * @note  参数device必须来自通过\ref NeuracleV4GetFoundedDevicesByIndex "NeuracleV4GetFoundedDevicesByIndex"读取到的设备结构体。
 *
 * @param[in]  device                                   设备。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT void NeuracleV4FreeDevice(struct HostInfoV4 *device);

/**
 * @brief 打印设备信息到控制台。
 * @note  参数device必须来自通过\ref NeuracleV4GetFoundedDevicesByIndex "NeuracleV4GetFoundedDevicesByIndex"读取到的设备结构体。
 *
 * @param[in]  device                                   设备。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT void NeuracleV4PrintDevice(struct HostInfoV4 *device);

/**
 * @brief 打印设备信息到日志文件。
 * @note  参数device必须来自通过\ref NeuracleV4GetFoundedDevicesByIndex "NeuracleV4GetFoundedDevicesByIndex"读取到的设备结构体。
 *
 * @param[in]  device                                   设备。
 */
void NeuracleV4LogDevice(struct HostInfoV4 *device);

/**
 * @brief 创建设备控制器结构体的实例。
 *
 * @param[out]  controller                              这是一个二级指针，成功时*ppController返回实例的地址。
 * @param[in]   autoReconnect                           当设备掉线时，是否自动重连。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_Environment           环境缺少关键配置文件。
 * @retval NeuracleV4Result_Error_MemoryAllocFailed     内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerInitialize(NeuracleV4Controller** controller, bool autoReconnect);

/**
 * @brief 通知设备控制器：不再将Trigger数据包与脑电数据包合并成一个整体。
 * @note  本API必须在\ref NeuracleV4ControllerInitialize "NeuracleV4ControllerInitialize"之后、\ref NeuracleV4ControllerStart "NeuracleV4ControllerStart"之前调用；一旦设置成功，该状态在controller的生命周期内一直有效。
 * @details 默认情况下，SDK内部会等待Trigger盒采集到的事件数据到达后，将其填入\ref DataBlockV4 "DataBlockV4"的Markers数组，
 *          并与脑电数据一起通过\ref NeuracleV4ControllerReadData "NeuracleV4ControllerReadData"返回。<br>
 *          调用本API后，SDK不再执行上述合并动作：脑电数据到达即返回，不再等待Trigger数据，\ref DataBlockV4 "DataBlockV4"的Markers字段保持为空指针。
 *          适用于Trigger数据由上层软件另行处理、无需依赖SDK合并的场景。
 *
 * @param[in]   controller                              设备控制器结构体的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数，controller为空指针。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerSkipMergeTriggerToEEGPacket(NeuracleV4Controller* controller);

/**
 * @brief 销毁设备控制器结构体的实例。
 *
 * @param[in]   controller                              设备控制器结构体的实例。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  void NeuracleV4ControllerFinalize(NeuracleV4Controller* controller);

/**
 * @brief 添加脑电主机(设备)。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  ipAddress                                IPV4地址字符串。
 * @param[in]  port                                     设备TCP侦听的端口号。
 * @param[in]  deviceType                               设备类型字符串。
 * @param[in]  maxDelayMilliSeconds                     \ref NeuracleV4ControllerReadData "NeuracleV4ControllerReadData" 返回数据时的固定时间长度(毫秒)，
 *                                                      最小是1，最大是1000。它和sampleRate参数以及\ref DataBlockV4 "DataBlockV4"中DataCountPerChannel密切相关。
 *                                                      比如采样率250，那么它至少是4毫秒，即 >= 1000 / sampleRate，且是4的倍数。这样才能保证
 *                                                      \ref DataBlockV4 "DataBlockV4"的DataCountPerChannel大于1。<br> <br>
 *                                                      另外该参数请使用偶数。<br> <br>
 *                                                      如果是采集阻抗，这里必须传入1000，这样\ref NeuracleV4ControllerReadImpedance "NeuracleV4ControllerReadImpedance"每间隔1秒刷新一次新的数据。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 * @retval NeuracleV4Result_Error_InvalidEEGHostType    无效设备类型，非脑电设备。
 * @retval NeuracleV4Result_Error_MoreThanOneHost       不支持1个以上设备，调用者已经添加了1个。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerAddEEGHost(NeuracleV4Controller* controller, char* ipAddress, int port, const unsigned char* deviceType, int maxDelayMilliSeconds);

/**
 * @brief 删除脑电主机(设备)。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  ipAddress                                IPV4地址字符串。
 * @param[in]  port                                     设备TCP侦听的端口号。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerRemoveEEGHost(NeuracleV4Controller* controller, char* ipAddress, int port);

/**
 * @brief 读取脑电主机状态。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[out] deviceStatus                             设备状态。
 * @param[in]  targetDeviceSerialNumber                 设备序列号，由于只支持单个主机、单个探头和单个传感器，所以这里请使用0。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                         成功，<strong>*deviceStatus</strong>返回状态。
 * @retval NeuracleV4Result_Error_InvalidHandle             无效句柄，controller非法。
 * @retval NeuracleV4Result_Error_InvalidParameter          无效参数。
 * @retval NeuracleV4Result_Warning_DongleLost              授权狗被拔出。
 * @retval NeuracleV4Result_Error_InvalidDeviceController   无效的内部设备控制器。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerGetEEGHostStatus(NeuracleV4Controller* controller, NeuracleV4DeviceStatus *deviceStatus, unsigned int targetDeviceSerialNumber);

/**
 * @brief 添加同步盒设备。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  ipAddress                                IPV4地址字符串。
 * @param[in]  port                                     设备TCP侦听的端口号。
 * @param[in]  deviceType                               设备类型字符串。
 * @param[in]  firmwareVersion                          固件版本。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 * @retval NeuracleV4Result_Error_InvalidSyncboxType    无效同步盒设备类型。
 * @retval NeuracleV4Result_Error_InstanceExisted       调用者已经添加了同步盒。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerAddTriggerBox(NeuracleV4Controller* controller, char* ipAddress, int port, const unsigned char* deviceType, unsigned char firmwareVersion);

/**
 * @brief 删除同步盒设备。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  ipAddress                                IPV4地址字符串。
 * @param[in]  port                                     设备TCP侦听的端口号。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerRemoveTriggerBox(NeuracleV4Controller* controller, char* ipAddress, int port);

/**
 * @brief 读取同步盒状态。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[out] deviceStatus                             设备状态。
 * @param[in]  targetDeviceSerialNumber                 设备序列号，由于只支持单个主机、单个探头和单个传感器，所以这里请使用0。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                         成功，<strong>*deviceStatus</strong>返回状态。
 * @retval NeuracleV4Result_Error_InvalidHandle             无效句柄，controller非法。
 * @retval NeuracleV4Result_Error_InvalidParameter          无效参数。
 * @retval NeuracleV4Result_Warning_DongleLost              授权狗被拔出。
 * @retval NeuracleV4Result_Error_InvalidDeviceController   无效的内部设备控制器。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerGetTriggerBoxStatus(NeuracleV4Controller* controller, NeuracleV4DeviceStatus *deviceStatus, unsigned int targetDeviceSerialNumber);

/**
 * @brief 添加探头。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  moduleSerialNumber                       序列号。
 * @param[in]  moduleDeviceType                         设备类型字符串。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 * @retval NeuracleV4Result_Error_InvalidEEGModuleType  无效脑电探头类型。
 * @retval NeuracleV4Result_Error_InvalidDeviceController 无效的内部设备控制器。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerAddModule(NeuracleV4Controller* controller, unsigned int moduleSerialNumber, char* moduleDeviceType);

/**
 * @brief 删除探头。各项参数的主机信息应来自设备发现API的返回值。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  moduleSerialNumber                       序列号。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerRemoveModule(NeuracleV4Controller* controller, unsigned int moduleSerialNumber);

/**
 * @brief 连接设备。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 * @retval NeuracleV4Result_Error_StartTriggerFailed    启动同步盒失败。
 * @retval NeuracleV4Result_Error_StartAmplifyFailed    启动采集设备失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerConnect(NeuracleV4Controller* controller);

/**
 * @brief 根据传感器类型名称读取传感器的设备类型的指针，该值不能释放。
 *
 * @param[in]  sensorTypeName                           传感器类型名称。
 *
 * @return 执行结果
 *
 * @retval 非NULL                                       成功。
 * @retval NULL                                         无效参数。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  struct SensorDeviceType* NeuracleV4GetEEGSensorDeviceTypeByName(char* sensorTypeName);

/**
 * @brief 设置脑电参数。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[in]  moduleSerialNumber                       探头序列号。
 * @param[in]  boardId                                  主板序号。
 * @param[in]  cpuId                                    CPU序号。
 * @param[in]  pSensorDeviceType                        传感器设备类型的指针。
 * @param[in]  sensorType                               传感器类型。
 * @param[in]  gain                                     增益。
 * @param[in]  workMode                                 工作模式。
 * @param[in]  channelCount                             通道数量。
 * @param[in]  sampleRate                               采样率。
 * @param[in]  deviceSerialNumber                       设备序列号。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidDeviceController 无效的内部设备控制器。
 * @retval NeuracleV4Result_Error_MoreThanOneModule     超过1个探头。
 * @retval NeuracleV4Result_Error_InvalidParameter      无效参数。
 * @retval NeuracleV4Result_System_Error_NoEnoughMemory 内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerSetEEGParameter(NeuracleV4Controller* controller,
                                                                                unsigned int moduleSerialNumber,
                                                                                unsigned int boardId,
                                                                                unsigned short cpuId,
                                                                                SensorDeviceType* pSensorDeviceType,
                                                                                SensorTypeV4 sensorType,
                                                                                unsigned short gain,
                                                                                WorkModeV4 workMode,
                                                                                unsigned short channelCount,
                                                                                unsigned short sampleRate,
                                                                                unsigned int deviceSerialNumber);

/**
 * @brief 启动采集信号。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidHandle         无效句柄，controller非法。
 * @retval NeuracleV4Result_System_Error_NoEnoughMemory 内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerStart(NeuracleV4Controller* controller);

/**
 * @brief 启动采集阻抗。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_DoggleVerifyFailed    授权狗校验失败，原因可能是没插狗或者狗已过期。请插入合法的狗。
 * @retval NeuracleV4Result_Error_InvalidHandle         无效句柄，controller非法。
 * @retval NeuracleV4Result_System_Error_NoEnoughMemory 内存不足，请确认应用软件是否存在内存泄露、导致SDK申请内存失败。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerStartImpedance(NeuracleV4Controller* controller);

/**
 * @brief 停止采集。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                     成功。
 * @retval NeuracleV4Result_Error_InvalidHandle         无效句柄，controller非法。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerStop(NeuracleV4Controller* controller);

/**
 * @brief 读取设备采集信号。
 *
 * @param[in]  controller                               设备控制器结构体的实例。
 * @param[out] dataBlock                                采样数据块的二级指针。如果有数据，SDK内部将分配一个\ref
 *                                                      DataBlock "DataBlock"实例和其对应采样的空间，并填充数据，
 *                                                      然后让<strong>*dataBlock</strong>指向该实例地址。
 * @param[in]  targetDeviceSerialNumber                 设备序列号，由于只支持单个主机、单个探头和单个传感器，所以这里请使用0。
 * @param[out] remainingDataLen                         返回剩余尚未读取的数据的个数
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                         成功，<strong>*dataBlock</strong>返回数据块地址。
 * @retval NeuracleV4Result_Error_InvalidParameter          无效参数。
 * @retval NeuracleV4Result_Error_InvalidHandle             无效句柄，controller非法。
 * @retval NeuracleV4Result_Error_InvalidDeviceController   无效的内部设备控制器。
 * @retval NeuracleV4Result_Warning_DongleLost              授权狗被拔出。
 * @retval NeuracleV4Result_Warning_NoEnoughData            没有数据，由于本API是轮询而非阻塞的方式读取采样，这使得
 *                                                          调用频率和采样抵达间隔不可能一摸一样，故可能返回该值。
 *                                                          调用者可以控制调用间隔和前述maxDelayMilliSeconds差不多。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerReadData(NeuracleV4Controller* controller, DataBlockV4** dataBlock, unsigned int targetDeviceSerialNumber, int *remainingDataLen);

/**
 * @brief 释放设备采集信号。
 *
 * @param[in] dataBlock                                     \ref NeuracleV4ControllerReadData "NeuracleV4ControllerReadData"返回的采样数据块。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  void NeuracleV4FreeDataBlock(DataBlockV4* dataBlock);

/**
 * @brief 读取设备阻抗。
 *
 * @param[in]  controller                                 设备控制器结构体的实例。
 * @param[out] impedance                                  由调用者分配的阻抗数据数组，数组长度是传入时的*count。
 * @param[in,out] count                                   impedance数组长度，传入时其应大于或等于调用\ref NeuracleV4ControllerSetEEGParameter "NeuracleV4ControllerSetEEGParameter"时传入的通道数。<br><br>
 *                                                        返回时该值等于阻抗数据的通道数量。
 * @param[in]  targetDeviceSerialNumber                   设备序列号，由于只支持单个主机、单个探头和单个传感器，所以这里请使用0。
 *
 * @return 执行结果
 *
 * @retval NeuracleV4Result_Succeed                       成功，<strong>*impedance</strong>返回数据块地址。
 * @retval NeuracleV4Result_Error_InvalidHandle           无效句柄，controller非法。
 * @retval NeuracleV4Result_Error_InvalidDeviceController 无效的内部设备控制器。
 * @retval NeuracleV4Result_Warning_NeedMoreSpace         需要更多空间，*count小于通道数。
 * @retval NeuracleV4Result_Warning_DongleLost            授权狗被拔出。
 * @retval NeuracleV4Result_Information_ValueNoChanging   阻抗没变化，由于本API是轮询而非阻塞的方式读取采样，这使得
 *                                                        调用频率和阻抗抵达间隔不可能一摸一样，故可能返回该值。
 *                                                        调用者可以控制调用间隔和1秒差不多，使得降低系统CPU使用率。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  NeuracleV4ResultCode NeuracleV4ControllerReadImpedance(NeuracleV4Controller* controller, float* impedance, int* count, unsigned int targetDeviceSerialNumber);

/**
 * @brief 是否有效的同步盒设备类型。
 *
 * @param[in]  deviceType                               同步盒的设备类型。
 *
 * @return 执行结果
 *
 * @retval true                                         是。
 * @retval false                                        否。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  bool NeuracleV4IsValidSyncBox(const unsigned char* deviceType);

/**
 * @brief 是否有效的主机设备类型。
 *
 * @param[in]  deviceType                               主机的设备类型。
 *
 * @return 执行结果
 *
 * @retval true                                         是。
 * @retval false                                        否。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  bool NeuracleV4IsValidEEGHost(const unsigned char* deviceType);

/**
 * @brief 是否有效的脑电传感器设备类型。
 *
 * @param[in]  deviceType                               脑电传感器的设备类型。
 *
 * @return 执行结果
 *
 * @retval true                                         是。
 * @retval false                                        否。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  bool NeuracleV4IsValidEEGSensor(const unsigned char* deviceType);

/**
 * @brief 是否有效的脑电探头设备类型。
 *
 * @param[in]  deviceType                               脑电探头的设备类型。
 *
 * @return 执行结果
 *
 * @retval true                                         是。
 * @retval false                                        否。
 */
NEURACLE_W4SDK_LIBRARY_EXPORT  bool NeuracleV4IsValidEEGModule(const unsigned char* deviceType);

#ifdef __cplusplus
}
#endif
#endif
