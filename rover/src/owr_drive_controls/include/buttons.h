//joystick array indexes
#define BUTTON_A       0
#define BUTTON_B       1
#define BUTTON_X       2 
#define BUTTON_Y       3
#define BUTTON_LB      4
#define BUTTON_RB      5
#define BUTTON_BACK    6
#define BUTTON_START   7
#define BUTTON_POWER   8
#define BUTTON_STICK_L 9
#define BUTTON_STICK_R 10

//LR = Left Right, UD = Up Down
#define STICK_L_LR     0
#define STICK_L_UD     1
#define STICK_LT       2
#define STICK_R_LR     3
#define STICK_R_UD     4
#define STICK_RT       5

#define DPAD_LEFT	   6 // These aren't mapped yet it used to be 2 array indexes (L&R - U&D)
#define DPAD_RIGHT     7
#define DPAD_UP 	   8
#define DPAD_DOWN      9


//and then the layout
//right stick for navigation
#define DRIVE_AXES_UD   STICK_R_UD
#define DRIVE_AXES_LR   STICK_R_LR

//a,b,x,y buttons for Camera feeds
#define CAM_FEED_0   BUTTON_A
#define CAM_FEED_1   BUTTON_B
#define CAM_FEED_2   BUTTON_X
#define CAM_FEED_3   BUTTON_Y
