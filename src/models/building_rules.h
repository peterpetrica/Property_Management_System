#define MAX_BUILDINGS 10        // 最大楼栋数
#define MAX_FLOORS 33          // 每栋最大楼层
#define MAX_ROOMS_PER_FLOOR 8  // 每层最大房间数

// 楼号规则: A1-A10 
const char* BUILDING_PREFIX = "A";

// 房号规则: 0102 表示1层02室
typedef struct {
    int floor;      // 1-33
    int room;       // 1-8  
} RoomNumber;
