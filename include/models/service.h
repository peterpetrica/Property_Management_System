#ifndef SERVICE_H
#define SERVICE_H

#include "db/database.h"
#include "db/db_query.h"
#include <stdbool.h>
#include <time.h>

// 服务类型
typedef struct
{
    char id[32];
    char name[64];
    char description[256];
} ServiceType;

// 服务记录
typedef struct
{
    char id[32];
    char staff_id[32];
    char building_id[32];
    char apartment_id[32];
    char service_type_id[32];
    time_t service_time;
    char description[256];
} ServiceRecord;

// 添加服务类型
bool add_service_type(Database *db, const char *token, ServiceType *type);

// 修改服务类型
bool update_service_type(Database *db, const char *token, ServiceType *type);

// 删除服务类型
bool delete_service_type(Database *db, const char *token, const char *type_id);

// 获取所有服务类型
bool list_service_types(Database *db, QueryResult *result);

// 记录服务
bool record_service(Database *db, const char *token, ServiceRecord *record);

// 获取服务人员的服务记录
bool get_service_records_by_staff(Database *db, const char *token, const char *staff_id, QueryResult *result);

// 获取房屋的服务记录
bool get_service_records_by_apartment(Database *db, const char *token, const char *apartment_id, QueryResult *result);

#endif /* SERVICE_H */
