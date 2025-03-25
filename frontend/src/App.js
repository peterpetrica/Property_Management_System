// ...existing code...
import AdminDashboard from './pages/AdminDashboard';
import StaffDashboard from './pages/StaffDashboard';
import UserDashboard from './pages/UserDashboard';
// ...existing code...

function App() {
    return (
        <BrowserRouter>
            <Routes>
                {/* ...existing routes... */}
                <Route path="/admin/dashboard" element={<AdminDashboard />} />
                <Route path="/staff/dashboard" element={<StaffDashboard />} />
                <Route path="/user/dashboard" element={<UserDashboard />} />
            </Routes>
        </BrowserRouter>
    );
}
// ...existing code...
