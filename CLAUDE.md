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

Transitions triggered by UPlanetAttachmentComponent delegates (OnAttachedToPlanet → EnterSurfaceGravity, OnDetachedFromPlanet → EnterZeroG) and APlanet hollow sphere overlaps. `GetVelocity()` overridden to return world-space velocity including planet orbital velocity via PlanetAttachment. Interact action finds nearby AShipPawn and calls `BoardShip()`. `PossessedBy` override restores DefaultMappingContext on re-possession.

**UUWCharacterMovementComponent** (`Character/UWCharacterMovementComponent`) — Overrides `UpdateBasedMovement()` for planet-relative movement.

**AGravityController** (`GravityController`) — Gravity-relative look input. ZeroG and non-character pawns (e.g. ship) defer to physics rotation.

**UThrusterComponent** (`Pawn/ThrusterComponent`) — Directional force. Character mode (CMC AddForce) and physics mode (rigid body AddForce). Mass auto-detected from CMC or root PrimitiveComponent.

**UPlanetAttachmentComponent** (`Pawn/PlanetAttachmentComponent`) — Reusable component for planet attach/detach logic. Tracks CurrentPlanet, handles AttachToActor/DetachFromActor, orbital velocity inheritance on detach (auto for physics-simulating primitives), and initial state detection via CheckInitialPlanetState(). Broadcasts OnAttachedToPlanet/OnDetachedFromPlanet delegates. Used by both AUWCharacter and AShipPawn.

### Ship System

**AShipPawn** (`Pawn/ShipPawn`) — Physics-driven spaceship (APawn). 6DOF movement identical to character ZeroG: torque-based rotation + thruster force. Components: UStaticMeshComponent root (SimulatePhysics, ECC_Pawn), UCameraComponent, UThrusterComponent (physics mode), UProbeLauncherComponent, UPlanetAttachmentComponent, USphereComponent (interaction zone).

- **Boarding**: Character walks into InteractionZone overlap, presses Interact → `BoardShip()` hides character, Controller possesses ship. Ship Interact → `OnExitShip()` restores character, re-possesses, calls `CheckInitialMovementState`.
- **Planet attachment**: Handled by UPlanetAttachmentComponent. APlanet AtmosphereSphere overlap finds the component and calls AttachToPlanet/DetachFromPlanet. Velocity inheritance automatic (ShipMesh always simulates physics).
- **GetVelocity()** overridden to return physics velocity + PlanetAttachment orbital velocity.
- Input mapping context managed in `PossessedBy`/`UnPossessed`.

### Celestial Bodies

**ACelestialBody** (`Astro/CelestialBody`) — Base actor with UGravitySourceComponent.

**APlanet** (`Astro/Planet`) — Orbital mechanics, HollowInnerSphere (zero-g entry, Character-specific), AtmosphereSphere (triggers UPlanetAttachmentComponent attach/detach for any actor with the component). `GetOrbitalVelocity()` for momentum inheritance.

---

1. 如果觉得我有表述不清晰的地方，可以向我提问，我会补充。
2. plan里不要包含代码。
3. 如果我有不合理的架构、实现方法，可以直接指出。
