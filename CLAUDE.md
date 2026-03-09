# CLAUDE.md

## Project Overview

UE 5.4 C++ project, custom planetary gravity and orbital mechanics inspired by Outer Wilds. Source code in `Source/UnrealWilds/`.

## Coding Standards

Follow UE coding conventions (U/A/F/E prefixes).

## Architecture

### Gravity System

1. **UGravitySourceComponent** (`Gravity/GravitySourceComponent`) — Attached to celestial bodies. Defines mass, radii, gravity mode.
2. **UGravityWorldSubsystem** (`Gravity/GravityWorldSubsystem`) — Manages all gravity sources. Tick sets CMC gravity direction; registers FGravityAsyncCallback with Chaos solver.
3. **FGravityAsyncCallback** (`Gravity/GravityAsyncCallback`) — Physics thread, applies gravity to all dynamic rigid bodies.

**Gravity calculation**: inverse-square outside planet; shell-theorem interpolation inside shell (linear from 0 at hollow radius to GM/R² at surface); zero inside hollow.

### Character System

**AUWCharacter** (`Character/UWCharacter`) — First-person, two states (`ECharacterMovementState`):
- **SurfaceGravity**: CMC-driven, attached to planet.
- **ZeroG**: Capsule physics simulation, torque-based rotation.

Transitions triggered by APlanet sphere overlap events. `GetVelocity()` overridden to return world-space velocity including planet orbital velocity.

**UUWCharacterMovementComponent** (`Character/UWCharacterMovementComponent`) — Overrides `UpdateBasedMovement()` for planet-relative movement.

**AGravityController** (`GravityController`) — Gravity-relative look input. ZeroG defers to physics rotation.

**UThrusterComponent** (`Pawn/ThrusterComponent`) — Directional force. Character mode (CMC) and physics mode (rigid body).

### Celestial Bodies

**ACelestialBody** (`Astro/CelestialBody`) — Base actor with UGravitySourceComponent.

**APlanet** (`Astro/Planet`) — Orbital mechanics, HollowInnerSphere (zero-g entry), AtmosphereSphere (surface gravity entry). `GetOrbitalVelocity()` for momentum inheritance.

---

1. 如果觉得我有表述不清晰的地方，可以向我提问，我会补充。
2. plan里不要包含代码。
