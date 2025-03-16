#ifndef BUILDING_H
#define BUILDING_H

#include "db/database.h"
#include "db/db_query.h"
#include <stdbool.h>

// 小区信息
typedef struct
{
    char id[32];
    char name[64];
} Community;

// 楼宇信息
typedef struct
{
    char id[32];
    char building_number[16];
    char community_id[32];
    int floors;
    int units_per_floor;
} Building;

// 添加小区
bool add_community(Database *db, const char *token, Community *community);

// 添加楼宇
bool add_building(Database *db, const char *token, Building *building);

// 修改楼宇信息
bool update_building(Database *db, const char *token, Building *building);

// 删除楼宇
bool delete_building(Database *db, const char *token, const char *building_id);

// 获取楼宇信息
bool get_building(Database *db, const char *building_id, Building *building);

// 获取所有楼宇列表
bool list_buildings(Database *db, const char *token, QueryResult *result);

// 分配服务人员到楼宇
bool assign_staff_to_building(Database *db, const char *token, const char *staff_id, const char *building_id);

// 取消服务人员的楼宇分配
bool unassign_staff_from_building(Database *db, const char *token, const char *staff_id, const char *building_id);

#endif /* BUILDING_H */
