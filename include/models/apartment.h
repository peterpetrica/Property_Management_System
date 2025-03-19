#ifndef APARTMENT_H
#define APARTMENT_H

#include "db/database.h"
#include "db/db_query.h"
#include "auth/auth.h"
#include <stdbool.h>

// 房屋信息
typedef struct
{
    char room_id[64];
    char building_id[64];
    char room_number[20];
    int floor;
    float area_sqm;
    char owner_id[64];
    char status[64];
} Room;

// 添加房屋
bool add_room(Database *db, const char *user_id, UserType user_type, Room *room);

// 修改房屋信息
bool update_room(Database *db, const char *user_id, UserType user_type, Room *room);

// 删除房屋
bool delete_room(Database *db, const char *user_id, UserType user_type, const char *room_id);

// 获取房屋信息
bool get_room(Database *db, const char *room_id, Room *room);

// 获取某楼宇内所有房屋
bool list_rooms_by_building(Database *db, const char *user_id, UserType user_type, const char *building_id, QueryResult *result);

// 查询业主的房屋
bool get_owner_rooms(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result);

#endif /* APARTMENT_H */
