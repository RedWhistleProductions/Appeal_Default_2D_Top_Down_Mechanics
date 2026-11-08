#include "Mechanics.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

namespace
{
    constexpr double Pi = 3.14159265358979323846;
    constexpr const char *Vertical = MECHANICS_3D ? "Y" : "Z";
    constexpr const char *Forward = MECHANICS_3D ? "Z" : "Y";
    struct Entity_State
    {
        std::string Mode = "Ground";
        std::string Action = "Stand";
        std::string Gait = "Walk";
        bool Grounded = true;
        bool External_Physics = false;
        bool Ducking = false;
        double Lateral_Input = 0;
        double Forward_Input = 0;
        std::map<std::string, double> Values = {
            {"Speed", 4}, {"Run_Multiplier", 2}, {"Duck_Multiplier", 0.5},
            {"Jump_Speed", 6}, {"Gravity", 9.8}, {"Delta_Time", 1.0 / 60},
            {"Ground_Height", 0}, {"Position_X", 0}, {"Position_Y", 0}, {"Position_Z", 0},
            {"Velocity_X", 0}, {"Velocity_Y", 0}, {"Velocity_Z", 0},
            {"Duck_Amount", 0}, {"Input", 0}, {"Yaw", 0}, {"Pitch", 0},
            {"Turn_Speed", 180}, {"Look_Speed", 90}, {"Facing_X", 0}, {"Facing_Y", 1}
        };
    };
    std::map<std::string, Entity_State> Entities;
    std::string Selected_Entity;
    bool Debug_Output = false;

    void Refresh_Movement(Entity_State &State)
    {
        auto &V = State.Values;
        // Cap the vector length, not each axis, to prevent diagonal speed boosts.
        const double Length = std::max(1.0, std::hypot(State.Lateral_Input, State.Forward_Input));
        const double Right = State.Lateral_Input / Length;
        const double Front = State.Forward_Input / Length;
        const double Multiplier = State.Gait == "Run" ? V["Run_Multiplier"]
            : State.Gait == "Duck_Walk" ? V["Duck_Multiplier"] : 1;
        const double Speed = V["Speed"] * Multiplier;
        if(MECHANICS_3D)
        {
            const double Angle = V["Yaw"] * Pi / 180;
            V["Velocity_X"] = (Right * std::cos(Angle) + Front * std::sin(Angle)) * Speed;
            V["Velocity_Z"] = (-Right * std::sin(Angle) + Front * std::cos(Angle)) * Speed;
        }
        else
        {
            V["Velocity_X"] = Right * Speed;
            V["Velocity_Y"] = Front * Speed;
            if(Right != 0 || Front != 0)
            {
                const double Facing_Length = std::hypot(Right, Front);
                V["Facing_X"] = Right / Facing_Length;
                V["Facing_Y"] = Front / Facing_Length;
            }
        }
        if(State.Grounded)
        {
            if(Right == 0 && Front == 0) State.Action = State.Ducking ? "Duck_Stand" : "Stand";
            else State.Action = State.Gait;
        }
        else State.Action = "Jump";
    }

    bool Supported(const std::string &Action)
    {
        if(Action == "Stand" || Action == "Stop" || Action == "Walk" || Action == "Run" ||
           Action == "Jump" || Action == "Duck_Stand" || Action == "Duck_Walk" ||
           Action == "Move_Left" || Action == "Move_Right") return true;
        if(MECHANICS_3D) return Action == "Move_Forward" || Action == "Move_Backward" ||
            Action == "Strafe_Left" || Action == "Strafe_Right" || Action == "Turn_Left" ||
            Action == "Turn_Right" || Action == "Look_Up" || Action == "Look_Down";
        return Action == "Move_Up" || Action == "Move_Down";
    }

    void Act(const std::string &Action, int Value)
    {
        auto Found = Entities.find(Selected_Entity);
        if(Found == Entities.end() || Value < -100 || Value > 100 || !Supported(Action)) return;
        auto &State = Found->second;
        auto &V = State.Values;
        const double Amount = Value / 100.0; // No dead zone or device remapping.
        V["Input"] = Value;
        if(Action == "Jump")
        {
            if(Value == 0 || !State.Grounded) return;
            V[std::string("Velocity_") + Vertical] = V["Jump_Speed"] * std::abs(Amount);
            State.Grounded = false;
            State.Action = "Jump";
        }
        else if(Action == "Turn_Left" || Action == "Turn_Right")
        {
            const double Direction = Action == "Turn_Left" ? -1 : 1;
            V["Yaw"] = std::remainder(V["Yaw"] + Direction * V["Turn_Speed"] * Amount * V["Delta_Time"], 360);
        }
        else if(Action == "Look_Up" || Action == "Look_Down")
        {
            const double Direction = Action == "Look_Up" ? 1 : -1;
            V["Pitch"] = std::clamp(V["Pitch"] + Direction * V["Look_Speed"] * Amount * V["Delta_Time"], -89.0, 89.0);
        }
        else if(Action == "Stand" || Action == "Stop" || Action == "Duck_Stand")
        {
            State.Lateral_Input = State.Forward_Input = 0;
            State.Ducking = Action == "Duck_Stand" && Value != 0;
            V["Duck_Amount"] = State.Ducking ? std::abs(Amount) : 0;
            State.Gait = State.Ducking ? "Duck_Walk" : "Walk";
        }
        else if(Action == "Walk" || Action == "Run" || Action == "Duck_Walk")
        {
            State.Gait = Action;
            State.Ducking = Action == "Duck_Walk" && Value != 0;
            V["Duck_Amount"] = State.Ducking ? std::abs(Amount) : 0;
            State.Forward_Input = Amount;
        }
        else if(Action == "Move_Left" || Action == "Strafe_Left") State.Lateral_Input = -Amount;
        else if(Action == "Move_Right" || Action == "Strafe_Right") State.Lateral_Input = Amount;
        else if(Action == "Move_Up" || Action == "Move_Forward") State.Forward_Input = Amount;
        else if(Action == "Move_Down" || Action == "Move_Backward") State.Forward_Input = -Amount;
        Refresh_Movement(State);
        if(Debug_Output) std::cout << "Mechanics " << Selected_Entity << " " << Action << " " << Value << '\n';
    }
}

extern "C" void Constructor() {}
extern "C" void Destructor() { Entities.clear(); Selected_Entity.clear(); }
extern "C" void Debug(bool Enable) { Debug_Output = Enable; }
extern "C" void Set_Paths(std::string) {}
extern "C" void Select(std::string Entity) { Selected_Entity = Entities.count(Entity) ? Entity : ""; }
extern "C" void Enter(std::string Entity) { Entities.try_emplace(Entity); Select(Entity); }
extern "C" void Exit(std::string Entity)
{
    auto Found = Entities.find(Entity);
    if(Found != Entities.end())
    {
        auto &State = Found->second;
        State.Lateral_Input = State.Forward_Input = 0;
        State.Ducking = false;
        State.Gait = "Walk";
        State.Values["Duck_Amount"] = 0;
        State.Values["Velocity_X"] = State.Values["Velocity_Y"] = State.Values["Velocity_Z"] = 0;
        State.Action = "Stand";
    }
    if(Selected_Entity == Entity) Selected_Entity.clear();
}

#define MECHANIC(Name) \
    extern "C" void Name() { Act(#Name, 100); } \
    extern "C" void Name##_Analog(int Value) { Act(#Name, Value); }
MECHANIC(Stand)
MECHANIC(Stop)
MECHANIC(Walk)
MECHANIC(Run)
MECHANIC(Jump)
MECHANIC(Duck_Stand)
MECHANIC(Duck_Walk)
MECHANIC(Move_Left)
MECHANIC(Move_Right)
#if MECHANICS_3D
MECHANIC(Move_Forward)
MECHANIC(Move_Backward)
MECHANIC(Strafe_Left)
MECHANIC(Strafe_Right)
MECHANIC(Turn_Left)
MECHANIC(Turn_Right)
MECHANIC(Look_Up)
MECHANIC(Look_Down)
#else
MECHANIC(Move_Up)
MECHANIC(Move_Down)
#endif
#undef MECHANIC

extern "C" void Digital(std::string Entity, std::string Action)
{
    const std::string Previous = Selected_Entity;
    Select(Entity); Act(Action, 100); Selected_Entity = Previous;
}
extern "C" void Analog(std::string Entity, std::string Action, int Value)
{
    const std::string Previous = Selected_Entity;
    Select(Entity); Act(Action, Value); Selected_Entity = Previous;
}
extern "C" void Update(std::string Entity)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return;
    auto &State = Found->second;
    auto &V = State.Values;
    if(State.External_Physics) { Refresh_Movement(State); return; }
    const double Dt = V["Delta_Time"];
    V["Position_X"] += V["Velocity_X"] * Dt;
    V[std::string("Position_") + Forward] += V[std::string("Velocity_") + Forward] * Dt;
    auto &Height = V[std::string("Position_") + Vertical];
    auto &Lift = V[std::string("Velocity_") + Vertical];
    if(State.Grounded) Height = V["Ground_Height"];
    else
    {
        Height += Lift * Dt - 0.5 * V["Gravity"] * Dt * Dt;
        Lift -= V["Gravity"] * Dt;
        if(Height <= V["Ground_Height"] && Lift <= 0)
        {
            Height = V["Ground_Height"]; Lift = 0; State.Grounded = true;
        }
    }
    Refresh_Movement(State);
}
extern "C" void Set(std::string Entity, std::string Key, std::string Value)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return;
    auto &State = Found->second;
    auto &V = State.Values;
    if(Key == "External_Physics") { State.External_Physics = Value == "1" || Value == "True"; return; }
    if(Key == "Mode") { State.Mode = Value; return; }
    if(Key == "Grounded")
    {
        if(Value != "0" && Value != "1") return;
        State.Grounded = Value == "1";
        if(State.Grounded)
        {
            V[std::string("Velocity_") + Vertical] = 0;
            if(!State.External_Physics) V[std::string("Position_") + Vertical] = V["Ground_Height"];
        }
        Refresh_Movement(State);
        return;
    }
    const bool Nonnegative = Key == "Speed" || Key == "Run_Multiplier" || Key == "Duck_Multiplier" ||
        Key == "Jump_Speed" || Key == "Gravity" || (MECHANICS_3D && (Key == "Turn_Speed" || Key == "Look_Speed"));
    const bool Signed = Key == "Velocity_X" || Key == "Velocity_Y" || Key == "Velocity_Z" || Key == "Position_X" || Key == "Position_Y" || Key == "Position_Z" || Key == "Ground_Height" ||
        (MECHANICS_3D && (Key == "Yaw" || Key == "Pitch"));
    if(!Nonnegative && !Signed && Key != "Delta_Time") return;
    try
    {
        std::size_t Used = 0;
        double Number = std::stod(Value, &Used);
        if(Used != Value.size() || !std::isfinite(Number) || (Nonnegative && Number < 0) ||
           (Key == "Delta_Time" && Number <= 0)) return;
        if(Key == "Yaw") Number = std::remainder(Number, 360);
        if(Key == "Pitch") Number = std::clamp(Number, -89.0, 89.0);
        V[Key] = Number;
        Refresh_Movement(State);
    }
    catch(const std::exception &) {}
}
extern "C" std::string Get(std::string Entity, std::string Key)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return "";
    const auto &State = Found->second;
    if(Key == "Orientation") return MECHANICS_3D ? "3D" : "2D_Top_Down";
    if(Key == "Mode") return State.Mode;
    if(Key == "Action") return State.Action;
    if(Key == "External_Physics") return State.External_Physics ? "1" : "0";
    if(Key == "Grounded") return State.Grounded ? "1" : "0";
    if(Key == "Input") return std::to_string(static_cast<int>(State.Values.at("Input")));
    if(!MECHANICS_3D && (Key == "Yaw" || Key == "Pitch" || Key == "Turn_Speed" || Key == "Look_Speed")) return "";
    if(MECHANICS_3D && (Key == "Facing_X" || Key == "Facing_Y")) return "";
    auto Value = State.Values.find(Key);
    return Value == State.Values.end() ? "" : std::to_string(Value->second);
}
