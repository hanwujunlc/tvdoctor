#include <string>
#include <map>
#include <list>
#include <vector>
#ifndef COOCAA_OS_VNC_DATAPACKAGE_H_
#define COOCAA_OS_VNC_DATAPACKAGE_H_

#include "json.h"

class VncContent;

enum CmdArgumentType
{
    CMD_ARGUMENT_TYPE_NONE = 0x0000,
    CMD_ARGUMENT_TYPE_INT32 = 0x0001,
    CMD_ARGUMENT_TYPE_STRING = 0x0002,
    CMD_ARGUMENT_TYPE_KEYVALUE = 0x0003,
};

struct  VncPackContent
{
    unsigned int            m_target;
    unsigned int            m_source;
    unsigned int            m_version;
    unsigned int            m_tag;
    unsigned int            m_command;
    unsigned int            m_paramType;
    std::string             m_bufParam;
    /////
    unsigned int            m_intParam;
    Json::Value             m_jsonroot;

    VncPackContent()
    {
        m_target = 0;
        m_source = 0;
        m_version = 1;
        m_tag = 0;
        m_command = 0;
        m_paramType = CMD_ARGUMENT_TYPE_NONE;
        m_intParam = 0;
    }
};


class VncDataPackage
{
public:
    VncDataPackage();
    ~VncDataPackage();

    // 通过原始协议包数据创建
    VncDataPackage(const unsigned char* rawdata, unsigned int  datasize);

    // 通过数据包包体内容构建数据包
    VncDataPackage(const VncPackContent& content);
    VncDataPackage(const VncPackContent* pcontent);

    // 得到数据包的包体内容信息
    const VncPackContent* getPackContent();

    // 取得数据包的原始的协议的数据
    std::string & getProtocolBuffer();
    
    // 从原始协议数据中获得 “目标” 与 “源” 信息
    static int getSourceAndTargetInfo(const unsigned char* rawdata, unsigned rawsize, unsigned & source, unsigned & target);

    // 设置源与目标
    void setSourceAndTarget(unsigned source, unsigned target);

    // 设置命令ID
    void setCommandId(unsigned cmd, unsigned tag);

    // 设置整数参数
    void setIntegerParam(unsigned param);

    // 设置字符串参数
    void setStringParam(const std::string & param);

    // 添加Key-Value配对参数
    void addKeyValueParam(const std::string & key, const std::string & value);

    // 获取包内容值
    unsigned getSource();
    unsigned getTarget();
    unsigned getTag();
    unsigned getCommandId();
    CmdArgumentType getParamType();
    unsigned getInt32Param();
    const std::string & getStringParam();
    const std::string & getKeyValue(const std::string & key);
    unsigned getCrc32();

    // 创建数据包
    static VncDataPackage * create(unsigned source, unsigned target, unsigned cmd, unsigned tag);
    static VncDataPackage * create(unsigned source, unsigned target, unsigned cmd, unsigned tag, unsigned param);
    static VncDataPackage * create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::string & param);
    static VncDataPackage * create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::map<std::string, std::string> & params);
    static VncDataPackage * create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::string & param, bool isJsonString);

private:
    int makeNewProtocolBuffer();
    std::string         m_protocolbuffer;       // 数据包原始协议数据
    VncPackContent      m_content;              // 数据包已经解析出的内容
    unsigned int        m_pkgSize;              // 包长度 表示整个包的长度大小，包括所有字段
    unsigned int        m_crc32;                // 校验和 为整个包的CRC32校验和，除了CRC32本身的4字节外，整个包的其他数据作CRC32运算的结果
    bool                m_dirty;                // 标识包内容是否已经修改
};














#endif

