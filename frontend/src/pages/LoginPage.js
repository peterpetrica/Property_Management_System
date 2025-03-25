// ...existing code...
const handleLogin = async () => {
    try {
        const response = await fetch('/api/auth/login', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password }),
        });

        const data = await response.json();
        if (response.ok) {
            const { role } = data;

            // 根据角色跳转到对应页面
            if (role === 'admin') {
                window.location.href = '/admin/dashboard';
            } else if (role === 'staff') {
                window.location.href = '/staff/dashboard';
            } else if (role === 'user') {
                window.location.href = '/user/dashboard';
            }
        } else {
            alert(data.message);
        }
    } catch (error) {
        console.error('Login error:', error);
        alert('An error occurred during login.');
    }
};
// ...existing code...
