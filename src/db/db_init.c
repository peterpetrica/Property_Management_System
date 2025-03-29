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
 * - db_init_staff: 初始化默认物业服务人员账户
 */

#include "db/database.h"
#include "utils/utils.h"
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

// 创建用户表 - 修改了user_id为TEXT类型
static const char *CREATE_USERS_TABLE =
    "CREATE TABLE IF NOT EXISTS users ("
    "user_id TEXT PRIMARY KEY,"           // 用户ID
    "username TEXT NOT NULL UNIQUE,"      // 用户名
    "password_hash TEXT NOT NULL,"        // 密码哈希
    "name TEXT NOT NULL,"                 // 真实姓名
    "phone_number TEXT,"                  // 电话号码
    "email TEXT,"                         // 电子邮件
    "role_id TEXT NOT NULL,"              // 角色ID
    "status INTEGER DEFAULT 1,"           // 状态
    "registration_date INTEGER NOT NULL," // 注册时间
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

// 初始化角色数据 - 使用静态标识符而非UUID
static const char *INSERT_ROLES[] = {
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_admin', '管理员', 1);",
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_staff', '物业服务人员', 2);",
    "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES ('role_owner', '业主', 3);",
    NULL // 结束标记
};

// 初始化默认费用标准
static const char *INSERT_DEFAULT_FEE_STANDARDS[] = {
    // 物业费标准（按房屋类型）
    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('PF01', 1, 3.5, '元/㎡/月', strftime('%s','2024-01-01'));", // 普通住宅物业费

    // 停车费标准（按车位类型）
    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('CF01', 2, 300.0, '元/月', strftime('%s','2024-01-01'));", // 地上停车费

    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('CF02', 2, 400.0, '元/月', strftime('%s','2024-01-01'));", // 地下停车费

    // 水费标准
    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('WF01', 3, 4.9, '元/m³', strftime('%s','2024-01-01'));", // 水费

    // 电费标准
    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('EF01', 4, 0.98, '元/kWh', strftime('%s','2024-01-01'));", // 电费

    // 燃气费标准
    "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) "
    "VALUES ('GF01', 5, 3.2, '元/m³', strftime('%s','2024-01-01'));", // 燃气费

    NULL};

// 初始化默认楼栋数据
static const char *INSERT_DEFAULT_BUILDINGS[] = {
    // 添加测试楼栋A1
    "INSERT OR IGNORE INTO buildings (building_id, building_name, address, floors_count) "
    "VALUES ('B001', 'A1', '小区1号院', 33)",

    // 添加测试楼栋A2
    "INSERT OR IGNORE INTO buildings (building_id, building_name, address, floors_count) "
    "VALUES ('B002', 'A2', '小区1号院', 33)",

    NULL};

/**
 * 使用UUID初始化默认管理员账户
 * 注意：用户ID使用UUID，但角色ID使用静态标识符'role_admin'
 */
int db_init_admin(Database *db)
{
    char admin_uuid[37];
    generate_uuid(admin_uuid);

    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT OR IGNORE INTO users (user_id, username, password_hash, name, role_id, status, registration_date) "
             "VALUES ('%s', 'admin', 'admin123', '系统管理员', 'role_admin', 1, strftime('%%s','now'));",
             admin_uuid);

    int result = db_execute(db, sql);
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "初始化管理员账户失败: %s\n", sqlite3_errmsg(db->db));
        return result;
    }

    printf("管理员账户初始化完成\n");
    return SQLITE_OK;
}

/**
 * 使用UUID初始化默认物业服务人员账户
 */
int db_init_staff(Database *db)
{
    int result;
    char staff_type_uuid[37];
    char staff_user_uuid[37];
    char staff_uuid[37];

    // 生成UUID
    generate_uuid(staff_type_uuid);
    generate_uuid(staff_user_uuid);
    generate_uuid(staff_uuid);

    // 构建SQL语句
    char sql[512];

    // 插入默认物业人员类型
    snprintf(sql, sizeof(sql),
             "INSERT OR IGNORE INTO staff_types (staff_type_id, type_name, description) "
             "VALUES ('%s', '普通物业人员', '默认物业服务人员类型');",
             staff_type_uuid);

    result = db_execute(db, sql);
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "初始化物业人员类型失败: %s\n", sqlite3_errmsg(db->db));
        return result;
    }

    // 检查是否已存在staff用户
    snprintf(sql, sizeof(sql), "SELECT user_id FROM users WHERE username = 'staff' LIMIT 1;");
    sqlite3_stmt *stmt;
    result = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);

    if (result == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 用户已存在，使用现有用户ID
        const char *existing_id = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(staff_user_uuid, existing_id);
        printf("使用已存在的物业人员用户ID: %s\n", staff_user_uuid);
        sqlite3_finalize(stmt);
    }
    else
    {
        // 用户不存在，需要创建
        sqlite3_finalize(stmt);

        // 插入默认物业人员用户
        snprintf(sql, sizeof(sql),
                 "INSERT INTO users (user_id, username, password_hash, name, role_id, status, registration_date) "
                 "VALUES ('%s', 'staff', 'staff123', '物业服务员', 'role_staff', 1, strftime('%%s','now'));",
                 staff_user_uuid);

        result = db_execute(db, sql);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化物业人员用户失败: %s\n", sqlite3_errmsg(db->db));
            return result;
        }
    }

    // 检查staff关联记录是否已存在
    snprintf(sql, sizeof(sql), "SELECT staff_id FROM staff WHERE user_id = '%s' LIMIT 1;", staff_user_uuid);
    result = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);

    if (result == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
    {
        // staff记录已存在，无需再创建
        sqlite3_finalize(stmt);
        printf("物业服务人员记录已存在\n");
    }
    else
    {
        sqlite3_finalize(stmt);

        // 插入默认物业人员记录
        snprintf(sql, sizeof(sql),
                 "INSERT INTO staff (staff_id, user_id, staff_type_id, hire_date, status) "
                 "VALUES ('%s', '%s', '%s', strftime('%%s','now'), 1);",
                 staff_uuid, staff_user_uuid, staff_type_uuid);

        result = db_execute(db, sql);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化物业人员记录失败: %s\n", sqlite3_errmsg(db->db));
            return result;
        }
    }

    printf("物业服务人员账户初始化完成\n");
    return SQLITE_OK;
}

/**
 * 初始化默认用户和房间数据
 */
int db_init_default_data(Database *db)
{
    int result;
    char sql[512];

    // 用户名数组
    const char *usernames[] = {"zhangsan", "lisi", "wangwu"};
    const char *names[] = {"张三", "李四", "王五"};
    const char *phones[] = {"13800138001", "13800138002", "13800138003"};
    const char *emails[] = {"test1@example.com", "test2@example.com", "test3@example.com"};

    // 用户UUID数组
    char user_uuids[3][37];

    // 检查用户是否已存在，并获取其ID
    for (int i = 0; i < 3; i++)
    {
        snprintf(sql, sizeof(sql), "SELECT user_id FROM users WHERE username = '%s' LIMIT 1;", usernames[i]);
        sqlite3_stmt *stmt;
        result = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);

        if (result == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
        {
            // 用户已存在，使用现有ID
            const char *existing_id = (const char *)sqlite3_column_text(stmt, 0);
            strcpy(user_uuids[i], existing_id);
            printf("使用已存在的用户ID: %s 对应用户: %s\n", user_uuids[i], usernames[i]);
            sqlite3_finalize(stmt);
        }
        else
        {
            // 用户不存在，生成新ID并创建用户
            sqlite3_finalize(stmt);
            generate_uuid(user_uuids[i]);

            snprintf(sql, sizeof(sql),
                     "INSERT INTO users ("
                     "user_id, username, password_hash, name, phone_number, email, role_id, "
                     "status, registration_date"
                     ") VALUES ("
                     "'%s', '%s', 'test123', '%s', '%s', '%s', "
                     "'role_owner', 1, strftime('%%s','now'))",
                     user_uuids[i], usernames[i], names[i], phones[i], emails[i]);

            result = db_execute(db, sql);
            if (result != SQLITE_OK)
            {
                fprintf(stderr, "初始化用户数据失败: %s\n", sqlite3_errmsg(db->db));
                return result;
            }
        }
    }

    // 房间号数组
    const char *room_numbers[] = {"A101", "A102", "A103"};
    const double areas[] = {89.5, 126.5, 89.5};

    // 检查房间是否已存在
    for (int i = 0; i < 3; i++)
    {
        snprintf(sql, sizeof(sql),
                 "SELECT room_id FROM rooms WHERE building_id = 'B001' AND room_number = '%s' LIMIT 1;",
                 room_numbers[i]);
        sqlite3_stmt *stmt;
        result = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);

        if (result == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
        {
            // 房间已存在
            printf("房间 %s 已存在\n", room_numbers[i]);
            sqlite3_finalize(stmt);
        }
        else
        {
            // 房间不存在，创建新房间
            sqlite3_finalize(stmt);
            char room_uuid[37];
            generate_uuid(room_uuid);

            snprintf(sql, sizeof(sql),
                     "INSERT INTO rooms ("
                     "room_id, building_id, room_number, floor, area_sqm, owner_id, status"
                     ") VALUES ("
                     "'%s', 'B001', '%s', 1, %.1f, '%s', 1)",
                     room_uuid, room_numbers[i], areas[i], user_uuids[i]);

            result = db_execute(db, sql);
            if (result != SQLITE_OK)
            {
                fprintf(stderr, "初始化房间数据失败: %s\n", sqlite3_errmsg(db->db));
                return result;
            }
        }
    }

    printf("默认用户和房间数据初始化完成\n");
    return SQLITE_OK;
}

/**
 * db_init_tables
 *
 * 创建物业管理系统所需的所有数据库表并初始化角色数据
 *
 * @param db 数据库连接指针
 * @return SQLITE_OK 表示成功，其他值表示失败
 */
int db_init_tables(Database *db)
{
    int result;

    // 开始事务
    result = db_execute(db, "BEGIN TRANSACTION;");
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "无法开始事务: %s\n", sqlite3_errmsg(db->db));
        return result;
    }

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
        NULL};

    int i = 0;
    while (create_tables[i] != NULL)
    {
        result = db_execute(db, create_tables[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "创建表失败: %s\n", sqlite3_errmsg(db->db));
            db_execute(db, "ROLLBACK;");
            return result;
        }
        i++;
    }

    i = 0;
    while (INSERT_ROLES[i] != NULL)
    {
        result = db_execute(db, INSERT_ROLES[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化角色数据失败: %s\n", sqlite3_errmsg(db->db));
            db_execute(db, "ROLLBACK;");
            return result;
        }
        i++;
    }

    // 初始化费用标准
    i = 0;
    while (INSERT_DEFAULT_FEE_STANDARDS[i] != NULL)
    {
        result = db_execute(db, INSERT_DEFAULT_FEE_STANDARDS[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化费用标准失败: %s\n", sqlite3_errmsg(db->db));
            db_execute(db, "ROLLBACK;");
            return result;
        }
        i++;
    }

    // 初始化楼栋数据
    i = 0;
    while (INSERT_DEFAULT_BUILDINGS[i] != NULL)
    {
        result = db_execute(db, INSERT_DEFAULT_BUILDINGS[i]);
        if (result != SQLITE_OK)
        {
            fprintf(stderr, "初始化楼栋数据失败: %s\n", sqlite3_errmsg(db->db));
            db_execute(db, "ROLLBACK;");
            return result;
        }
        i++;
    }

    // 初始化管理员账户
    result = db_init_admin(db);
    if (result != SQLITE_OK)
    {
        db_execute(db, "ROLLBACK;");
        return result;
    }

    // 初始化物业服务人员账户
    result = db_init_staff(db);
    if (result != SQLITE_OK)
    {
        db_execute(db, "ROLLBACK;");
        return result;
    }

    // 初始化默认用户和房间数据
    result = db_init_default_data(db);
    if (result != SQLITE_OK)
    {
        db_execute(db, "ROLLBACK;");
        return result;
    }

    // 提交事务
    result = db_execute(db, "COMMIT;");
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "无法提交事务: %s\n", sqlite3_errmsg(db->db));
        db_execute(db, "ROLLBACK;");
        return result;
    }

    printf("数据库初始化完成\n");
    return SQLITE_OK;
}
