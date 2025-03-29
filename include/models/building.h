#ifndef BUILDING_H
#define BUILDING_H

#include "db/database.h"
#include "db/db_query.h"
#include "auth/auth.h"
#include <stdbool.h>

// 楼宇信息
typedef struct
{
    char building_id[40];
    char building_name[64];
    char address[256];
    int floors_count;
} Building;

// 添加楼宇
bool add_building(Database *db, const char *user_id, UserType user_type, Building *building);

// 修改楼宇信息
bool update_building(Database *db, const char *user_id, UserType user_type, Building *building);

// 删除楼宇
bool delete_building(Database *db, const char *user_id, UserType user_type, const char *building_id);

// 获取楼宇信息
bool get_building(Database *db, const char *building_id, Building *building);

// 获取所有楼宇列表
bool list_buildings(Database *db, const char *user_id, UserType user_type, QueryResult *result);

// 分配服务人员到楼宇
bool assign_staff_to_building(Database *db, const char *user_id, UserType user_type, const char *staff_id, const char *building_id);

// 取消服务人员的楼宇分配
bool unassign_staff_from_building(Database *db, const char *user_id, UserType user_type, const char *staff_id, const char *building_id);

#endif /* BUILDING_H */
// ...existing code...

/**
 * 获取未缴费业主数量
 */
bool get_unpaid_owners_count(Database *db, int *count);

/**
 * 按缴费状态统计业主数量
 */
bool get_owners_count_by_payment_status(Database *db, int *paid_count, int *unpaid_count);

/**
 * 获取指定年份未缴费业主列表
 */
bool get_unpaid_owners_by_year(Database *db, int year, QueryResult *result);

/**
 * 获取截至指定日期已缴费业主数量
 */
bool get_paid_owners_count_by_date(Database *db, const char *end_date, int *count);
/**
 * 统计未缴费业主数量
 */
bool get_unpaid_owners_count(Database *db, int *count);