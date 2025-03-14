#ifndef APARTMENT_H
#define APARTMENT_H

#include "db/database.h"
#include "db/db_query.h"
#include <stdbool.h>

// 房屋信息
typedef struct
{
    char id[32];
    char building_id[32];
    int floor;
    int unit;
    float area;
    bool occupied;
    float property_fee_balance;
    bool fee_paid;
} Apartment;

// 添加房屋
bool add_apartment(Database *db, const char *token, Apartment *apartment);

// 修改房屋信息
bool update_apartment(Database *db, const char *token, Apartment *apartment);

// 删除房屋
bool delete_apartment(Database *db, const char *token, const char *apartment_id);

// 获取房屋信息
bool get_apartment(Database *db, const char *apartment_id, Apartment *apartment);

// 获取某楼宇内所有房屋
bool list_apartments_by_building(Database *db, const char *token, const char *building_id, QueryResult *result);

// 查询业主的房屋
bool get_owner_apartments(Database *db, const char *token, const char *owner_id, QueryResult *result);

#endif /* APARTMENT_H */
