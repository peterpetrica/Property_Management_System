#ifndef SERVICE_H
#define SERVICE_H

#include "db/database.h"
#include "db/db_query.h"
#include "auth/auth.h"
#include <stdbool.h>
#include <time.h>

// 服务类型
typedef struct
{
    char id[40];
    char name[64];
    char description[256];
} ServiceType;

// 服务人员类型
typedef struct
{
    char staff_type_id[40];
    char type_name[64];
    char description[256];
} StaffType;

// 服务区域
typedef struct
{
    char area_id[40];
    char staff_id[40];
    char building_id[40];
    time_t assignment_date;
} ServiceArea;

// 服务记录
typedef struct
{
    char record_id[40];
    char staff_id[40];
    char service_type[64];
    time_t service_date;
    char description[256];
    int status;
    char target_id[40]; // 目标ID：楼宇ID或房屋ID
} ServiceRecord;

bool assign_staff_to_building(Database *db, const char *admin_user_id, UserType admin_type, const char *staff_user_id, const char *building_id);

// 添加服务类型
bool add_service_type(Database *db, const char *user_id, UserType user_type, ServiceType *type);

// 修改服务类型
bool update_service_type(Database *db, const char *user_id, UserType user_type, ServiceType *type);

// 删除服务类型
bool delete_service_type(Database *db, const char *user_id, UserType user_type, const char *type_id);

// 获取所有服务类型
bool list_service_types(Database *db, QueryResult *result);

// 添加服务人员类型
bool add_staff_type(Database *db, const char *user_id, UserType user_type, StaffType *type);

// 修改服务人员类型
bool update_staff_type(Database *db, const char *user_id, UserType user_type, StaffType *type);

// 删除服务人员类型
bool delete_staff_type(Database *db, const char *user_id, UserType user_type, const char *staff_type_id);

// 获取所有服务人员类型
bool list_staff_types(Database *db, QueryResult *result);

// 分配服务区域
bool assign_service_area(Database *db, const char *user_id, UserType user_type, ServiceArea *area);

// 取消服务区域分配
bool unassign_service_area(Database *db, const char *user_id, UserType user_type, const char *area_id);

// 获取服务人员的服务区域
bool get_staff_service_areas(Database *db, const char *user_id, UserType user_type, const char *staff_id, QueryResult *result);

// 记录服务
bool record_service(Database *db, const char *user_id, UserType user_type, ServiceRecord *record);

// 获取服务人员的服务记录
bool get_service_records_by_staff(Database *db, const char *user_id, UserType user_type, const char *staff_id, QueryResult *result);

// 获取房屋的服务记录
bool get_service_records_by_room(Database *db, const char *user_id, UserType user_type, const char *room_id, QueryResult *result);

// 获取楼宇的服务记录
bool get_service_records_by_building(Database *db, const char *user_id, UserType user_type, const char *building_id, QueryResult *result);

#endif /* SERVICE_H */
