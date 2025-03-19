#ifndef PARKING_H
#define PARKING_H

#include "db/database.h"
#include "db/db_query.h"
#include "auth/auth.h"
#include <stdbool.h>

// 停车位信息
typedef struct
{
    char parking_id[40];
    char parking_number[16];
    char owner_id[40];
    int status;
} ParkingSpace;

// 添加停车位
bool add_parking_space(Database *db, const char *user_id, UserType user_type, ParkingSpace *space);

// 修改停车位信息
bool update_parking_space(Database *db, const char *user_id, UserType user_type, ParkingSpace *space);

// 删除停车位
bool delete_parking_space(Database *db, const char *user_id, UserType user_type, const char *parking_id);

// 获取停车位信息
bool get_parking_space(Database *db, const char *parking_id, ParkingSpace *space);

// 获取所有停车位
bool list_parking_spaces(Database *db, const char *user_id, UserType user_type, QueryResult *result);

// 获取业主的停车位
bool get_owner_parking_spaces(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result);

#endif /* PARKING_H */
