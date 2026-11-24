# Validation

Verified in a Linux environment with GCC 13.3, CMake 4.4, raylib 5.5, and a Mesa software OpenGL display:

- Full Release game and test targets compile successfully.
- CTest passes the physics suite (4,012 checks, including 1,000 varied shots).
- AddressSanitizer and UndefinedBehaviorSanitizer pass the physics suite. Leak detection was disabled because the execution environment does not support LeakSanitizer.
- Graphical smoke test runs the real executable through aim, flight, result, replay, practice-wall collision, and all ten shots through the final results screen.
- Rendered aim, shot-result, wall-result, and final-summary screenshots were visually reviewed.

Mac and Windows build instructions and launchers are provided, but those platforms were not available for execution testing. This package contains source, not a signed or precompiled app.
