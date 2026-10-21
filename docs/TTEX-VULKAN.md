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
sync-video #f          # uncapped: MAILBOX if available, else IMMEDIATE, >=4 swapchain images
```

Optional Vulkan knobs:

| prc | default | |
|---|---|---|
| `vulkan-present-mode` | `auto` | `fifo`, `fifo-relaxed`, `mailbox`, `immediate`; unsupported falls back to fifo |
| `vulkan-swapchain-images` | `0` (auto) | auto = 1 + back-buffers, at least 4 when uncapped |
| `vulkan-validation` | `#f` | enable `VK_LAYER_KHRONOS_validation` (install `vulkan-validation-layers`); slow |
| `vulkan-frame-stats` | `0` | every N frames log uploads/allocs/descriptor allocs and fence/acquire/present waits (needs `notify-level-vulkandisplay info`) |
| `vulkan-recycle-semaphores` | `#t` | reuse per-frame acquire semaphores |

The chosen present mode and image count are logged at info level
(`Swapchain 1280x720 present mode immediate, 4 images requested ...`). On
NVIDIA/Xwayland the surface offers no MAILBOX, so uncapped means IMMEDIATE.

There's no `aux-display` fallback, so a broken Vulkan setup fails loudly
instead of silently going back to GL.

**Linux/Hyprland:** the Vulkan window is an X11 (Xwayland) window. Use the
Xwayland display Hyprland started (`pgrep -a Xwayland`, currently `DISPLAY=:1`).
With the wrong display you now get
`Cannot open a Vulkan window: no X11 display connection (check DISPLAY / XAUTHORITY)`
and "Could not open window" instead of a segfault.

`threading-model Cull/Draw` works on Vulkan (and is faster, see below); the
default single-threaded model is still what the measurements in the main
table use.

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
measurement is repeated. Runs alternate Vulkan and GL, one client at a time.
GL ran on the NVIDIA GPU (`__NV_PRIME_RENDER_OFFLOAD=1
__GLX_VENDOR_LIBRARY_NAME=nvidia __GL_SYNC_TO_VBLANK=0`; renderer string
checked every run). Run-to-run noise is about 10% (more on the street, where
cogs walk through the view).

Before/after the VULKAN3 fixes (single-threaded, avg fps / 1% low fps / p99 ms):

| Scene | Vulkan before (3 swapchain imgs) | Vulkan after, series 1 / 2 | GL, series 1 / 2 |
|---|---|---|---|
| TTC playground | 289-300 / 93-123 / 9.8-10.2 | 327 / 198 / 4.57 ; 331 / 195 / 4.55 | 294 / 165 / 4.98 ; 296 / 173 / 4.89 |
| Street 2226 | 244-280 / 94-104 / 8.1-9.6 | 324 / 195 / 4.70 ; 244 / 147 / 5.80 | 222 / 131 / 6.45 ; 220 / 120 / 6.98 |

Vulkan now beats GL on average fps, 1% lows and p99 in both scenes.

Cause of the old bad 1% lows: in IMMEDIATE mode with 3 swapchain images,
`vkAcquireNextImageKHR` blocked for ~6 ms in bursts of every 3rd frame about
twice a second (PStats: all of the extra time in the spike frames was
`Wait:Flip`, with no child collectors). Requesting 4 images removes it (5 is no
better). Fence waits, descriptor-set allocation (~11 cached sets/frame),
semaphore creation (~1 us) and buffer uploads were measured with
`vulkan-frame-stats` and were all negligible.

Cog battle (street 2226, toon teleported onto the nearest free cog, 12 s
measured from 4 s after the place enters `battle`; different cog each run):

| Run | avg fps | 1% low | p99 ms |
|---|---|---|---|
| Vulkan 1 | 386 | 158 | 4.29 |
| GL 1 | 377 | 162 | 4.55 |
| Vulkan 2 | 454 | 174 | 4.80 |
| GL 2 | 476 | 150 | 4.84 |

Battle is roughly a tie (within noise); the scene is light.

`threading-model Cull/Draw` (one run each): Vulkan TTC 416 / 206 / 3.70,
street 358 / 196 / 4.27; GL TTC 448 / 195 / 4.22, street 253 / 104 / 9.05.

Startup to the toon picker is the same on both renderers: ~5.4 s warm,
~21 s fully cold. The cold time is Python bytecode, the model cache and disk,
not Vulkan. Pipeline creation is 10-50 ms in total, and ~0.3 ms with the
persisted cache.

Note: TTEX doesn't use `setShaderAuto`, so on Vulkan almost everything is
drawn with the Vulkan GSG's built-in default shader. Variation comes from
pipelines (render states), which the pipeline cache covers. The
generated-shader cache matters for `setShaderAuto`, `CommonFilters` /
string-made shaders, and future custom shaders.

## Validation

Run with `vulkan-validation #t` (Arch: `vulkan-validation-layers`). A full
TTEX session (picker, TTC, screenshot, street, exit) is clean, in both
single-threaded and `Cull/Draw` mode. Errors found and fixed this way:

| VUID | Cause | Fix |
|---|---|---|
| `vkCmdDraw*-dynamicRenderingUnusedAttachments-08917` | `framebuffer-stencil #t` gives a D24S8 attachment but the pipeline had `stencilAttachmentFormat` UNDEFINED | set stencil format for combined depth/stencil |
| `VkPresentInfoKHR-pImageIndices-01430` | screenshot left the presented swapchain image in TRANSFER_SRC | restore PRESENT_SRC after the readback |
| `vkDestroyDevice-device-05137` (VkSemaphore) | acquire semaphore of a failed (out-of-date) acquire overwritten on swapchain recreate | destroy it in `destroy_swapchain()` |
| `VkBufferCreateInfo-size-00912`, `vkCmdCopyBuffer-size-00116/-dstOffset-00114`, `vkCmdBindVertexBuffers-pOffsets-00626`, `vkCmdDrawIndexed-robustBufferAccess2-08798` (Cull/Draw) | vertex/index buffers sized at prepare time and never regrown; empty arrays made 0-byte buffers | regrow on size change, minimum 16 bytes |

## Hangs fixed

* **FM_refresh / open_windows hang** (the one rdb's reverted "skip
  FM_refresh" patch targeted): the GSG has a 40-entry command-buffer pool;
  when one frame needed more (many offscreen buffers, or `make_contexts()`
  doing FM_refresh on every window), `create_command_buffer()` waited for
  the oldest frame, which was the unsubmitted current frame, forever. Now it
  only waits for submitted frames and otherwise grows the pool. FM_refresh
  still does real work, so `saveScreenshot` keeps working. Repro:
  `samples/ttex-vulkan/refresh_repro.py` (N offscreen buffers + openWindows + renderFrame +
  saveScreenshot) hung at buffer 39, now fine with N=100.
* **Cull/Draw hang**: `DisplayRegion::get_screenshot()` deadlocked with any
  separate draw thread (GL too): the callback run on the draw thread re-entered
  `get_screenshot()`, failed the pipeline-stage check (run_on_thread lends the
  caller's stage) and called `run_on_draw_thread()` again, blocking on the
  engine lock. Plus the Vulkan window screenshot used the GSG's last-bound
  framebuffer (null on the draw thread) instead of the window's last rendered
  swapchain image.

## Known limitations / TODO

* Window screenshots read the last *presented* swapchain image. That works
  on every driver tested but isn't strictly allowed by the spec; a
  persistent copy at end_frame would be the clean fix.
* `ShowBase` exiting via `sys.exit()` from inside a task after
  `removeAllWindows()` hangs in `Messenger.ignoreAll` on both GL and Vulkan
  (Python lock, not the renderer). Use `os._exit` after closing windows, as
  the QA hooks do.

## Added on this branch

1. Clean error instead of a segfault when the X display can't be opened.
2. VkPipelineCache persisted to disk with header validation and atomic writes,
   plus pipeline-creation timing logs.
3. On-disk cache for generated / string-made shaders (`generated-shaders/*.sho`).
4. `samples/ttex-vulkan/`: hologram + post shaders, demo, `shader_precache.py`.
5. Validation-layer fixes (stencil format, present layout, semaphore leaks)
   and the `vulkan-validation` switch.
6. Swapchain: >=4 images when uncapped, `vulkan-present-mode`,
   `vulkan-swapchain-images`, maxImageCount=0 clamp fix, recycled semaphores,
   `vulkan-frame-stats`.
7. Command-buffer pool growth (FM_refresh / many-buffers hang).
8. Cull/Draw: screenshot deadlock fix (display), correct screenshot source,
   vertex/index buffer regrow.
