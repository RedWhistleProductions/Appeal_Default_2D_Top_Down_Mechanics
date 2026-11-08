# Appeal_Default_2D_Top_Down_Mechanics

Standalone default top-down mechanics for players and NPCs. This is a separate
local Git repository with no remote. No other plugin repository or external
library is needed. C++17 and Make are required.

## Build and install

Run `make` to build and `make test` to run the native tests. The output is
`Appeal_Default_2D_Top_Down_Mechanics.dll` on Windows or
`Appeal_Default_2D_Top_Down_Mechanics.so` on Linux. Keep the project beneath
`Resources/Modules/Appeal_Module_Mechanics/`. Windows users can run
`powershell -NoProfile -ExecutionPolicy Bypass -File Packaging/Windows/Install.ps1`.
The installer builds in place; `-AppealResources <path>` copies the plugin to
another installation. It does not install or rebuild the Mechanics module.
Use the current module bridge from the Appeal checkout for the new commands.

Choose the orientation once in your setup config:

```text
Add_Dynamic_Module Mechanics Appeal_Default_2D_Top_Down_Mechanics
```

## Basic functions

```text
Mechanics Select Player
Mechanics Move_Right
Mechanics Move_Up
Mechanics Update All
Mechanics Move_Right_Analog 0
Mechanics Move_Up_Analog 0
Mechanics Jump
Mechanics Update All
```

X/Y is the ground plane: +X is right, +Y is up on the map. Z is elevation above
the ground. Jumping affects Z; it never changes the map position in Y.

Every action below has a zero-argument digital function and a one-argument analog
function with `_Analog` appended, both as module commands and C exports. They act
on the selected entity. Digital uses 100% of that entity's configured value;
analog takes an integer -100..100, including 0, without dead zones or device
remapping. Unknown actions and out-of-range inputs are ignored.

| Action | Behavior |
| --- | --- |
| Stand, Stop | Stop both ground movement axes and stand upright |
| Walk | Set walking speed and the Y input |
| Run | Set running speed and the Y input |
| Duck_Stand | Stop both axes and duck |
| Duck_Walk | Set duck-walking speed and the Y input |
| Move_Left, Move_Right | Set the signed X input, preserving the Y input and gait |
| Move_Up, Move_Down | Set the signed Y input, preserving the X input and gait |
| Jump | Launch upward along Z if grounded; preserve planar motion |

`Walk` uses Speed; Run uses Speed * Run_Multiplier; Duck_Walk uses Speed *
Duck_Multiplier. Analog movement scales effective velocity by Value / 100 without
overwriting configured Speed. Left and Down negate that fraction, so negative
input reverses their direction. Independent X/Y inputs persist until replaced or
stopped. The latest command for an axis wins. Send zero to release that axis;
Stand or Stop releases both. Movement is capped at the gait's speed, preventing
diagonal boosts while preserving smaller analog values. Facing_X/Y holds the last
nonzero normalized direction, including while standing, for animation selection.

Analog ducking sets Duck_Amount to abs(Value / 100). Jump strength is Jump_Speed *
abs(Value / 100); zero does not jump. Airborne jump requests are ignored. Gravity
and landing restore the prior ground gait. Stand/Stop stop horizontal movement
but do not cancel a jump.

`Select Entity` selects an existing entity or attaches a new one to Default.
`Use Entity Plugin_Name` attaches and selects an entity in another registered
slot. `Digital Entity Action` and `Analog Entity Action Value` address an entity
without changing selection; these are useful for NPCs. `Update Entity` advances
one entity; `Update All` advances every attached entity. `Release Entity` exits
its plugin; re-entering preserves its settings. Select the player before input
callbacks, since module selection is shared by callbacks.

## Advanced functions

`Mechanics Set Entity Key Value` tunes an attached entity. Distances are game
units; speeds are units/second, gravity is units/second squared, and Delta_Time
is seconds per Update. Defaults require no tuning.

| Setting | Default | Accepted values |
| --- | --- | --- |
| Speed | 4 | Nonnegative finite number |
| Run_Multiplier | 2 | Nonnegative finite number |
| Duck_Multiplier | 0.5 | Nonnegative finite number |
| Jump_Speed | 6 | Nonnegative finite number |
| Gravity | 9.8 | Nonnegative finite number |
| Delta_Time | 1/60 | Positive finite number |
| Ground_Height | 0 | Finite Z height; applied on the next grounded Update |
| Position_X, Position_Y, Position_Z | 0 | Finite number |
| Grounded | 1 | 0 or 1; 1 snaps to Ground_Height and zeros vertical velocity |
| Mode | Ground | String label, independent of action |

Invalid settings do not change state. Set refreshes effective planar movement.
The standard C hooks are Constructor, Destructor, Debug(bool), Set_Paths(string),
Enter(Entity), Exit(Entity), Update(Entity), Digital(Entity, Action),
Analog(Entity, Action, int), and Set(Entity, Key, Value). Optional Select(Entity)
and Get(Entity, Key) support native selection and module state publication.

## Globals

The bridge publishes `Mechanics.Selected_Entity` (string, initially empty),
`Mechanics.<Entity>.Plugin` (string slot name), and all settings above under
`Mechanics.<Entity>.<Key>`. Mode is a string, Grounded is bool, and the other
settings are floats. Additional per-entity globals are:

| Key | Type | Default | Meaning |
| --- | --- | --- | --- |
| Orientation | string | 2D_Top_Down | Fixed coordinate convention |
| Action | string | Stand | Ground gait, stationary posture, or Jump |
| Velocity_X, Velocity_Y, Velocity_Z | float | 0 | Effective per-axis speed |
| Duck_Amount | float | 0 | Ducking blend, 0..1 |
| Input | int | 0 | Last accepted input value |
| Facing_X, Facing_Y | float | 0, 1 | Last planar facing direction |

Globals refresh on Select, Use, actions, Set, and Update. Release clears Plugin,
Mode, Action, and Orientation and clears selection when appropriate. Numeric
values remain inactive snapshots. Unsupported globals from other orientations
are empty. Direct C calls do not publish globals until the next module refresh.

## Scope and validation

This is kinematic movement on a configurable flat ground plane. Each entity has
independent state. It does not detect collisions, move World entities, drive
Physics bodies, resize collision shapes, or switch Graphics animations. Collision
code may supply Ground_Height and Grounded. Water/ladder/vehicle modes and state
transfer between plugins remain future integration work. Mode and Action are
separate so Graphics can eventually select an animation set and clip.

Native tests cover all action exports, signed/zero input, diagonal movement,
facing, the Z jump axis, landing, and entity isolation. The checkout's
`Tests/Default_Mechanics_Tests.ps1` additionally loads all three orientations
simultaneously through Appeal and checks shared globals and action routing.
