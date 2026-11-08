#pragma once
#include "Orientation.h"
#include <string>

extern "C"
{
    void Constructor();
    void Destructor();
    void Debug(bool Enable);
    void Set_Paths(std::string Resources);
    void Select(std::string Entity);
    void Enter(std::string Entity);
    void Exit(std::string Entity);
    void Update(std::string Entity);
    void Digital(std::string Entity, std::string Action);
    void Analog(std::string Entity, std::string Action, int Value);
    void Set(std::string Entity, std::string Key, std::string Value);
    std::string Get(std::string Entity, std::string Key);
#define MECHANIC(Name) void Name(); void Name##_Analog(int Value);
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
}
