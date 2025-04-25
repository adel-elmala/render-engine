# Render Engine
> Design is still in progress.

Custom render engine, with support for multiple APIs (dx11 only for now, and to be extended to use vulkan/dx12 in the future)

### Feature List:
1. mirror reflection
2. shadows (wip)


### Old cpu-based render engine can be found [here](https://github.com/adel-elmala/render-engine/tree/software-renderer).

## TODO:
1. <del>Refactor the engine, handle  only d3d11 and remove any software-backend related stuff</del>
2. <del>Refactor Application stuff, and make it more flexable to use </del>
3. <del>make the engine API more verbose with options to render the scene and models, (i.e enable shadows, wireframe, transparentcy,...), passes should not be implemented into the main by the user.</del>
4. <del>add imgui support</del>
5. <del>render light (toggle rendering the light as sprites from the engine)</del>
6. <del> render bounding-boxes for scence models </del>
7. add logging
8. <del>make a branch that uses only software backend for showcasing</del>
9. <del>make new engine after all this to be the main branch state</del>
10. <del>edit readme</del>
11. fix rendeing artifacts (lighting direction, shadows, black flickering...)
12. fix leaks
