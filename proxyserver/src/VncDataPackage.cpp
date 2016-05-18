#include <string>
#include <map>
#include <list>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "VncDataPackage.h"
#include "VncUtils.h"
#include "json.h"

#define PKG_WRAP_SIZE           (8)
#define PKG_START_OFFSET        (4)
#define PKG_FIXED_PART_SIZE     (32)

VncDataPackage::VncDataPackage()
{
    m_dirty = true;
}

VncDataPackage::~VncDataPackage()
{
}

// 通过原始协议包数据创建
VncDataPackage::VncDataPackage(const unsigned char* rawdata, unsigned int  datasize)
    : m_protocolbuffer((const char*)rawdata, datasize)
{
    unsigned cnt = 0;
    const unsigned char * p;
    unsigned paramLength;

    if (rawdata == nullptr || datasize <= 8)
        return;

    m_pkgSize = datasize;
    m_crc32 = (rawdata[datasize - 4] << 24) | (rawdata[datasize - 3] << 16) | (rawdata[datasize - 2] << 8) | (rawdata[datasize - 1]);
    m_dirty = false;

    p = &rawdata[PKG_START_OFFSET];

    // target
    m_content.m_target = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;

    // source
    m_content.m_source = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;

    // version
    m_content.m_version = p[cnt];
    cnt += 1;

    // paramType
    m_content.m_paramType = p[cnt];
    cnt += 1;

    // skip
    cnt += 2;

    // command
    m_content.m_command = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;

    // tag
    m_content.m_tag = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;

    // length of param
    paramLength = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;

    // param
    if (m_content.m_paramType != CMD_ARGUMENT_TYPE_NONE)
    {
        if (paramLength)
            m_content.m_bufParam = std::string((char*)(&p[cnt]), paramLength);
        else
            m_content.m_bufParam = "";
        cnt += paramLength;

        if (m_content.m_paramType == CMD_ARGUMENT_TYPE_INT32)
        {
            m_content.m_intParam = atoi(m_content.m_bufParam.c_str());
        }
        else if (m_content.m_paramType == CMD_ARGUMENT_TYPE_KEYVALUE)
        {
            Json::Reader reader;
            reader.parse(m_content.m_bufParam, m_content.m_jsonroot);
        }
    }
    else
    {
        m_content.m_bufParam = "";
    }
}

// 通过数据包包体内容构建数据包
VncDataPackage::VncDataPackage(const VncPackContent& content)
{
    m_content = content;
    m_dirty = true;
}

// 通过数据包包体内容构建数据包
VncDataPackage::VncDataPackage(const VncPackContent* pcontent)
{
    m_content = *pcontent;
    m_dirty = true;
}

// 得到数据包的包体内容信息
const VncPackContent* VncDataPackage::getPackContent()
{
    return &m_content;
}

// 取得数据包的原始的协议的数据
std::string & VncDataPackage::getProtocolBuffer()
{
    if (m_dirty)
    {
        makeNewProtocolBuffer();
    }
    return m_protocolbuffer;
}

// 从原始协议数据中获得 “目标” 与 “源” 信息
int VncDataPackage::getSourceAndTargetInfo(const unsigned char* rawdata, unsigned rawsize, unsigned & source, unsigned & target)
{
    const unsigned char * p;
    unsigned cnt = 0;
    p = &rawdata[PKG_START_OFFSET];

    target = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);
    cnt += 4;
    source = (p[cnt + 0] << 24) | (p[cnt + 1] << 16) | (p[cnt + 2] << 8) | (p[cnt + 3]);

    return 0;
}

int VncDataPackage::makeNewProtocolBuffer()
{
    unsigned int v32;
    unsigned short v16;
    unsigned char v8;
    
    if (m_content.m_paramType == CMD_ARGUMENT_TYPE_INT32)
    {
        char tmpbuf[128];
        snprintf(tmpbuf, sizeof(tmpbuf), "%d", m_content.m_intParam);
        m_content.m_bufParam = tmpbuf;
    }
    else if (m_content.m_paramType == CMD_ARGUMENT_TYPE_KEYVALUE)
    {
        Json::FastWriter writer;
        m_content.m_bufParam = writer.write(m_content.m_jsonroot);
    }

    unsigned cnt = 0;
    unsigned char * p;          // temp buffer
    m_pkgSize = PKG_FIXED_PART_SIZE + m_content.m_bufParam.length();
    p = new unsigned char[m_pkgSize];

    // 包长度
    v32 = htonl(m_pkgSize);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // target
    v32 = htonl(m_content.m_target);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // source
    v32 = htonl(m_content.m_source);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // version
    v8 = (unsigned char)m_content.m_version;
    memcpy(&p[cnt], &v8, 1);
    cnt += 1;

    // paramType 
    v8 = (unsigned char)m_content.m_paramType;
    memcpy(&p[cnt], &v8, 1);
    cnt += 1;

    // reserve  
    v16 = 0;
    memcpy(&p[cnt], &v16, 2);
    cnt += 2;

    // command
    v32 = htonl(m_content.m_command);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // tag
    v32 = htonl(m_content.m_tag);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // arraylength (参数长度)
    v32 = htonl(m_content.m_bufParam.length());
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // 参数
    memcpy(&p[cnt], m_content.m_bufParam.c_str(), m_content.m_bufParam.length());
    cnt += m_content.m_bufParam.length();

    // CRC32
    m_crc32 = crc32_hash(p, m_pkgSize - 4);
    v32 = htonl(m_crc32);
    memcpy(&p[cnt], &v32, 4);
    cnt += 4;

    // 
    m_protocolbuffer = std::string((char*)p, m_pkgSize);
    delete[] p;

    m_dirty = false;
    return 0;
}


// 设置源与目标
void VncDataPackage::setSourceAndTarget(unsigned source, unsigned target)
{
    m_content.m_source = source;
    m_content.m_target = target;
    m_dirty = true;
}

// 设置命令ID
void VncDataPackage::setCommandId(unsigned cmd, unsigned tag)
{
    m_content.m_command = cmd;
    m_content.m_tag = tag;
    m_dirty = true;
}

// 设置整数参数
void VncDataPackage::setIntegerParam(unsigned param)
{
    m_content.m_paramType = CMD_ARGUMENT_TYPE_INT32;
    m_content.m_intParam = param;
    m_dirty = true;
}

// 设置字符串参数
void VncDataPackage::setStringParam(const std::string & param)
{
    m_content.m_paramType = CMD_ARGUMENT_TYPE_STRING;
    m_content.m_bufParam = param;
    m_dirty = true;
}

// 添加Key-Value配对参数
void VncDataPackage::addKeyValueParam(const std::string & key, const std::string & value)
{
    m_content.m_paramType = CMD_ARGUMENT_TYPE_KEYVALUE;
    m_content.m_intParam = 0;
    m_content.m_jsonroot[key] = value;
    m_dirty = true;
}

// 获取包内容值
unsigned VncDataPackage::getSource() { return m_content.m_source; };
unsigned VncDataPackage::getTarget() { return m_content.m_target; };
unsigned VncDataPackage::getTag() { return m_content.m_tag; };
unsigned VncDataPackage::getCommandId() { return m_content.m_command; };
CmdArgumentType VncDataPackage::getParamType() { return (CmdArgumentType)m_content.m_paramType; };

unsigned VncDataPackage::getInt32Param() 
{ 
    if (m_content.m_paramType == CMD_ARGUMENT_TYPE_INT32)
        return m_content.m_intParam;
    else
        return 0;
};

const std::string & VncDataPackage::getStringParam() 
{
    static std::string nullstring("");
    if (m_content.m_paramType == CMD_ARGUMENT_TYPE_STRING)
        return m_content.m_bufParam;
    else
        return nullstring;
};

const std::string & VncDataPackage::getKeyValue(const std::string & key) 
{  
    static std::string nullstring("");
    if (m_content.m_paramType == CMD_ARGUMENT_TYPE_KEYVALUE)
        nullstring = m_content.m_jsonroot[key].asString();
    else
        nullstring = "";
    return nullstring;
};

unsigned VncDataPackage::getCrc32()
{
    if (m_dirty)
    {
        makeNewProtocolBuffer();
    }
    return m_crc32;
}

// 创建数据包

VncDataPackage * VncDataPackage::create(unsigned source, unsigned target, unsigned cmd, unsigned tag)
{
    auto package = new VncDataPackage();
    auto content = &package->m_content;
    
    content->m_source = source;
    content->m_target = target;
    content->m_command = cmd;
    content->m_tag = tag;

    package->m_dirty = true;
    return package;
}

VncDataPackage * VncDataPackage::create(unsigned source, unsigned target, unsigned cmd, unsigned tag, unsigned param)
{
    auto package = new VncDataPackage();
    auto content = &package->m_content;

    content->m_source = source;
    content->m_target = target;
    content->m_command = cmd;
    content->m_tag = tag;
    content->m_paramType = CMD_ARGUMENT_TYPE_INT32;
    content->m_intParam = param;

    package->m_dirty = true;
    return package;
}

VncDataPackage * VncDataPackage::create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::string & param)
{
    auto package = new VncDataPackage();
    auto content = &package->m_content;

    content->m_source = source;
    content->m_target = target;
    content->m_command = cmd;
    content->m_tag = tag;
    content->m_paramType = CMD_ARGUMENT_TYPE_STRING;
    content->m_bufParam = param;

    package->m_dirty = true;
    return package;
}

VncDataPackage * VncDataPackage::create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::map<std::string, std::string> & params)
{
    auto package = new VncDataPackage();
    auto content = &package->m_content;

    content->m_source = source;
    content->m_target = target;
    content->m_command = cmd;
    content->m_tag = tag;
    content->m_paramType = CMD_ARGUMENT_TYPE_KEYVALUE;
    std::map<std::string, std::string>::const_iterator it;
    for (it = params.begin(); it != params.end(); it++)
    {
        content->m_jsonroot[it->first] = it->second;
    }

    package->m_dirty = true;
    return package;
}

VncDataPackage * VncDataPackage::create(unsigned source, unsigned target, unsigned cmd, unsigned tag, const std::string & param, bool isJsonString)
{
    auto package = new VncDataPackage();
    auto content = &package->m_content;

    content->m_source = source;
    content->m_target = target;
    content->m_command = cmd;
    content->m_tag = tag;
    if (isJsonString == false)
    {
        content->m_paramType = CMD_ARGUMENT_TYPE_STRING;
        content->m_bufParam = param;
    }
    else
    {
        content->m_paramType = CMD_ARGUMENT_TYPE_KEYVALUE;
        Json::Reader reader;
        reader.parse(param, content->m_jsonroot);
        content->m_bufParam = param;
    }

    package->m_dirty = true;
    return package;
}







