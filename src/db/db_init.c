#include "db/database.h"
#include "db/db_init.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 创建管理员表
static const char *CREATE_ADMIN_TABLE =
    "CREATE TABLE IF NOT EXISTS admins ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "password_hash TEXT NOT NULL,"
    "weight INTEGER DEFAULT 1"
    ");";

// 创建物业服务人员表
static const char *CREATE_STAFF_TABLE =
    "CREATE TABLE IF NOT EXISTS staff ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "phone TEXT,"
    "password_hash TEXT NOT NULL,"
    "service_type TEXT,"
    "weight INTEGER DEFAULT 2"
    ");";

// 创建业主表
static const char *CREATE_OWNER_TABLE =
    "CREATE TABLE IF NOT EXISTS owners ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "phone TEXT,"
    "password_hash TEXT NOT NULL,"
    "notification_required INTEGER DEFAULT 1,"
    "weight INTEGER DEFAULT 3"
    ");";

// 创建小区表
static const char *CREATE_COMMUNITY_TABLE =
    "CREATE TABLE IF NOT EXISTS communities ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL"
    ");";

// 创建楼宇表
static const char *CREATE_BUILDING_TABLE =
    "CREATE TABLE IF NOT EXISTS buildings ("
    "id TEXT PRIMARY KEY,"
    "building_number TEXT NOT NULL,"
    "community_id TEXT NOT NULL,"
    "floors INTEGER DEFAULT 0,"
    "units_per_floor INTEGER DEFAULT 0,"
    "FOREIGN KEY (community_id) REFERENCES communities(id)"
    ");";

// 创建房屋表
static const char *CREATE_APARTMENT_TABLE =
    "CREATE TABLE IF NOT EXISTS apartments ("
    "id TEXT PRIMARY KEY,"
    "building_id TEXT NOT NULL,"
    "floor INTEGER NOT NULL,"
    "unit INTEGER NOT NULL,"
    "area REAL NOT NULL,"
    "occupied INTEGER DEFAULT 0,"
    "property_fee_balance REAL DEFAULT 0.0,"
    "fee_paid INTEGER DEFAULT 0,"
    "owner_id TEXT,"
    "FOREIGN KEY (building_id) REFERENCES buildings(id),"
    "FOREIGN KEY (owner_id) REFERENCES owners(id)"
    ");";

// 创建停车位表
static const char *CREATE_PARKING_TABLE =
    "CREATE TABLE IF NOT EXISTS parking_spaces ("
    "id TEXT PRIMARY KEY,"
    "space_number TEXT NOT NULL,"
    "community_id TEXT NOT NULL,"
    "occupied INTEGER DEFAULT 0,"
    "owner_id TEXT,"
    "FOREIGN KEY (community_id) REFERENCES communities(id),"
    "FOREIGN KEY (owner_id) REFERENCES owners(id)"
    ");";

// 创建交易表
static const char *CREATE_TRANSACTION_TABLE =
    "CREATE TABLE IF NOT EXISTS transactions ("
    "id TEXT PRIMARY KEY,"
    "owner_id TEXT NOT NULL,"
    "apartment_id TEXT,"
    "type INTEGER NOT NULL,"
    "amount REAL NOT NULL,"
    "payment_date INTEGER NOT NULL,"
    "description TEXT,"
    "FOREIGN KEY (owner_id) REFERENCES owners(id),"
    "FOREIGN KEY (apartment_id) REFERENCES apartments(id)"
    ");";

// 创建费用标准表
static const char *CREATE_FEE_STANDARD_TABLE =
    "CREATE TABLE IF NOT EXISTS fee_standards ("
    "id TEXT PRIMARY KEY,"
    "type INTEGER NOT NULL,"
    "rate REAL NOT NULL,"
    "valid_from INTEGER NOT NULL,"
    "valid_to INTEGER DEFAULT 0"
    ");";

// 创建服务类型表
static const char *CREATE_SERVICE_TYPE_TABLE =
    "CREATE TABLE IF NOT EXISTS service_types ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "description TEXT"
    ");";

// 创建服务记录表
static const char *CREATE_SERVICE_RECORD_TABLE =
    "CREATE TABLE IF NOT EXISTS service_records ("
    "id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "building_id TEXT,"
    "apartment_id TEXT,"
    "service_type_id TEXT NOT NULL,"
    "service_time INTEGER NOT NULL,"
    "description TEXT,"
    "FOREIGN KEY (staff_id) REFERENCES staff(id),"
    "FOREIGN KEY (building_id) REFERENCES buildings(id),"
    "FOREIGN KEY (apartment_id) REFERENCES apartments(id),"
    "FOREIGN KEY (service_type_id) REFERENCES service_types(id)"
    ");";

// 创建服务人员分配表
static const char *CREATE_STAFF_ASSIGNMENT_TABLE =
    "CREATE TABLE IF NOT EXISTS staff_assignments ("
    "id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "building_id TEXT NOT NULL,"
    "assign_time INTEGER NOT NULL,"
    "end_time INTEGER DEFAULT 0,"
    "FOREIGN KEY (staff_id) REFERENCES staff(id),"
    "FOREIGN KEY (building_id) REFERENCES buildings(id)"
    ");";

// 创建令牌表
static const char *CREATE_TOKEN_TABLE =
    "CREATE TABLE IF NOT EXISTS tokens ("
    "token TEXT PRIMARY KEY,"
    "user_id TEXT NOT NULL,"
    "user_type INTEGER NOT NULL,"
    "expire_time INTEGER NOT NULL"
    ");";

// 定义所有表创建SQL语句的数组
static const char *CREATE_TABLES[] = {
    "CREATE TABLE IF NOT EXISTS admins ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "password_hash TEXT NOT NULL,"
    "weight INTEGER DEFAULT 1"
    ");",

    "CREATE TABLE IF NOT EXISTS staff ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "phone TEXT,"
    "password_hash TEXT NOT NULL,"
    "service_type TEXT,"
    "weight INTEGER DEFAULT 2"
    ");",

    "CREATE TABLE IF NOT EXISTS owners ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "phone TEXT,"
    "password_hash TEXT NOT NULL,"
    "notification_required INTEGER DEFAULT 1,"
    "weight INTEGER DEFAULT 3"
    ");",

    "CREATE TABLE IF NOT EXISTS communities ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL"
    ");",

    "CREATE TABLE IF NOT EXISTS buildings ("
    "id TEXT PRIMARY KEY,"
    "building_number TEXT NOT NULL,"
    "community_id TEXT NOT NULL,"
    "floors INTEGER DEFAULT 0,"
    "units_per_floor INTEGER DEFAULT 0,"
    "FOREIGN KEY (community_id) REFERENCES communities(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS apartments ("
    "id TEXT PRIMARY KEY,"
    "building_id TEXT NOT NULL,"
    "floor INTEGER NOT NULL,"
    "unit INTEGER NOT NULL,"
    "area REAL NOT NULL,"
    "occupied INTEGER DEFAULT 0,"
    "property_fee_balance REAL DEFAULT 0.0,"
    "fee_paid INTEGER DEFAULT 0,"
    "owner_id TEXT,"
    "FOREIGN KEY (building_id) REFERENCES buildings(id),"
    "FOREIGN KEY (owner_id) REFERENCES owners(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS parking_spaces ("
    "id TEXT PRIMARY KEY,"
    "space_number TEXT NOT NULL,"
    "community_id TEXT NOT NULL,"
    "occupied INTEGER DEFAULT 0,"
    "owner_id TEXT,"
    "FOREIGN KEY (community_id) REFERENCES communities(id),"
    "FOREIGN KEY (owner_id) REFERENCES owners(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS transactions ("
    "id TEXT PRIMARY KEY,"
    "owner_id TEXT NOT NULL,"
    "apartment_id TEXT,"
    "type INTEGER NOT NULL,"
    "amount REAL NOT NULL,"
    "payment_date INTEGER NOT NULL,"
    "description TEXT,"
    "FOREIGN KEY (owner_id) REFERENCES owners(id),"
    "FOREIGN KEY (apartment_id) REFERENCES apartments(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS fee_standards ("
    "id TEXT PRIMARY KEY,"
    "type INTEGER NOT NULL,"
    "rate REAL NOT NULL,"
    "valid_from INTEGER NOT NULL,"
    "valid_to INTEGER DEFAULT 0"
    ");",

    "CREATE TABLE IF NOT EXISTS service_types ("
    "id TEXT PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "description TEXT"
    ");",

    "CREATE TABLE IF NOT EXISTS service_records ("
    "id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "building_id TEXT,"
    "apartment_id TEXT,"
    "service_type_id TEXT NOT NULL,"
    "service_time INTEGER NOT NULL,"
    "description TEXT,"
    "FOREIGN KEY (staff_id) REFERENCES staff(id),"
    "FOREIGN KEY (building_id) REFERENCES buildings(id),"
    "FOREIGN KEY (apartment_id) REFERENCES apartments(id),"
    "FOREIGN KEY (service_type_id) REFERENCES service_types(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS staff_assignments ("
    "id TEXT PRIMARY KEY,"
    "staff_id TEXT NOT NULL,"
    "building_id TEXT NOT NULL,"
    "assign_time INTEGER NOT NULL,"
    "end_time INTEGER DEFAULT 0,"
    "FOREIGN KEY (staff_id) REFERENCES staff(id),"
    "FOREIGN KEY (building_id) REFERENCES buildings(id)"
    ");",

    "CREATE TABLE IF NOT EXISTS tokens ("
    "token TEXT PRIMARY KEY,"
    "user_id TEXT NOT NULL,"
    "user_type INTEGER NOT NULL,"
    "expire_time INTEGER NOT NULL"
    ");",

    NULL // 结束标记
};

// 初始化默认管理员账户
static const char *INSERT_DEFAULT_ADMIN =
    "INSERT OR IGNORE INTO admins (id, name, password_hash, weight) "
    "VALUES ('admin_001', 'admin', 'admin123', 1);";
// **仅作示例**

int db_init_tables(Database *db) // 修改函数名，避免与 database.c 中的 db_init 冲突
{
    // TODO: 实现数据库表初始化功能
    return SQLITE_OK;
}

int db_init_admin(Database *db)
{
    // TODO: 实现默认管理员账户初始化功能
    return SQLITE_OK;
}
