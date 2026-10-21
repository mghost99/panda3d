# TTEX on Panda3D's Vulkan renderer

Branch `ttex-vulkan` = open-toontown/panda3d master + rdb's `vulkan` branch
(eeb376e855), plus the TTEX fixes listed at the bottom.

## Build

```sh
# Arch: vulkan-headers vulkan-icd-loader glslang spirv-tools spirv-cross (no extra SDK needed)
python3.13 makepanda/makepanda.py --everything --use-vulkan --no-eigen \
    --threads 6 --outputdir built --optimize 3
```

The display module is `built/lib/libp3vulkandisplay.so`.

## Enable Vulkan for the game

Add this to a prc page loaded **after** `config/dev.prc` (dev.prc sets `load-display pandagl`):

```
load-display p3vulkandisplay
sync-video #f          # uses MAILBOX present mode when available
```

There's no `aux-display` fallback, so a broken Vulkan setup fails loudly
instead of silently going back to GL.

**Linux/Hyprland:** the Vulkan window is an X11 (Xwayland) window. Use the
Xwayland display Hyprland started (`pgrep -a Xwayland`, currently `DISPLAY=:1`).
With the wrong display you now get
`Cannot open a Vulkan window: no X11 display connection (check DISPLAY / XAUTHORITY)`
and "Could not open window" instead of a segfault.

Leave `threading-model` at its default (single-threaded). `Cull/Draw` hangs
while opening the Vulkan window (see limitations).

## Custom shaders

Write normal GLSL (`#version 330`+) using Panda's `p3d_*` inputs. Panda
compiles it with glslang to SPIR-V, and the same shader runs unchanged on
OpenGL and Vulkan:

```python
from panda3d.core import Shader, Vec4
sh = Shader.load(Shader.SL_GLSL, "shaders/hologram.vert", "shaders/hologram.frag")
np.setShader(sh, 10)
np.setShaderInput("rim_color", Vec4(0.2, 0.8, 1.0, 1.0))
np.setShaderInput("rim_power", 2.5)
```

Post-processing works through `direct.filter.FilterManager`. The quad shader
must take a `texpad_tex` input: `(0.5*(w-padx)/w, 0.5*(h-pady)/h)`, because
offscreen textures are power-of-two padded. See `samples/ttex-vulkan/`:

* `shaders/hologram.*`: "virtual cog" hologram (fresnel rim glow). This is the
  blue glowing toon from QA, opt-in per node.
* `shaders/post.*`: saturation + vignette post-process.
* `shader_demo.py [p3vulkandisplay|pandagl]`: renders both and saves a screenshot.

Tips: declare every uniform you read, and set a default value for each custom
input *before* the first frame (GL asserts on a missing input). Avoid relying
on implicit varyings. Panda's `p3d_ColorScale`, `p3d_Texture0`,
`p3d_ModelViewProjectionMatrix`, `p3d_NormalMatrix` etc. work on both backends.

## Shader / pipeline caches ("Steam-style" pre-cache)

Two layers, both under `model-cache-dir` (`$XDG_CACHE_HOME/panda3d` by default):

| Cache | File | Keyed by | Portable? |
|---|---|---|---|
| Compiled shaders (SPIR-V) | `generated-shaders/<hash>.sho` | 128-bit hash of shader source + language + format tag | Yes, GPU independent. Can be built once and shipped with the game |
| Vulkan pipelines | `vulkan-pipeline-cache.bin` | `VkPipelineCacheHeaderVersionOne` (vendorID, deviceID, pipelineCacheUUID) | No, per GPU/driver. A stale file is ignored and rebuilt |

* Generated shaders (ShaderGenerator / `Shader.make`) are compiled once, then
  loaded from disk. Prc: `cache-generated-shaders-on-disk #t`.
* Shaders loaded from files go through Panda's existing `.sho` cache when
  `model-cache-compiled-shaders #t`.
* The pipeline cache is loaded when the GSG is created and saved atomically in
  `close_gsg()`. Prc: `vulkan-persist-pipeline-cache #t`,
  `vulkan-pipeline-cache-file <path>` (optional override). It's only written
  when the window is closed normally (`base.graphicsEngine.removeAllWindows()`
  / normal exit). `os._exit()` skips it.

Planned launcher flow:

1. **Build machine:** run `samples/ttex-vulkan/shader_precache.py` over a
   model manifest (every hood/street/cog/toon model plus `shaders/*.vert|frag`)
   and ship the resulting `generated-shaders/*.sho` with the game (the "prebuilt SPIR-V").
2. **Client first launch, or after a driver/GPU change** (detected by the
   pipeline cache header no longer matching): the launcher shows "Pre-compiling
   shaders..." and runs `shader_precache.py` against the shipped manifest. This
   copies/uses the shipped `.sho` files and warms `vulkan-pipeline-cache.bin`
   for this GPU. Later launches skip the step.
3. Bump the `ttex-gensho-N` tag in `Shader::make` (and re-ship) whenever the
   shader pipeline changes incompatibly.

## Measured (RTX 4060 Ti, NVIDIA 615.78, vsync off, same spot/camera)

Deterministic QA hook: the toon is placed at a fixed TTC spot, frames are
timed for 15 s, then the tunnel trigger takes it to street 2226 and the
measurement is repeated. The two series alternated Vulkan and GL runs. GL ran
on the NVIDIA GPU (`__NV_PRIME_RENDER_OFFLOAD=1
__GLX_VENDOR_LIBRARY_NAME=nvidia __GL_SYNC_TO_VBLANK=0`; renderer string
checked every run). Run-to-run noise is about 10%, and another build job was
sharing the CPU (load average 3-4).

| Scene | Run | Vulkan avg / 1% low fps | GL avg / 1% low fps |
|---|---|---|---|
| TTC playground | series 1 | 291 / 123 | 272 / 144 |
| TTC playground | series 2 | 289 / 97 | 284 / 167 |
| Street 2226 | series 1 | 274 / 104 | 206 / 132 |
| Street 2226 | series 2 | 244 / 104 | 206 / 115 |

Vulkan has higher average fps, especially on streets (+18-33%), but worse
1% lows (Vulkan p99 8-9 ms vs GL 5-7 ms). Frames are CPU-bound in Panda's
cull traversal and Python game code. The backend's own state/draw setup costs
about the same on both (~5% of samples each, py-spy).

Startup to the toon picker is the same on both renderers: ~5.4 s warm,
~21 s fully cold. The cold time is Python bytecode, the model cache and disk,
not Vulkan. Pipeline creation is 10-50 ms in total, and ~0.3 ms with the
persisted cache.

Note: TTEX doesn't use `setShaderAuto`, so on Vulkan almost everything is
drawn with the Vulkan GSG's built-in default shader. Variation comes from
pipelines (render states), which the pipeline cache covers. The
generated-shader cache matters for `setShaderAuto`, `CommonFilters` /
string-made shaders, and future custom shaders.

## Known limitations / TODO

* `threading-model Cull/Draw` hangs while opening a Vulkan window. Window
  creation/swapchain isn't safe yet on the draw thread.
* Vulkan validation layers (`vulkan-validation-layers`) aren't installed on
  the dev box. Needs `sudo pacman -S vulkan-validation-layers` for diagnostics.
* rdb's local "skip FM_refresh" hang patch was tried and reverted: it breaks
  every screenshot (`DisplayRegion::get_screenshot` records commands inside
  `begin_frame(FM_refresh)`).
* 1% lows on Vulkan are still behind GL (p99 ~8.5 ms vs ~6 ms). Next: per-frame descriptor-set
  allocation and the per-frame semaphore/present wait in
  `VulkanGraphicsWindow::end_frame`.

## Added on this branch

1. Clean error instead of a segfault when the X display can't be opened.
2. VkPipelineCache persisted to disk with header validation and atomic writes,
   plus pipeline-creation timing logs.
3. On-disk cache for generated / string-made shaders (`generated-shaders/*.sho`).
4. `samples/ttex-vulkan/`: hologram + post shaders, demo, `shader_precache.py`.
