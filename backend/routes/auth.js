// ...existing code...
router.post('/login', async (req, res) => {
    const { username, password } = req.body;

    try {
        // 验证用户名和密码
        const user = await User.findOne({ username });
        if (!user || user.password !== password) {
            return res.status(401).json({ message: 'Invalid credentials' });
        }

        // 根据用户名获取角色
        const role = user.role; // 假设数据库中有 role 字段，值为 'admin'、'staff' 或 'user'

        // 返回角色信息
        return res.status(200).json({ message: 'Login successful', role });
    } catch (error) {
        console.error(error);
        return res.status(500).json({ message: 'Server error' });
    }
});
// ...existing code...
