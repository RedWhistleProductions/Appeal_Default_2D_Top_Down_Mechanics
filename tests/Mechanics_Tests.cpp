#include "../src/Mechanics.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void Check(bool Condition, const char *Message)
{
    if(!Condition) throw std::runtime_error(Message);
}
double Number(const std::string &Entity, const std::string &Key)
{
    return std::stod(Get(Entity, Key));
}
void Near(double Actual, double Expected)
{
    if(std::abs(Actual - Expected) > 0.00001)
        throw std::runtime_error("Expected " + std::to_string(Expected) + ", got " + std::to_string(Actual));
}

int main()
{
    const std::string Forward = MECHANICS_3D ? "Velocity_Z" : "Velocity_Y";
    const std::string Lift = MECHANICS_3D ? "Velocity_Y" : "Velocity_Z";
    const std::string Height = MECHANICS_3D ? "Position_Y" : "Position_Z";
    Constructor(); Walk(); Check(Get("", "Speed").empty(), "Created unselected entity");
    Enter("Player"); Set("Player", "Speed", "10");
    Walk(); Near(Number("Player", Forward), 10);
    Run(); Near(Number("Player", Forward), 20);
    Duck_Walk(); Near(Number("Player", Forward), 5);
    Duck_Stand(); Near(Number("Player", Forward), 0);
    Near(Number("Player", "Duck_Amount"), 1);
    Stand(); Near(Number("Player", "Duck_Amount"), 0);
    for(int Value : {-100, -50, -1, 0, 1, 50, 100})
    {
        Walk_Analog(Value); Near(Number("Player", Forward), 10 * Value / 100.0);
        Run_Analog(Value); Near(Number("Player", Forward), 20 * Value / 100.0);
        Duck_Walk_Analog(Value); Near(Number("Player", Forward), 5 * Value / 100.0);
        Duck_Stand_Analog(Value); Near(Number("Player", Forward), 0);
        Stand_Analog(Value); Near(Number("Player", Forward), 0);
    }
    Walk(); Walk_Analog(101); Walk_Analog(-101); Near(Number("Player", Forward), 10);
    Analog("Player", "Unknown", 20); Near(Number("Player", Forward), 10);
    Near(Number("Player", "Speed"), 10);
    Move_Right();
    Near(std::hypot(Number("Player", "Velocity_X"), Number("Player", Forward)), 10);
    Near(Number("Player", "Velocity_X"), 10 / std::sqrt(2));
    Move_Right_Analog(0); Near(Number("Player", "Velocity_X"), 0);
    Stop(); Move_Left(); Near(Number("Player", "Velocity_X"), -10);
    Move_Left_Analog(-50); Near(Number("Player", "Velocity_X"), 5);
    Move_Left_Analog(0); Move_Right_Analog(1); Near(Number("Player", "Velocity_X"), 0.1);
    Stop_Analog(0); Near(Number("Player", "Velocity_X"), 0);
#if MECHANICS_3D
    Move_Forward(); Near(Number("Player", Forward), 10);
    Move_Backward(); Near(Number("Player", Forward), -10);
    Move_Backward_Analog(-50); Near(Number("Player", Forward), 5);
    Move_Forward_Analog(0); Strafe_Right(); Near(Number("Player", "Velocity_X"), 10);
    Strafe_Left(); Near(Number("Player", "Velocity_X"), -10);
    Strafe_Left_Analog(-50); Near(Number("Player", "Velocity_X"), 5);
    Strafe_Right_Analog(0); Near(Number("Player", "Velocity_X"), 0);
    Set("Player", "Delta_Time", "1"); Set("Player", "Turn_Speed", "90");
    Turn_Right(); Near(Number("Player", "Yaw"), 90);
    Walk(); Near(Number("Player", "Velocity_X"), 10); Near(Number("Player", Forward), 0);
    Turn_Left(); Near(Number("Player", "Yaw"), 0);
    Turn_Left_Analog(50); Near(Number("Player", "Yaw"), -45);
    Turn_Right_Analog(50); Near(Number("Player", "Yaw"), 0);
    Look_Up(); Near(Number("Player", "Pitch"), 89);
    Look_Down(); Near(Number("Player", "Pitch"), -1);
    Look_Down_Analog(-100); Near(Number("Player", "Pitch"), 89);
    Look_Up_Analog(-100); Near(Number("Player", "Pitch"), -1);
#else
    Move_Up(); Near(Number("Player", Forward), 10);
    Move_Down(); Near(Number("Player", Forward), -10);
    Move_Down_Analog(-50); Near(Number("Player", Forward), 5);
    Near(Number("Player", "Facing_Y"), 1);
    Move_Up_Analog(0); Near(Number("Player", Forward), 0);
    Near(Number("Player", "Facing_Y"), 1); // Facing survives stop for idle animation.
#endif
    Stand(); Set("Player", "Delta_Time", "0.1");
    Jump_Analog(0); Check(Get("Player", "Grounded") == "1", "Zero jumped");
    Jump_Analog(50); Near(Number("Player", Lift), 3);
    Jump(); Near(Number("Player", Lift), 3); // Airborne jump is ignored.
    Update("Player"); Check(Number("Player", Height) > 0, "Wrong vertical axis");
    for(int Step = 0; Step < 30; ++Step) Update("Player");
    Check(Get("Player", "Grounded") == "1", "Failed to land");
    Near(Number("Player", Height), 0);
    Jump(); Near(Number("Player", Lift), 6);
    Set("Player", "Grounded", "1"); Jump_Analog(-100); Near(Number("Player", Lift), 6);
    Set("Player", "Grounded", "1"); Set("Player", "Ground_Height", "2");
    Update("Player"); Near(Number("Player", Height), 2);
    Set("Player", "Speed", "nan"); Set("Player", "Speed", "-1");
    Set("Player", "Speed", "bad"); Near(Number("Player", "Speed"), 10);
    Enter("NPC"); Set("NPC", "Speed", "2"); Walk();
    Digital("Player", "Run"); Stand();
    Near(Number("NPC", Forward), 0); Near(Number("Player", Forward), 20);
    Analog("Player", "Walk", -50); Near(Number("Player", Forward), -5);
    Exit("Player"); Near(Number("Player", Forward), 0);
    Enter("Player"); Near(Number("Player", "Speed"), 10);
    Destructor(); Check(Get("Player", "Mode").empty(), "State survived destruction");
    std::cout << (MECHANICS_3D ? "3D" : "2D top-down") << " Mechanics tests passed\n";
}
