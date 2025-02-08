#ifndef GLOBAL_H
#define GLOBAL_H

#include "Tool.h"
#include "NetworkService.h"

extern  Tool* pUtils;						//指向唯一的工具类实例，只在main函数结束前delete
// 提供常用工具函数
extern  NetworkService* pns;				//指向唯一的模拟网络环境类实例，只在main函数结束前delete
// 用于模拟网络数据包的发送和接收，负责管理整个模拟环境下的通信。
#endif

