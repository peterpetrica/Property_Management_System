/**
 * db_init.c
 * 数据库初始化模块
 *
 * 本模块负责物业管理系统数据库的初始化工作，包括：
 * 1. 创建所有必要的数据库表（用户、角色、楼宇、房屋等）
 * 2. 设置表之间的外键关系
 * 3. 初始化默认管理员账户
 *
 * 主要功能：
 * - db_init_tables: 创建所有必要的数据库表
 * - db_init_admin: 初始化默认管理员账户
 */

#include "db/database.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 创建角色表
static const char *CREATE_ROLES_TABLE =
    "CREATE TABLE IF NOT EXISTS roles ("
    "role_id TEXT PRIMARY KEY,"
    "role_name TEXT NOT NULL,"
    "permission_level INTEGER NOT NULL"
    ");";

// 创建用户表
static const char *CREATE_USERS_TABLE =
    "CREATE TABLE IF NOT EXISTS users ("
    "user_id TEXT PRIMARY KEY,"
    "username TEXT NOT NULL UNIQUE,"
    "password_hash TEXT NOT NULL,"
    "name TEXT NOT NULL,"
    "phone_number TEXT,"
    "email TEXT,"
    "role_id TEXT NOT NULL,"
    "status INTEGER DEFAULT 1,"
    "registration_date INTEGER NOT NULL,"
    "FOREIGN KEY (role_id) REFERENCES roles(role_id)"
    ");";

// 创建楼宇表
static const char *CREATE_BUILDINGS_TABLE =
    "CREATE TABLE IF NOT EXISTS buildings ("
    "building_id TEXT PRIMARY KEY,"
    "building_name TEXT NOT NULL,"
    "address TEXT NOT NULL,"
    "floors_count INTEGER NOT NULL"
    ");";

// 创建房屋表
static const char *CREATE_ROOMS_TABLE =
    "CREATE TABLE IF NOT EXISTS rooms ("
    "room_id TEXT PRIMARY KEY,"
    "building_id TEXT NOT NULL,"
    "room_number TEXT NOT NULL,"
    "floor INTEGER NOT NULL,"
    "area_sqm REAL NOT NULL,"
    "owner_id TEXT,"
    "status INTEGER DEFAULT 0,"
    "FOREIGN KEY (building_id) REFERENCES buildings(building_id),"
    "FOREIGN KEY (owner_id) REFERENCES users(user_id)"
    ");";

// 创建停车位表
static const char *CREATE_PARKING_SPACES_TABLE =
    "CREATE TABLE IF NOT EXISTS parking_spaces ("
    "parking_id TEXT PRIMARY KEY,"
    "parking_number TEXT NOT NULL,"
    "owner_id TEXT,"
    "status INTEGER DEFAULT 0,"
    "FOREIGN KEY (owner_id) REFERENCES users(user_id)"
    ");";

// 创建人员类型表
static const char *CREATE_STAFF_TYPES_TABLE =
    "CREATE TABLE IF NOT EXISTS staff_types ("
    "staff_type_id TEXT PRIMARY KEY,"
    "type_name TEXT NOT NULL,"
    "description TEXT"
    ");";

// 创建物业人员表
static const char *CREATE_STAFF_TABLE =
    "CREATE TABLE IF NOT EXISTS staff ("
    "staff_id TEXT PRIMARY KEY,"
    "user_id TEXT NOT NULL,"
    "staff_type_id TEXT NOT NULL,"
    "hire_date INTEGER NOT NULL,"
    "status INTEGER DEFAULT 1,"
    "FOREIGN KEY (user_id) REFERENCES users(user_id),"
    "FOREIGN KEY (staff_type_id) REFERENCES staff_types(staff_type_id)"
    ");";

// 创建服务区域表
static const char *CREATE_SERVICE_AREAS_TABLE =
    "CREATE TABLE IF NOT EXISTS service_areas ("
    "area_id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "building_id TEXT NOT NULL,"
    "assignment_date INTEGER NOT NULL,"
    "FOREIGN KEY (staff_id) REFERENCES staff(staff_id),"
    "FOREIGN KEY (building_id) REFERENCES buildings(building_id)"
    ");";

// 创建服务记录表
static const char *CREATE_SERVICE_RECORDS_TABLE =
    "CREATE TABLE IF NOT EXISTS service_records ("
    "record_id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "service_type TEXT NOT NULL,"
    "service_date INTEGER NOT NULL,"
    "description TEXT,"
    "status INTEGER DEFAULT 0,"
    "target_id TEXT NOT NULL,"
    "FOREIGN KEY (staff_id) REFERENCES staff(staff_id)"
    ");";

// 创建费用标准表
static const char *CREATE_FEE_STANDARDS_TABLE =
    "CREATE TABLE IF NOT EXISTS fee_standards ("
    "standard_id TEXT PRIMARY KEY,"
    "fee_type INTEGER NOT NULL,"
    "price_per_unit REAL NOT NULL,"
    "unit TEXT NOT NULL,"
    "effective_date INTEGER NOT NULL,"
    "end_date INTEGER DEFAULT 0"
    ");";

// 创建交易表
static const char *CREATE_TRANSACTIONS_TABLE =
    "CREATE TABLE IF NOT EXISTS transactions ("
    "transaction_id TEXT PRIMARY KEY,"
    "user_id TEXT NOT NULL,"
    "room_id TEXT,"
    "parking_id TEXT,"
    "fee_type INTEGER NOT NULL,"
    "amount REAL NOT NULL,"
    "payment_date INTEGER NOT NULL,"
    "due_date INTEGER NOT NULL,"
    "payment_method INTEGER DEFAULT 0,"
    "status INTEGER DEFAULT 0,"
    "period_start INTEGER NOT NULL,"
    "period_end INTEGER NOT NULL,"
    "FOREIGN KEY (user_id) REFERENCES users(user_id),"
    "FOREIGN KEY (room_id) REFERENCES rooms(room_id),"
    "FOREIGN KEY (parking_id) REFERENCES parking_spaces(parking_id)"
    ");";

// 初始化角色数据
static const char *INSERT_ROLES[] = {
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_admin', '管理员', 1);",
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_staff', '物业服务人员', 2);",
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_owner', '业主', 3);",
    NULL // 结束标记
};

// 初始化默认管理员账户
static const char *INSERT_DEFAULT_ADMIN =
    "INSERT OR IGNORE INTO users (user_id, username, password_hash, name, role_id, status, registration_date) "
    "VALUES ('1', 'admin', 'admin123', '系统管理员', 'role_admin', 1, strftime('%s','now'));";

int db_init_tables(Database *db)
{
    int result;

    // 在函数内定义表创建语句的数组
    const char *create_tables[] = {
        CREATE_ROLES_TABLE,
        CREATE_USERS_TABLE,
        CREATE_BUILDINGS_TABLE,
        CREATE_ROOMS_TABLE,
        CREATE_PARKING_SPACES_TABLE,
        CREATE_STAFF_TYPES_TABLE,
        CREATE_STAFF_TABLE,
        CREATE_SERVICE_AREAS_TABLE,
        CREATE_SERVICE_RECORDS_TABLE,
        CREATE_FEE_STANDARDS_TABLE,
        CREATE_TRANSACTIONS_TABLE,
        NULL // 结束标记
    };

    // 创建表
    int i = 0;
    while (create_tables[i] != NULL)
    {
        result = db_execute(db, create_tables[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "创建表失败: %s\n", sqlite3_errmsg(db->db));
            return result;
        }
        i++;
    }

    // 初始化角色数据
    i = 0;
    while (INSERT_ROLES[i] != NULL)
    {
        result = db_execute(db, INSERT_ROLES[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化角色数据失败: %s\n", sqlite3_errmsg(db->db));
            return result;
        }
        i++;
    }

    printf("数据库表初始化完成\n");
    return SQLITE_OK;
}

int db_init_admin(Database *db)
{
    int result = db_execute(db, INSERT_DEFAULT_ADMIN);
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "初始化管理员账户失败: %s\n", sqlite3_errmsg(db->db));
        return result;
    }

    printf("管理员账户初始化完成\n");
    return SQLITE_OK;
}
