#include "models/user.h"
#include "utils/utils.h"
#include "auth/tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建业主账户
bool create_owner(Database *db, Owner *owner, const char *password)
{
    if (!db || !owner || !password)
        return false;

    // 哈希密码
    char *hashed_password = hash_password(password);
    if (!hashed_password)
        return false;

    // 构造SQL语句
    const char *sql = "INSERT INTO owners (id, name, phone, notification_required, weight) "
                      "VALUES (?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, owner->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, owner->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, owner->phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, owner->notification_required ? 1 : 0);
    sqlite3_bind_int(stmt, 5, owner->weight);

    // 执行SQL
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        free(hashed_password);
        return false;
    }

    // 存储密码哈希
    const char *update_sql = "UPDATE owners SET password_hash = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    sqlite3_bind_text(stmt, 1, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, owner->id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    free(hashed_password);

    return result;
}

// 更新业主信息
bool update_owner(Database *db, const char *token, Owner *owner)
{
    if (!db || !token || !owner)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 只有管理员或者业主本人才能更新业主信息
    char user_id[32] = {0};
    if (!get_user_id_from_token(db, token, user_id, sizeof(user_id)))
    {
        return false;
    }

    if (user_type != USER_ADMIN && strcmp(user_id, owner->id) != 0)
    {
        return false;
    }

    // 构造SQL语句，更新除密码外的信息
    const char *sql = "UPDATE owners SET name = ?, phone = ?, notification_required = ? WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, owner->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, owner->phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, owner->notification_required ? 1 : 0);
    sqlite3_bind_text(stmt, 4, owner->id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取业主信息
bool get_owner_by_id(Database *db, const char *owner_id, Owner *owner)
{
    if (!db || !owner_id || !owner)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT id, name, phone, building_id, apartment_id, notification_required, weight "
                      "FROM owners WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, owner_id, -1, SQLITE_STATIC);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);
        const char *building_id = (const char *)sqlite3_column_text(stmt, 3);
        const char *apartment_id = (const char *)sqlite3_column_text(stmt, 4);
        int notification_required = sqlite3_column_int(stmt, 5);
        int weight = sqlite3_column_int(stmt, 6);

        safe_strcpy(owner->id, id ? id : "", sizeof(owner->id));
        safe_strcpy(owner->name, name ? name : "", sizeof(owner->name));
        safe_strcpy(owner->phone, phone ? phone : "", sizeof(owner->phone));
        safe_strcpy(owner->building_id, building_id ? building_id : "", sizeof(owner->building_id));
        safe_strcpy(owner->apartment_id, apartment_id ? apartment_id : "", sizeof(owner->apartment_id));
        owner->notification_required = notification_required ? true : false;
        owner->weight = weight;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 创建服务人员账户
bool create_staff(Database *db, Staff *staff, const char *password)
{
    if (!db || !staff || !password)
        return false;

    // 哈希密码
    char *hashed_password = hash_password(password);
    if (!hashed_password)
        return false;

    // 构造SQL语句
    const char *sql = "INSERT INTO staff (id, name, phone, service_type, weight) "
                      "VALUES (?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, staff->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, staff->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, staff->service_type, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, staff->weight);

    // 执行SQL
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        free(hashed_password);
        return false;
    }

    // 存储密码哈希
    const char *update_sql = "UPDATE staff SET password_hash = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    sqlite3_bind_text(stmt, 1, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff->id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    free(hashed_password);

    return result;
}

// 更新服务人员信息
bool update_staff(Database *db, const char *token, Staff *staff)
{
    if (!db || !token || !staff)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 只有管理员或者服务人员本人才能更新服务人员信息
    char user_id[32] = {0};
    if (!get_user_id_from_token(db, token, user_id, sizeof(user_id)))
    {
        return false;
    }

    if (user_type != USER_ADMIN && strcmp(user_id, staff->id) != 0)
    {
        return false;
    }

    // 构造SQL语句，更新除密码外的信息
    const char *sql = "UPDATE staff SET name = ?, phone = ?, service_type = ? WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, staff->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, staff->service_type, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, staff->id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取服务人员信息
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff)
{
    if (!db || !staff_id || !staff)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT id, name, phone, service_type, weight "
                      "FROM staff WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);
        const char *service_type = (const char *)sqlite3_column_text(stmt, 3);
        int weight = sqlite3_column_int(stmt, 4);

        safe_strcpy(staff->id, id ? id : "", sizeof(staff->id));
        safe_strcpy(staff->name, name ? name : "", sizeof(staff->name));
        safe_strcpy(staff->phone, phone ? phone : "", sizeof(staff->phone));
        safe_strcpy(staff->service_type, service_type ? service_type : "", sizeof(staff->service_type));
        staff->weight = weight;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 创建管理员账户
bool create_admin(Database *db, Admin *admin, const char *password)
{
    if (!db || !admin || !password)
        return false;

    // 验证是否有权限创建管理员（需要现有管理员权限）
    // 这里不检查，因为在UI层已经做了权限检查

    // 哈希密码
    char *hashed_password = hash_password(password);
    if (!hashed_password)
        return false;

    // 构造SQL语句
    const char *sql = "INSERT INTO admins (id, name, weight) VALUES (?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, admin->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, admin->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, admin->weight);

    // 执行SQL
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        free(hashed_password);
        return false;
    }

    // 存储密码哈希
    const char *update_sql = "UPDATE admins SET password_hash = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(hashed_password);
        return false;
    }

    sqlite3_bind_text(stmt, 1, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, admin->id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    free(hashed_password);

    return result;
}

// 更新管理员信息
bool update_admin(Database *db, const char *token, Admin *admin)
{
    if (!db || !token || !admin)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 只有管理员才能更新管理员信息

    // 构造SQL语句，更新除密码外的信息
    const char *sql = "UPDATE admins SET name = ? WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, admin->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, admin->id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取管理员信息
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin)
{
    if (!db || !admin_id || !admin)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT id, name, weight FROM admins WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, admin_id, -1, SQLITE_STATIC);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int weight = sqlite3_column_int(stmt, 2);

        safe_strcpy(admin->id, id ? id : "", sizeof(admin->id));
        safe_strcpy(admin->name, name ? name : "", sizeof(admin->name));
        admin->weight = weight;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 删除用户 (仅管理员可用)
bool delete_user(Database *db, const char *admin_token, const char *user_id, UserType user_type)
{
    if (!db || !admin_token || !user_id)
        return false;

    // 验证管理员令牌
    int admin_weight;
    UserType admin_type;
    if (!validate_token(db, admin_token, &admin_weight, &admin_type) || admin_type != USER_ADMIN)
    {
        return false;
    }

    // 确定用户表
    const char *table_name;
    switch (user_type)
    {
    case USER_STAFF:
        table_name = "staff";
        break;
    case USER_OWNER:
        table_name = "owners";
        break;
    default:
        return false; // 不允许删除管理员
    }

    // 构造删除SQL
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE id = ?;", table_name);

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    // 执行删除
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    // 清理
    sqlite3_finalize(stmt);

    return result;
}
